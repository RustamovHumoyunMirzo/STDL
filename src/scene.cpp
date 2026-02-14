#include "scene.hpp"
#include "parser.hpp"
#include "stdl.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace STDL {

ScenePtr LoadFile(const std::string& path){
    std::ifstream file(path);
    if(!file) return nullptr;
    std::stringstream ss;
    ss << file.rdbuf();
    return LoadString(ss.str());
}

ScenePtr LoadString(const std::string& content){
    ScenePtr scene = std::make_shared<Scene>();
    if(!STDLParser::ParseSTDL(content, *scene)){
        std::cerr << "Failed to parse STDL content\n";
        return nullptr;
    }
    return scene;
}

bool SaveFile(const ScenePtr& scene, const std::string& path){
    std::ofstream file(path);
    if(!file) return false;
    file << ToString(scene);
    return true;
}

namespace {
namespace {
std::string valueToString(const Value& val){
    if(std::holds_alternative<int>(val)) return std::to_string(std::get<int>(val));
    if(std::holds_alternative<double>(val)){
        std::ostringstream oss;
        oss << std::get<double>(val);
        return oss.str();
    }
    if(std::holds_alternative<bool>(val)) return std::get<bool>(val) ? "true" : "false";
    if(std::holds_alternative<std::string>(val)){
        auto& s = std::get<std::string>(val);
        std::string escaped = "\"";
        for(char c : s){
            switch(c){
                case '\n': escaped += "\\n"; break;
                case '\t': escaped += "\\t"; break;
                case '\r': escaped += "\\r"; break;
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                default: escaped += c; break;
            }
        }
        escaped += "\"";
        return escaped;
    }
    if(std::holds_alternative<Ref>(val)){
        auto& r = std::get<Ref>(val);
        std::string s = "<";
        if(r.type) s += *r.type;
        if(r.localID){
            s += "#" + std::to_string(*r.localID);
        } else if(r.globalID){
            if(r.name) s += ":" + *r.name;
            s += " @" + std::to_string(*r.globalID);
        }
        s += ">";
        return s;
    }
    if(std::holds_alternative<std::vector<std::shared_ptr<ValueNode>>>(val)){
        auto& vec = std::get<std::vector<std::shared_ptr<ValueNode>>>(val);
        std::string s = "[";
        for(size_t i = 0; i < vec.size(); ++i){
            s += valueToString(vec[i]->value);
            if(i+1 < vec.size()) s += ", ";
        }
        s += "]";
        return s;
    }
    return "";
}
}

void serializeNode(const NodePtr& node, std::ostream& os, int indent=0){
    std::string ind(indent,' ');
    os << ind << "node " << node->type << " " << node->name;
    if(node->globalID) os << " @" << *node->globalID;
    if(node->localID) os << " #" << *node->localID;
    os << "\n" << ind << "{\n";

    for (auto it = node->properties.begin(); it != node->properties.end(); ++it) {
        const std::string& k = it->first;
        Value& v = it->second;
        os << ind << "  " << k << " = " << valueToString(v) << "\n";
    }

    for (auto& c : node->children) {
        serializeNode(c, os, indent + 2);
    }

    os << ind << "}\n";
}
}

std::string ToString(const ScenePtr& scene){
    std::ostringstream os;
    os << "scene v1\n";
    for(auto& n: scene->nodes){
        serializeNode(n, os, 0);
    }
    return os.str();
}

}