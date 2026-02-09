# STDL — Scene Tree Description Language

[![License](https://img.shields.io/github/license/RustamovHumoyunMirzo/stdl)](LICENSE)

---

## What is STDL?

**STDL** (Scene Tree Description Language) is a human-readable, powerful, and flexible data format designed specifically for describing scene/level trees in game engines.

It supports:

- **Hierarchical nodes** with types, names, and optional dual IDs (local `#id` and global `@id`)
- Rich **property types**: numbers, strings, booleans, arrays, and references
- **Explicit cross-node references** by type, name, and ID
- Nested scene graphs with reusable and referenceable nodes
- Comments and whitespace-friendly syntax for readability
- Strict yet expressive parsing rules for robust engine integration

---

## Why STDL?

Traditional scene formats (JSON, XML, YAML) often fall short for complex game data due to lack of references, weak typing, or verbosity. STDL is:

- **Designed for game engines** — intuitive node typing and IDs  
- **Human-readable and writable** — clean syntax, comments, and flexible whitespace  
- **Powerful references** — both local and global, typed for safety  
- **Lightweight and extensible** — easy to parse and extend with new features  

---

## Example STDL file

```stdl
scene v1

node player MyPlayer @1
{
    health = 100
    armor = 72.65
    skin = "player.mat"
    isOnGround = false

    list = [0, 1, 2, 42, 23, "232", true, <#12>]

    node mynode MyNode #12
    {
        description = "Nested node example"
    }
}
