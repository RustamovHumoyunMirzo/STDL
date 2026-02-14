#include "parser.hpp"
#include <tao/pegtl.hpp>
#include <iostream>
#include <sstream>

namespace STDLParser {
namespace pegtl = TAO_PEGTL_NAMESPACE;

struct ParserState {
    Scene* scene = nullptr;
    std::vector<NodePtr> nodeStack;
    bool inList = false;
    std::vector<std::shared_ptr<ValueNode>> currentList;
};

inline std::string unquote(const std::string& str){
    if(str.size() < 2 || str.front() != '"' || str.back() != '"') 
        return str;
    
    std::string result;
    result.reserve(str.size() - 2);
    
    for(size_t i = 1; i < str.size() - 1; ++i){
        if(str[i] == '\\' && i + 1 < str.size() - 1){
            switch(str[i + 1]){
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                default: result += str[i + 1]; break;
            }
            ++i;
        } else {
            result += str[i];
        }
    }
    return result;
}

template<typename Rule>
struct Action : pegtl::nothing<Rule> {};

template<> struct Action<grammar::integer>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = std::stoi(in.string());
        
        if(state.inList){
            state.currentList.push_back(valNode);
        } else {
            state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
        }
    }
};

template<> struct Action<grammar::floating>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = std::stod(in.string());
        
        if(state.inList){
            state.currentList.push_back(valNode);
        } else {
            state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
        }
    }
};

template<> struct Action<grammar::boolean>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = (in.string()=="true");
        
        if(state.inList){
            state.currentList.push_back(valNode);
        } else {
            state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
        }
    }
};

template<> struct Action<grammar::quoted_string>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = unquote(in.string());
        
        if(state.inList){
            state.currentList.push_back(valNode);
        } else {
            state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
        }
    }
};

template<> struct Action<grammar::local_ref>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        
        std::string text = in.string();
        size_t hash = text.find('#');
        size_t gt = text.find('>');
        
        Ref ref;
        if(hash != std::string::npos && gt != std::string::npos){
            std::string idStr = text.substr(hash + 1, gt - hash - 1);
            ref.localID = std::stoi(idStr);
            
            if(hash > 1){
                std::string typeStr = text.substr(1, hash - 1);
                if(!typeStr.empty()) ref.type = typeStr;
            }
        }
        
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = ref;
        
        if(state.inList){
            state.currentList.push_back(valNode);
        } else {
            state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
        }
    }
};

template<> struct Action<grammar::global_ref>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        
        std::string text = in.string();
        size_t colon = text.find(':');
        size_t at = text.find('@');
        size_t gt = text.find('>');
        
        Ref ref;
        if(at != std::string::npos && gt != std::string::npos){
            std::string idStr = text.substr(at + 1, gt - at - 1);
            ref.globalID = std::stoi(idStr);
            
            if(colon != std::string::npos){
                if(colon > 1){
                    std::string typeStr = text.substr(1, colon - 1);
                    if(!typeStr.empty()) ref.type = typeStr;
                }
                if(at > colon + 1){
                    std::string nameStr = text.substr(colon + 1, at - colon - 1);
                    nameStr.erase(0, nameStr.find_first_not_of(" \t"));
                    nameStr.erase(nameStr.find_last_not_of(" \t") + 1);
                    if(!nameStr.empty()) ref.name = nameStr;
                }
            }
        }
        
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = ref;
        
        if(state.inList){
            state.currentList.push_back(valNode);
        } else {
            state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
        }
    }
};

template<> struct Action<pegtl::one<'['>>{
    template<typename Input>
    static void apply(const Input&, ParserState& state){
        state.inList = true;
        state.currentList.clear();
    }
};

template<> struct Action<pegtl::one<']'>>{
    template<typename Input>
    static void apply(const Input&, ParserState& state){
        if(state.nodeStack.empty()) return;
        state.inList = false;
        state.nodeStack.back()->properties["_last_value"] = state.currentList;
    }
};

template<> struct Action<grammar::property>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto node = state.nodeStack.back();
        std::string line = in.string();
        size_t eq = line.find('=');
        if(eq==std::string::npos) return;
        
        std::string key = line.substr(0, eq);
        key.erase(0,key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n")+1);
        
        auto it = node->properties.find("_last_value");
        if(it != node->properties.end()){
            auto& lastVal = it->second;
            
            if(std::holds_alternative<std::vector<std::shared_ptr<ValueNode>>>(lastVal)){
                auto& vec = std::get<std::vector<std::shared_ptr<ValueNode>>>(lastVal);
                node->properties[key] = vec;
            }
            node->properties.erase("_last_value");
        }
    }
};

template<> struct Action<grammar::node_header> {
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        std::string text = in.string();
        text.erase(0, text.find_first_not_of(" \t\r\n"));
        text.erase(text.find_last_not_of(" \t\r\n") + 1);
        
        std::istringstream ss(text);
        std::string tmp, type, name;
        ss >> tmp >> type >> name;
        
        NodePtr node = std::make_shared<Node>();
        node->type = type;
        node->name = name;
        
        std::string token;
        while(ss >> token){
            if(token[0]=='@') node->globalID = std::stoi(token.substr(1));
            else if(token[0]=='#') node->localID = std::stoi(token.substr(1));
        }

        if(state.nodeStack.empty()){
            state.scene->addNode(node);
        } else {
            state.nodeStack.back()->addChild(node);
        }

        state.nodeStack.push_back(node);
    }
};

template<> struct Action<pegtl::one<'}'>>{
    template<typename Input>
    static void apply(const Input&, ParserState& state){
        if(!state.nodeStack.empty()){
            state.nodeStack.pop_back();
        }
    }
};

bool ParseSTDL(const std::string& input, Scene& scene){
    ParserState state;
    state.scene = &scene;
    pegtl::memory_input<> in(input,"STDL");
    try{
        bool result = pegtl::parse<grammar::scene,Action>(in,state);
        return result;
    }catch(const pegtl::parse_error& e){
        std::cerr << "Parse error: " << e.what() << "\n";
        return false;
    }
}
}