# STDL - Simple Tree Description Language

A lightweight C++ parser for hierarchical scene data. Think JSON, but designed specifically for game scenes and structured data with cross-references.

## Why STDL?

I got tired of wrestling with JSON and XML for game scenes. STDL is what I wish existed when I started - simple syntax, built-in references between objects, and actually readable by humans.
```stdl
scene v1

node player Hero @1
{
    health = 100
    position = [10.5, 20.0, 5.5]
    
    node weapon Sword #10
    {
        damage = 25
        durability = 100
    }
}

node enemy Goblin @2
{
    health = 50
    target = <player:Hero @1>    // reference to the player
    weapon = <#10>                 // reference to local weapon
}
```

## Features

- **Clean syntax** - No angle brackets everywhere, no closing tags
- **Hierarchical nodes** - Nest as deep as you want
- **Type safety** - Integers, floats, booleans, strings, and lists
- **References** - Link objects using IDs instead of duplicating data
- **Comments** - Because code without comments is just cruel

## Building

You need CMake 3.15+ and a C++17 compiler.
```bash
git clone https://github.com/RustamovHumoyunMirzo/STDL.git
cd STDL
git submodule update --init --recursive  # Get PEGTL
mkdir build && cd build
cmake ..
cmake --build .
```

## Quick Start
```cpp
#include "stdl.hpp"

// Load a scene
auto scene = STDL::LoadFile("level1.stdl");
if(!scene) {
    // handle error
}

// Find nodes
auto player = scene->getNodeByName("Hero");

// Read properties
int health;
if(player->get("health", health)) {
    std::cout << "Player health: " << health << "\n";
}

// Navigate children
auto weapon = player->getChild("Sword");

// Modify and save
player->set("health", 150);
STDL::SaveFile(scene, "level1_modified.stdl");
```

## Syntax Guide

### Basic Structure

Every file starts with `scene v1` followed by your nodes:
```stdl
scene v1

node type_name NodeName
{
    // properties and child nodes
}
```

### Properties
```stdl
name = "Alice"              // string
health = 100                // integer
armor = 25.5                // float
alive = true                // boolean
position = [0, 10, 0]       // list
```

### Node IDs

Nodes can have local IDs (unique within their type) or global IDs (unique across the whole scene):
```stdl
node player Hero @1         // global ID 1
node weapon Sword #10       // local ID 10
```

### References

Point to other nodes using their IDs:
```stdl
target = <player:Hero @1>        // reference by global ID
equipped = <weapon:Sword #10>    // reference by local ID  
quick_ref = <#10>                // short form for local ID
```

### Comments
```stdl
// This is a comment
health = 100  // inline comment works too
```

### Lists

Lists can contain any mix of types:
```stdl
mixed = [1, 2.5, "text", true, <#12>]
```

### Nested Nodes
```stdl
node player Hero
{
    health = 100
    
    node inventory Backpack
    {
        capacity = 20
        
        node item Potion
        {
            quantity = 5
        }
    }
}
```

## API Reference

### Loading & Saving
```cpp
ScenePtr LoadFile(const std::string& path);
ScenePtr LoadString(const std::string& content);
bool SaveFile(const ScenePtr& scene, const std::string& path);
std::string ToString(const ScenePtr& scene);
```

### Scene Methods
```cpp
NodePtr getNodeByName(const std::string& name);
void addNode(const NodePtr& node);
```

### Node Methods
```cpp
// Get child by name
NodePtr getChild(const std::string& childName);

// Read property
template<typename T>
bool get(const std::string& key, T& out);

// Write property
template<typename T>
void set(const std::string& key, T val);

// Add child node
void addChild(const NodePtr& child);
```

### Node Fields
```cpp
std::string type;                           // node type
std::string name;                           // node name
std::optional<int> localID;                 // local ID if set
std::optional<int> globalID;                // global ID if set
std::map<std::string, Value> properties;    // all properties
std::vector<NodePtr> children;              // child nodes
```

## Example Use Cases

**Game Configuration:**
```stdl
node config GameSettings
{
    resolution = [1920, 1080]
    fullscreen = true
    fov = 90.5
}
```

**Level Data:**
```stdl
node level "Forest Entrance"
{
    difficulty = 2
    
    node spawn PlayerSpawn
    {
        position = [0, 0, 0]
        rotation = [0, 90, 0]
    }
    
    node enemy Goblin @100
    {
        health = 50
        patrol_points = [
            [10, 0, 5],
            [20, 0, 5],
            [20, 0, 15]
        ]
    }
}
```

**Dialogue Trees:**
```stdl
node dialogue "Meet the Merchant"
{
    text = "Welcome, traveler! Care to see my wares?"
    
    node choice "Yes" #1
    {
        text = "Show me what you have."
        next = <#2>
    }
    
    node choice "No" #2
    {
        text = "Maybe later."
    }
}
```

## Limitations

- Property names must start with a letter and can only contain letters, numbers, and underscores
- No negative numbers yet (use strings as a workaround: `damage = "-10"`)
- No escape sequences in strings (no `\n`, `\t`, etc.)
- References aren't automatically resolved - you have to look them up yourself

## Dependencies

- [PEGTL](https://github.com/taocpp/PEGTL) - Parser combinator library (included as submodule)
- C++17 standard library

## Contributing

Found a bug? Have an idea? Open an issue or PR. This started as a weekend project, so there's plenty of room for improvement.

## Why "STDL"?

Simple Tree Description Language. Yeah, I'm not great at naming things. If you have a better name, I'm all ears.