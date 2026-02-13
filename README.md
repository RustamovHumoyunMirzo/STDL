# STDL - Scene Tree Description Language

A C++ library for parsing and serializing hierarchical scene descriptions using a custom domain-specific language.

## Overview

STDL provides a lightweight, human-readable format for describing scene graphs and hierarchical data structures. It's designed for applications that need to define complex object hierarchies with properties, such as 3D scenes, UI layouts, or configuration systems.

## Features

- **Simple Syntax**: Clean, readable format inspired by configuration languages
- **Hierarchical Structure**: Natural tree representation with parent-child relationships
- **Rich Data Types**: Support for integers, floats, booleans, strings, lists, and references
- **Node References**: Local (`#ID`) and global (`@ID`) reference system
- **Header-only API**: Simple integration with `#include "stdl.hpp"`
- **PEG Parser**: Built on PEGTL for robust parsing

## Language Syntax

### Basic Structure

```
scene v1
node NodeType NodeName @globalID #localID
{
    propertyName = value
    node ChildType ChildName
    {
        // child properties
    }
}
```

### Example

```
scene v1
node Transform root @1 #1
{
    position = [0.0, 1.5, 0.0]
    rotation = [0.0, 0.0, 0.0]
    scale = [1.0, 1.0, 1.0]
    
    node Mesh cube @2 #2
    {
        vertices = 8
        material = "default"
        visible = true
        parentTransform = <Transform:root @1>
    }
    
    node Light mainLight @3 #3
    {
        intensity = 1.0
        color = [255, 255, 255]
        type = "directional"
    }
}
```

### Data Types

| Type | Example | Description |
|------|---------|-------------|
| Integer | `42` | Whole numbers |
| Float | `3.14` | Decimal numbers |
| Boolean | `true`, `false` | Boolean values |
| String | `"hello world"` | Quoted text |
| List | `[1, 2, 3]` | Arrays of values |
| Local Ref | `<#5>` | Reference by local ID |
| Global Ref | `<Type:name @10>` | Reference by type, name, and global ID |

### References

STDL supports two types of node references:

- **Local References**: `<#localID>` - References within the same file
- **Global References**: `<Type:name @globalID>` - References with type and name information

## Installation

### Requirements

- C++17 or later
- [PEGTL](https://github.com/taocpp/PEGTL) library

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Integration

Include the library in your project:

```cpp
#include "stdl.hpp"
```

## API Reference

### Loading Scenes

#### Load from File

```cpp
STDL::ScenePtr scene = STDL::LoadFile("path/to/scene.stdl");
if (scene) {
    // Process scene
}
```

#### Load from String

```cpp
std::string content = R"(
scene v1
node Object root @1
{
    name = "example"
}
)";

STDL::ScenePtr scene = STDL::LoadString(content);
```

### Saving Scenes

#### Save to File

```cpp
bool success = STDL::SaveFile(scene, "output.stdl");
```

#### Convert to String

```cpp
std::string content = STDL::ToString(scene);
```

### Working with Nodes

#### Access Nodes

```cpp
// Get node by name
NodePtr node = scene->getNodeByName("root");

// Access child nodes
NodePtr child = node->getChild("childName");

// Iterate through all nodes
for (auto& node : scene->nodes) {
    std::cout << node->name << std::endl;
}
```

#### Read Properties

```cpp
int value;
if (node->get("propertyName", value)) {
    // Use value
}

std::string text;
node->get("name", text);

double position;
node->get("x", position);
```

#### Set Properties

```cpp
node->set("health", 100);
node->set("name", std::string("player"));
node->set("active", true);
node->set("position", 3.14);
```

#### Add Children

```cpp
auto child = std::make_shared<Node>();
child->type = "Transform";
child->name = "childNode";
node->addChild(child);
```

### Node Structure

```cpp
struct Node {
    std::string type;                    // Node type
    std::string name;                    // Node name
    std::optional<int> localID;          // Local identifier
    std::optional<int> globalID;         // Global identifier
    std::map<std::string, Value> properties;  // Key-value properties
    std::vector<NodePtr> children;       // Child nodes
};
```

### Value Types

The `Value` type is a variant that can hold:

```cpp
std::variant<
    int,
    double,
    bool,
    std::string,
    Ref,
    std::vector<std::shared_ptr<ValueNode>>
>
```

## Use Cases

### 3D Scene Graphs

```
scene v1
node Scene world @1
{
    node Camera main @2
    {
        fov = 60.0
        position = [0.0, 5.0, 10.0]
        target = [0.0, 0.0, 0.0]
    }
    
    node Model character @3
    {
        mesh = "character.obj"
        texture = "character.png"
        position = [0.0, 0.0, 0.0]
    }
}
```

### UI Hierarchy

```
scene v1
node Window mainWindow @1
{
    width = 800
    height = 600
    title = "Application"
    
    node Panel sidebar @2
    {
        position = [0, 0]
        size = [200, 600]
        
        node Button btn1 @3
        {
            text = "Click Me"
            enabled = true
        }
    }
}
```

### Configuration System

```
scene v1
node Config app @1
{
    version = "1.0.0"
    debug = false
    
    node Database db @2
    {
        host = "localhost"
        port = 5432
        name = "mydb"
    }
    
    node Server web @3
    {
        port = 8080
        workers = 4
    }
}
```

## Advanced Features

### Comments

```
scene v1
// This is a comment
node Object test @1
{
    // Comments can appear anywhere
    value = 42
}
```

### Nested Lists

```
matrix = [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
```

### Reference System

```cpp
// Create a reference
Ref ref;
ref.globalID = 5;
ref.type = "Transform";
ref.name = "target";
node->set("reference", ref);
```

## Error Handling

The parser provides error messages for invalid syntax:

```cpp
STDL::ScenePtr scene = STDL::LoadFile("invalid.stdl");
if (!scene) {
    // Parse error occurred, check stderr for details
}
```

## Performance Considerations

- Parsing is single-threaded
- Memory usage scales with scene complexity
- References are stored as values, not resolved pointers
- Suitable for scenes with thousands of nodes

## Contributing

Contributions are welcome! Please ensure:

- Code follows existing style conventions
- New features include tests
- Documentation is updated

## Acknowledgments

Built with [PEGTL](https://github.com/taocpp/PEGTL) - Parsing Expression Grammar Template Library
