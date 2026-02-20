#include "parser.hpp"
#include <tao/pegtl.hpp>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace STDLParser {
namespace pegtl = TAO_PEGTL_NAMESPACE;

static bool hasPath(
    Node* from,
    Node* to,
    const std::unordered_map<Node*, std::unordered_set<Node*>>& graph,
    std::unordered_set<Node*>& visited)
{
    if(from == to) return true;
    if(!graph.count(from)) return false;

    for(Node* next : graph.at(from)){
        if(visited.insert(next).second){
            if(hasPath(next, to, graph, visited))
                return true;
        }
    }
    return false;
}

struct ParserState {
    Scene* scene = nullptr;
    std::vector<NodePtr> nodeStack;

    bool inList = false;
    std::vector<std::shared_ptr<ValueNode>> currentList;

    std::unordered_map<Node*, std::unordered_set<Node*>> refGraph;

    std::unordered_map<int, Node*> localIDMap;
    std::unordered_map<int, Node*> globalIDMap;
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

        Node* from = state.nodeStack.back().get();

        std::string text = in.string();
        size_t hash = text.find('#');
        size_t gt = text.find('>');

        Ref ref;
        if(hash != std::string::npos && gt != std::string::npos){
            ref.localID = std::stoi(text.substr(hash + 1, gt - hash - 1));
        }

        if (ref.localID.has_value()) {
            int id = ref.localID.value();

            auto it = state.localIDMap.find(id);
            if(it != state.localIDMap.end()){
                Node* to = it->second;
            
                std::unordered_set<Node*> visited;
                if(hasPath(to, from, state.refGraph, visited)){
                    throw pegtl::parse_error(
                        "Circular reference detected (local)",
                        in
                    );
                }
            
                state.refGraph[from].insert(to);
            }
        }

        auto valNode = std::make_shared<ValueNode>();
        valNode->value = ref;

        if(state.inList){
            state.currentList.push_back(valNode);
        } else {
            state.nodeStack.back()->properties["_last_value"] =
                std::vector<std::shared_ptr<ValueNode>>{valNode};
        }
    }
};

template<> struct Action<grammar::global_ref>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;

        Node* from = state.nodeStack.back().get();

        std::string text = in.string();
        size_t colon = text.find(':');
        size_t at = text.find('@');
        size_t gt = text.find('>');

        Ref ref;
        if(at != std::string::npos && gt != std::string::npos){
            ref.globalID = std::stoi(text.substr(at + 1, gt - at - 1));

            if(colon != std::string::npos){
                if(colon > 1){
                    ref.type = text.substr(1, colon - 1);
                }
                if(at > colon + 1){
                    ref.name = text.substr(colon + 1, at - colon - 1);
                    if (ref.name.has_value()) {
                        auto& s = *ref.name;
                        s.erase(0, s.find_first_not_of(" \t"));
                        s.erase(s.find_last_not_of(" \t") + 1);
                    }
                }
            }
        }

        if (ref.globalID.has_value()) {
            int id = ref.globalID.value();

            auto it = state.globalIDMap.find(id);
            if(it != state.globalIDMap.end()){
                Node* to = it->second;
            
                std::unordered_set<Node*> visited;
                if(hasPath(to, from, state.refGraph, visited)){
                    throw pegtl::parse_error(
                        "Circular reference detected (global)",
                        in
                    );
                }
            
                state.refGraph[from].insert(to);
            }
        }

        auto valNode = std::make_shared<ValueNode>();
        valNode->value = ref;

        if(state.inList){
            state.currentList.push_back(valNode);
        } else {
            state.nodeStack.back()->properties["_last_value"] =
                std::vector<std::shared_ptr<ValueNode>>{valNode};
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
            if(token[0]=='@'){
                node->globalID = std::stoi(token.substr(1));
                if (node->globalID) {
                    state.globalIDMap[*node->globalID] = node.get();
                }
            }
            else if(token[0]=='#'){
                node->localID = std::stoi(token.substr(1));
                if (node->localID) {
                    state.localIDMap[*node->localID] = node.get();
                }
            }
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