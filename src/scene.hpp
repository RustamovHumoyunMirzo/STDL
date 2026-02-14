#pragma once
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <memory>
#include <optional>

struct Scene;

struct Node;
using NodePtr = std::shared_ptr<Node>;

struct Ref {
    std::optional<int> localID;
    std::optional<int> globalID;
    std::optional<std::string> type;
    std::optional<std::string> name;
};

struct ValueNode;
using Value = std::variant<
    int,
    double,
    bool,
    std::string,
    Ref,
    std::vector<std::shared_ptr<ValueNode>>
>;

struct ValueNode {
    Value value;
};

struct Node {
    std::string type;
    std::string name;
    std::optional<int> localID;
    std::optional<int> globalID;
    std::map<std::string, Value> properties;
    std::vector<NodePtr> children;

    NodePtr getChild(const std::string& childName){
        for(auto& c: children)
            if(c->name == childName) return c;
        return nullptr;
    }
    
    NodePtr getChildByLocalID(int localID){
        return findChildByLocalID(children, localID, type);
    }
    
    template<typename T>
    bool get(const std::string& key, T& out){
        auto it = properties.find(key);
        if(it != properties.end() && std::holds_alternative<T>(it->second)){
            out = std::get<T>(it->second);
            return true;
        }
        return false;
    }
    
    bool getRef(const std::string& key, Ref& out){
        auto it = properties.find(key);
        if(it != properties.end() && std::holds_alternative<Ref>(it->second)){
            out = std::get<Ref>(it->second);
            return true;
        }
        return false;
    }
    
    NodePtr resolveRef(const Ref& ref, Scene* scene);
    
    template<typename T>
    void set(const std::string& key, T val){
        properties[key] = val;
    }
    
    void addChild(const NodePtr& child){
        children.push_back(child);
    }

    bool getList(const std::string& key, std::vector<std::shared_ptr<ValueNode>>& out){
        auto it = properties.find(key);
        if(it != properties.end() && std::holds_alternative<std::vector<std::shared_ptr<ValueNode>>>(it->second)){
            out = std::get<std::vector<std::shared_ptr<ValueNode>>>(it->second);
            return true;
        }
        return false;
    }

    template<typename T>
    bool getListElement(const std::string& key, size_t index, T& out){
        std::vector<std::shared_ptr<ValueNode>> list;
        if(getList(key, list) && index < list.size()){
            if(std::holds_alternative<T>(list[index]->value)){
                out = std::get<T>(list[index]->value);
                return true;
            }
        }
        return false;
    }

private:
    NodePtr findChildByLocalID(const std::vector<NodePtr>& nodeList, int localID, const std::string& nodeType){
        for(auto& n: nodeList){
            if(n->type == nodeType && n->localID && *n->localID == localID) 
                return n;
            auto found = findChildByLocalID(n->children, localID, nodeType);
            if(found) return found;
        }
        return nullptr;
    }
};

struct Scene {
    std::vector<NodePtr> nodes;
    
    NodePtr getNodeByName(const std::string& name){
        for(auto& n: nodes)
            if(n->name == name) return n;
        return nullptr;
    }
    
    NodePtr getNodeByGlobalID(int globalID){
        return findNodeByGlobalID(nodes, globalID);
    }
    
    void addNode(const NodePtr& node){
        nodes.push_back(node);
    }

private:
    NodePtr findNodeByGlobalID(const std::vector<NodePtr>& nodeList, int globalID){
        for(auto& n: nodeList){
            if(n->globalID && *n->globalID == globalID) return n;
            auto found = findNodeByGlobalID(n->children, globalID);
            if(found) return found;
        }
        return nullptr;
    }
};

namespace STDL {
std::string valueToString(const Value& val);
}
using ScenePtr = std::shared_ptr<Scene>;