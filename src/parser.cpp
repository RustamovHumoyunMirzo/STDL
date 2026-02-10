#include "parser.hpp"
#include <tao/pegtl.hpp>
#include <iostream>
#include <sstream>
#include <cctype>

namespace STDLParser {

namespace pegtl = TAO_PEGTL_NAMESPACE;

struct ParserState {
    Scene* scene = nullptr;
    std::vector<NodePtr> nodeStack;
};

inline std::string trim(const std::string& s){
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end   = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end-start+1);
}

inline std::string unquote(const std::string& s){
    if(s.size()>=2 && s.front()=='"' && s.back()=='"') return s.substr(1, s.size()-2);
    return s;
}

Value parsePrimitive(const std::string& s){
    std::string val = trim(s);

    if(val == "true") return true;
    if(val == "false") return false;

    try { return std::stoi(val); } catch(...) {}

    try { return std::stod(val); } catch(...) {}

    return unquote(val);
}

Ref parseLocalRef(const std::string& s){
    Ref r;
    size_t hash = s.find('#');
    if(hash != std::string::npos){
        r.localID = std::stoi(s.substr(hash+1, s.size()-hash-2));
    }
    return r;
}

Ref parseGlobalRef(const std::string& s){
    Ref r;
    size_t colon = s.find(':');
    size_t at = s.find('@');
    size_t end = s.find('>');

    if(colon != std::string::npos && at != std::string::npos){
        r.type = s.substr(1, colon-1);
        r.name = s.substr(colon+1, at-colon-1);
        r.globalID = std::stoi(s.substr(at+1, end-at-1));
    }
    return r;
}

template<typename Rule>
struct Action : pegtl::nothing<Rule> {};

template<> struct Action<grammar::integer>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        state.nodeStack.back()->properties["_last_value"] = std::stoi(in.string());
    }
};

template<> struct Action<grammar::floating>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        state.nodeStack.back()->properties["_last_value"] = std::stod(in.string());
    }
};

template<> struct Action<grammar::boolean>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        state.nodeStack.back()->properties["_last_value"] = (in.string()=="true");
    }
};

template<> struct Action<grammar::quoted_string>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        state.nodeStack.back()->properties["_last_value"] = unquote(in.string());
    }
};

template<> struct Action<grammar::local_ref>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        state.nodeStack.back()->properties["_last_value"] = parseLocalRef(in.string());
    }
};

template<> struct Action<grammar::global_ref>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        state.nodeStack.back()->properties["_last_value"] = parseGlobalRef(in.string());
    }
};

template<> struct Action<grammar::list>{
    template<typename Input>
    static void apply(const Input&, ParserState& state){
        if(state.nodeStack.empty()) return;

        NodePtr node = state.nodeStack.back();
        Value val = node->properties["_last_value"];

        if(!std::holds_alternative<std::vector<Value>>(val)){
            node->properties["_last_value"] = std::vector<Value>{val};
        }
    }
};

template<> struct Action<grammar::property>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        if(state.nodeStack.empty()) return;
        NodePtr node = state.nodeStack.back();

        std::string line = in.string();
        size_t eq = line.find('=');
        if(eq==std::string::npos) return;

        std::string key = trim(line.substr(0, eq));
        std::string valStr = trim(line.substr(eq+1));

        if(node->properties.find("_last_value") != node->properties.end()){
            node->properties[key] = node->properties["_last_value"];
            node->properties.erase("_last_value");
        } else {
            node->properties[key] = parsePrimitive(valStr);
        }
    }
};

template<> struct Action<grammar::node>{
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        std::string header = in.string();
        std::istringstream ss(header);
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

        if(state.nodeStack.empty())
            state.scene->addNode(node);
        else
            state.nodeStack.back()->addChild(node);

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

    pegtl::memory_input<> in(input, "STDL");

    try{
        pegtl::parse<grammar::scene, Action>(in, state);
    } catch(const pegtl::parse_error& e){
        std::cerr << "Parse error: " << e.what() << "\n";
        return false;
    }

    return true;
}

}