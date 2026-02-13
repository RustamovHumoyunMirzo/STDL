#include "parser.hpp"
#include <tao/pegtl.hpp>
#include <iostream>
#include <sstream>

namespace STDLParser {
namespace pegtl = TAO_PEGTL_NAMESPACE;

struct ParserState {
    Scene* scene = nullptr;
    std::vector<NodePtr> nodeStack;
};

inline std::string unquote(const std::string& str){
    if(str.size()>=2 && str.front()=='"' && str.back()=='"') return str.substr(1,str.size()-2);
    return str;
}

template<typename Rule>
struct Action : pegtl::nothing<Rule> {};

template<> struct Action<grammar::integer>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = std::stoi(in.string());
        state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
    }
};
template<> struct Action<grammar::floating>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = std::stod(in.string());
        state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
    }
};
template<> struct Action<grammar::boolean>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = (in.string()=="true");
        state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
    }
};
template<> struct Action<grammar::quoted_string>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        auto valNode = std::make_shared<ValueNode>();
        valNode->value = unquote(in.string());
        state.nodeStack.back()->properties["_last_value"] = std::vector<std::shared_ptr<ValueNode>>{valNode};
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
            auto& vec = std::get<std::vector<std::shared_ptr<ValueNode>>>(it->second);
            if(!vec.empty()){
                node->properties[key] = vec[0]->value;
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
template<> struct Action<grammar::node> {
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        std::string text = in.string();
        
        size_t brace = text.find('{');
        if(brace != std::string::npos) {
            text = text.substr(0, brace);
        }
        
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
        if(!state.nodeStack.empty()) state.nodeStack.pop_back();
    }
};

bool ParseSTDL(const std::string& input, Scene& scene){
    ParserState state;
    state.scene = &scene;
    pegtl::memory_input<> in(input,"STDL");
    try{
        pegtl::parse<grammar::scene,Action>(in,state);
    }catch(const pegtl::parse_error& e){
        std::cerr << "Parse error: " << e.what() << "\n";
        return false;
    }
    return true;
}
}