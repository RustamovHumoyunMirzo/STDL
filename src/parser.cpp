#include "parser.hpp"
#include <tao/pegtl.hpp>
#include <iostream>
#include <sstream>

namespace STDLParser {

namespace pegtl = TAO_PEGTL_NAMESPACE;

// -------------------------
// Helper actions for PEGTL
// -------------------------

struct ParserState {
    Scene* scene = nullptr;
    std::vector<NodePtr> nodeStack;  // keeps track of current nesting
};

// Utility: remove quotes from string
inline std::string unquote(const std::string& str) {
    if(str.size() >= 2 && str.front() == '"' && str.back() == '"')
        return str.substr(1, str.size()-2);
    return str;
}

// -------------------------
// PEGTL Actions
// -------------------------
template<typename Rule>
struct Action : pegtl::nothing<Rule> {};

template<> struct Action<grammar::integer> {
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        int val = std::stoi(in.string());
        if(!state.nodeStack.empty()){
            auto node = state.nodeStack.back();
            node->properties["_last_value"] = val; // temporary storage
        }
    }
};

template<> struct Action<grammar::floating> {
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        double val = std::stod(in.string());
        if(!state.nodeStack.empty()){
            auto node = state.nodeStack.back();
            node->properties["_last_value"] = val;
        }
    }
};

template<> struct Action<grammar::boolean> {
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        bool val = (in.string() == "true");
        if(!state.nodeStack.empty()){
            auto node = state.nodeStack.back();
            node->properties["_last_value"] = val;
        }
    }
};

template<> struct Action<grammar::quoted_string> {
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        std::string val = unquote(in.string());
        if(!state.nodeStack.empty()){
            auto node = state.nodeStack.back();
            node->properties["_last_value"] = val;
        }
    }
};

// -------------------------
// Node Actions
// -------------------------
template<> struct Action<grammar::node> {
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        std::string header = in.string(); // includes type+name+IDs
        std::istringstream ss(header);
        std::string tmp, type, name;
        ss >> tmp; // skip 'node'
        ss >> type;
        ss >> name;

        NodePtr node = std::make_shared<Node>();
        node->type = type;
        node->name = name;

        std::string token;
        while(ss >> token){
            if(token[0] == '@') node->globalID = std::stoi(token.substr(1));
            else if(token[0] == '#') node->localID = std::stoi(token.substr(1));
        }

        if(state.nodeStack.empty()){
            state.scene->addNode(node);
        } else {
            state.nodeStack.back()->addChild(node);
        }
        state.nodeStack.push_back(node);
    }
};

// Pop node stack at closing '}'
template<> struct Action<pegtl::one<'}'>> {
    template<typename Input>
    static void apply(const Input&, ParserState& state){
        if(!state.nodeStack.empty())
            state.nodeStack.pop_back();
    }
};

// -------------------------
// Property Actions
// -------------------------
template<> struct Action<grammar::property> {
    template<typename Input>
    static void apply(const Input& in, ParserState& state){
        std::string line = in.string();
        auto node = state.nodeStack.back();
        size_t eq = line.find('=');
        if(eq == std::string::npos) return;
        std::string key = line.substr(0, eq);
        std::string valStr = line.substr(eq+1);
        key.erase(0,key.find_first_not_of(" \t\n\r"));
        key.erase(key.find_last_not_of(" \t\n\r")+1);
        valStr.erase(0,valStr.find_first_not_of(" \t\n\r"));
        valStr.erase(valStr.find_last_not_of(" \t\n\r")+1);

        node->properties[key] = valStr;
    }
};

bool ParseSTDL(const std::string& input, Scene& scene){
    ParserState state;
    state.scene = &scene;

    pegtl::memory_input<> in(input, "STDL");

    try {
        pegtl::parse<grammar::scene, Action>(in, state);
    } catch (const pegtl::parse_error& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return false;
    }

    return true;
}

}