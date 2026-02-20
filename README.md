# STDL - Scene Tree Description Language

A lightweight C++ parser for hierarchical scene descriptions. Think of it as a simple, readable format for defining game objects, scenes, or any tree-structured data where you need references between nodes.

---

## What is STDL?

STDL lets you describe complex, interconnected data structures in a clean text format. It's particularly useful for game development, scene graphs, or configuration files where you need to:

- Define hierarchical relationships (parent-child nodes)
- Reference objects by ID (both locally and globally)
- Store typed data (numbers, strings, booleans, lists)
- Keep everything human-readable and version-control friendly

Here's a quick taste:

```stdl
scene v1

node player Hero @1
{
    health = 100
    position = [0.0, 5.0, 10.0]
    
    node weapon Sword #1
    {
        damage = 50
        enchanted = true
    }
}

node enemy Goblin @2
{
    target = <player:Hero @1>  // Reference to the player
    weapon = <#1>              // Reference to local weapon
}
````

---

## Features

* **Hierarchical structure** – Nest nodes as deep as you need
* **Two reference types** – Local IDs (`#`) for same-type siblings, global IDs (`@`) for anything
* **Rich data types** – Integers, floats, booleans, strings, lists, and references
* **Type-safe parsing** – Built on PEGTL for robust grammar handling
* **Easy API** – Simple C++ interface for loading, querying, and saving
* **Comments** – Because code without comments is like a map without labels

---

## Building

You'll need **CMake 3.15+** and a **C++17 compiler**.

```bash
git clone https://github.com/RustamovHumoyunMirzo/STDL.git
cd STDL
git submodule update --init --recursive  # Get PEGTL
mkdir build && cd build
cmake ..
make
```

The PEGTL library is included as a submodule in the `external/` directory.

---

## Quick Start

### Loading a Scene

```cpp
#include "stdl/stdl.hpp"

// From a file
auto scene = STDL::LoadFile("scene.stdl");

// From a string
std::string content = R"(
    scene v1
    node player Hero @1 { health = 100 }
)";
auto scene = STDL::LoadString(content);
```

### Accessing Nodes and Properties

```cpp
// Find node by name
auto player = scene->getNodeByName("Hero");

// Get properties
int health;
if (player->get("health", health)) {
    std::cout << "Health: " << health << "\n";
}

// Get string property
std::string name;
player->get("name", name);

// Get boolean
bool isAlive;
player->get("isAlive", isAlive);
```

### Working with Lists

```cpp
// Get a list property
std::vector<std::shared_ptr<ValueNode>> items;
if (player->getList("inventory", items)) {
    for (auto& item : items) {
        if (std::holds_alternative<std::string>(item->value)) {
            std::cout << std::get<std::string>(item->value) << "\n";
        }
    }
}

// Or get specific list element
std::string firstItem;
if (player->getListElement("inventory", 0, firstItem)) {
    std::cout << "First item: " << firstItem << "\n";
}
```

### Following References

```cpp
// Get a reference
Ref targetRef;
if (enemy->getRef("target", targetRef)) {
    auto target = enemy->resolveRef(targetRef, scene.get());
    if (target) {
        std::cout << "Enemy targeting: " << target->name << "\n";
    }
}
```

### Navigating the Tree

```cpp
// Get child by name
auto weapon = player->getChild("Sword");

// Get child by local ID
auto weapon = player->getChildByLocalID(1);

// Iterate children
for (auto& child : player->children) {
    std::cout << child->type << " " << child->name << "\n";
}
```

### Modifying and Saving

```cpp
// Set properties
player->set("health", 50);
player->set("position", std::vector<double>{1.0, 2.0, 3.0});

// Add children
auto weapon = std::make_shared<Node>();
weapon->type = "weapon";
weapon->name = "Axe";
weapon->set("damage", 75);
player->addChild(weapon);

// Save back to file
STDL::SaveFile(scene, "output.stdl");

// Or get as string
std::string output = STDL::ToString(scene);
```

---

## STDL Format Reference

### File Structure

Every STDL file starts with a version header:

```stdl
scene v1
```

Then you define nodes.

### Nodes

Nodes are the building blocks. Basic syntax:

```stdl
node <type> <name> [#localID] [@globalID]
{
    // properties
}
```

**IDs explained:**

* **Local ID (`#`)** – Unique within nodes of the same type. Used for sibling references.
* **Global ID (`@`)** – Unique across the scene. Used for cross-cutting references.

---

### Properties

Properties are key-value pairs. Keys must start with a letter and may contain letters, numbers, and underscores.

```stdl
health = 100
speed = 2.5
name = "Hero"
isAlive = true
```

**Supported data types:**

* Integers: `count = 42`, `negative = -10`
* Floats: `speed = 3.14`, `temperature = -273.15`
* Booleans: `active = true`, `hidden = false`
* Strings: `name = "Alice"`, `message = "Hello\nWorld"`
* Lists:

```stdl
numbers = [1, 2, 3, 4]
mixed = [42, "text", true, 3.14]
positions = [0.0, 5.0, 10.0]
targets = [<@1>, <@2>, <enemy:Goblin @3>]
```

---

### References

* **Local reference** – Points to a sibling node of the same type:

```stdl
weapon = <#1>         // Simple local reference
weapon = <weapon#1>   // With type hint
```

* **Global reference** – Points anywhere in the scene:

```stdl
target = <player:Hero @100>   // Type, name, and global ID
target = <:Hero @100>         // Name and global ID
target = <player: @100>       // Type and global ID
target = <@100>               // Just global ID
```

---

### Nested Nodes

```stdl
node player Hero @1
{
    health = 100
    
    node inventory Backpack
    {
        slots = 20
        
        node item Potion #1 { count = 5 }
        node item Sword #2 { damage = 50 }
    }
}
```

---

### Comments

```stdl
// This is a comment
node player Hero  // This too
{
    health = 100  // And this
}
```

---

## Common Patterns

### Scene Graph

```stdl
scene v1
node root SceneRoot @1
{
    node camera MainCamera @2 { position = [0.0,5.0,-10.0], target = <@1> }
    node light Sun @3 { color = [1.0,1.0,0.9], intensity = 1.0 }
    node mesh Player @10 { model = "player.obj", position = [0,0,0] }
}
```

### Inventory System

```stdl
node player Alice @1
{
    inventory = [
        <item:HealthPotion #1>,
        <item:HealthPotion #2>,
        <item:Sword #3>
    ]
    
    node item HealthPotion #1 { healAmount = 50 }
    node item HealthPotion #2 { healAmount = 50 }
    node item Sword #3 { damage = 75, durability = 100 }
}
```

### AI Behavior

```stdl
node enemy Goblin @50
{
    health = 30
    state = "patrol"
    target = <player:Hero @1>
    
    patrolPoints = [
        [0.0,0.0,0.0],
        [10.0,0.0,0.0],
        [10.0,0.0,10.0],
        [0.0,0.0,10.0]
    ]
}
```

---

## API Reference

### Loading Functions

```cpp
namespace STDL {
    ScenePtr LoadFile(const std::string& path);
    ScenePtr LoadString(const std::string& content);
    bool SaveFile(const ScenePtr& scene, const std::string& path);
    std::string ToString(const ScenePtr& scene);
}
```

### Scene Class

```cpp
struct Scene {
    std::vector<NodePtr> nodes;
    NodePtr getNodeByName(const std::string& name);
    NodePtr getNodeByGlobalID(int globalID);
    void addNode(const NodePtr& node);
};
```

### Node Class

```cpp
struct Node {
    std::string type;
    std::string name;
    std::optional<int> localID;
    std::optional<int> globalID;
    std::map<std::string, Value> properties;
    std::vector<NodePtr> children;

    NodePtr getChild(const std::string& childName);
    NodePtr getChildByLocalID(int localID);
    void addChild(const NodePtr& child);

    template<typename T>
    bool get(const std::string& key, T& out);

    bool getRef(const std::string& key, Ref& out);

    bool getList(const std::string& key, std::vector<std::shared_ptr<ValueNode>>& out);

    template<typename T>
    bool getListElement(const std::string& key, size_t index, T& out);

    template<typename T>
    void set(const std::string& key, T val);

    NodePtr resolveRef(const Ref& ref, Scene* scene);
};
```

### Value Types

```cpp
using Value = std::variant<
    int,
    double,
    bool,
    std::string,
    Ref,
    std::vector<std::shared_ptr<ValueNode>>
>;

struct Ref {
    std::optional<int> localID;
    std::optional<int> globalID;
    std::optional<std::string> type;
    std::optional<std::string> name;
};
```

---

## Error Handling

The parser reports errors to `std::cerr` and returns `nullptr` on failure:

```cpp
auto scene = STDL::LoadFile("scene.stdl");
if (!scene) {
    std::cerr << "Failed to load scene\n";
    return 1;
}
```

Parse errors include line/column information from PEGTL.

---

## Performance Notes

* Parsing is single-threaded but fast enough for most game assets
* Scene graph is kept in memory — watch RAM with huge scenes
* Reference resolution is O(n) worst-case

---

## Limitations

* No circular reference detection
* No schema validation
* No incremental parsing
* Comments are discarded during parsing

---

## Contributing

Found a bug? Want a feature? PRs are welcome!

* Keep the same code style
* Add tests for new features
* Update the README if the format changes

---

## Why STDL?

JSON is verbose. XML is worse. YAML has invisible syntax errors. Custom binary formats are a pain to debug.

STDL is simple: nodes, properties, references. Human-readable, version-control friendly, and focused.