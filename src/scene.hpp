#pragma once
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <iostream>

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

    NodePtr getChild(const std::string& childName) {
        for(auto& c: children)
            if(c->name == childName) return c;
        return nullptr;
    }

    template<typename T>
    bool get(const std::string& key, T& out) {
        auto it = properties.find(key);
        if(it != properties.end() && std::holds_alternative<T>(it->second)){
            out = std::get<T>(it->second);
            return true;
        }
        return false;
    }

    template<typename T>
    void set(const std::string& key, T val) {
        properties[key] = val;
    }

    void addChild(const NodePtr& child) {
        children.push_back(child);
    }
};

struct Scene {
    std::vector<NodePtr> nodes;

    NodePtr getNodeByName(const std::string& name) {
        for(auto& n: nodes)
            if(n->name == name) return n;
        return nullptr;
    }

    void addNode(const NodePtr& node){
        nodes.push_back(node);
    }
};