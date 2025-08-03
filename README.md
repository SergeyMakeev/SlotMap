# Slot Map


[![License](https://img.shields.io/github/license/SergeyMakeev/CircularBuffer)](LICENSE)
[![ci](https://github.com/SergeyMakeev/CircularBuffer/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/CircularBuffer/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/SergeyMakeev/slot_map/graph/badge.svg?token=3GRAFTRYQU)](https://codecov.io/gh/SergeyMakeev/slot_map)

A Slot Map is a high-performance associative container with persistent unique keys to access stored values. It's designed for performance-critical applications where stable references, O(1) operations, and memory efficiency are essential.

## Table of Contents

- [What is a Slot Map?](#what-is-a-slot-map)
- [Key Features](#key-features)
- [Quick Start](#quick-start)
- [Building](#building)
- [API Reference](#api-reference)
- [Key Types](#key-types)
- [Advanced Features](#advanced-features)
- [Performance](#performance)
- [Implementation Details](#implementation-details)
- [Thread Safety](#thread-safety)
- [Examples](#examples)
- [References](#references)

## What is a Slot Map?

A Slot Map solves the problem of storing collections of objects that need stable, safe references but have no clear ownership hierarchy. Unlike `std::unordered_map` where you provide keys, a slot map **generates** unique keys when you insert values.

**Key differences from `std::unordered_map`:**
- Slot map generates keys for you (no key collisions)
- Guaranteed O(1) insertion, removal, and access
- Memory-stable pointers (values never move in memory)
- Automatic key invalidation when items are removed
- Efficient memory layout with page-based allocation

**Perfect for:**
- Game entities and component systems
- Resource management (textures, sounds, etc.)
- Object pools and handle-based systems
- Any scenario requiring safe, stable references

## Key Features

- **O(1) Performance**: Insertion, removal, and access are all O(1) in best, worst, and average case
- **Memory Stability**: Pointers to values remain valid until explicitly removed
- **Safe References**: Keys automatically become invalid when items are removed
- **Memory Efficient**: Page-based allocation prevents memory fragmentation
- **Type Safety**: Keys are typed to prevent mixing between different slot maps
- **Iteration Support**: Both value-only and key-value iteration
- **Custom Allocators**: Support for custom memory allocators
- **Header Only**: Single header file, easy to integrate

## Quick Start

```cpp
#include "slot_map.h"

// Create a slot map for strings
dod::slot_map<std::string> strings;

// Insert values and get unique keys
auto red_key = strings.emplace("Red");
auto green_key = strings.emplace("Green");
auto blue_key = strings.emplace("Blue");

// Access values using keys
const std::string* red_value = strings.get(red_key);
if (red_value) {
    printf("Color: %s\n", red_value->c_str());  // Output: Color: Red
}

// Remove a value
strings.erase(green_key);

// Check if keys are still valid
printf("Green exists: %d\n", strings.has_key(green_key));  // Output: 0
printf("Blue exists: %d\n", strings.has_key(blue_key));    // Output: 1

// Iterate over all values
for (const auto& color : strings) {
    printf("Color: %s\n", color.c_str());
}

// Iterate over key-value pairs
for (const auto& [key, color] : strings.items()) {
    printf("Key: %" PRIslotkey ", Color: %s\n", uint64_t(key), color.get().c_str());
}
```

## Building

### Requirements
- C++17 or later
- CMake 3.10 or later (for building tests)

### Header-Only Integration
Simply include the header file in your project:

```cpp
#include "slot_map/slot_map.h"
```

### Building Tests
```bash
git clone https://github.com/SergeyMakeev/slot_map.git
cd slot_map
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
./SlotMapTest01
./SlotMapTest02
./SlotMapTest03
./SlotMapTest04
```

### Custom Memory Allocator
Define custom allocator macros before including the header:

```cpp
#define SLOT_MAP_ALLOC(sizeInBytes, alignment) your_aligned_alloc(alignment, sizeInBytes)
#define SLOT_MAP_FREE(ptr) your_free(ptr)
#include "slot_map.h"
```

## API Reference

### Core Operations

#### `emplace(Args&&... args) -> key`
Constructs element in-place and returns a unique key.
```cpp
auto key = slot_map.emplace("Hello", "World");  // Construct string from args
```

#### `get(key k) -> T*` / `get(key k) const -> const T*`
Returns pointer to value or `nullptr` if key is invalid.
```cpp
const std::string* value = slot_map.get(key);
```

#### `has_key(key k) const -> bool`
Returns `true` if the key exists and is valid.
```cpp
if (slot_map.has_key(key)) { /* key is valid */ }
```

#### `erase(key k)`
Removes element if key exists. Key becomes invalid.
```cpp
slot_map.erase(key);
```

#### `pop(key k) -> std::optional<T>`
Removes and returns the value if key exists.
```cpp
auto value = slot_map.pop(key);  // Returns optional<T>
```

### Container Operations

#### `size() const -> size_type`
Returns number of elements.

#### `empty() const -> bool`
Returns `true` if container is empty.

#### `clear()`
Removes all elements but keeps allocated memory. Invalidates all keys.

#### `reset()`
Removes all elements and releases memory. **Warning**: Only call when no keys are in use.

#### `swap(slot_map& other)`
Exchanges contents with another slot map.

### Iteration

#### Value iteration
```cpp
for (const auto& value : slot_map) {
    // Process value
}
```

#### Key-value iteration
```cpp
for (const auto& [key, value] : slot_map.items()) {
    // Process key and value
}
```

### Debug and Statistics

#### `debug_stats() const -> Stats`
Returns internal statistics (O(n) complexity).
```cpp
auto stats = slot_map.debug_stats();
printf("Active items: %u\n", stats.numAliveItems);
```

## Key Types

### 64-bit Keys (Default)
```cpp
dod::slot_map<T>        // Uses 64-bit keys
dod::slot_map64<T>      // Explicit 64-bit keys
```

| Component | Bits | Range |
|-----------|------|-------|
| Tag       | 12   | 0..4,095 |
| Version   | 20   | 0..1,048,575 |
| Index     | 32   | 0..4,294,967,295 |

### 32-bit Keys
```cpp
dod::slot_map32<T>      // Uses 32-bit keys
```

| Component | Bits | Range |
|-----------|------|-------|
| Tag       | 2    | 0..3 |
| Version   | 10   | 0..1,023 |
| Index     | 20   | 0..1,048,575 |

### Key Operations
```cpp
auto key = slot_map.emplace(value);

// Type-safe: this won't compile
// slot_map<int>::key int_key = int_map.emplace(42);
// string_map.get(int_key);  // Compiler error!

// Convert to/from raw numeric type
uint64_t raw_key = key;                    // Implicit conversion
slot_map<T>::key restored_key{raw_key};    // Explicit construction
```

## Advanced Features

### Tags
Keys can store small amounts of user data:

```cpp
auto key = slot_map.emplace("Value");
key.set_tag(42);                    // Store application data
auto tag = key.get_tag();           // Retrieve: tag == 42

// Tag limits:
// 64-bit keys: 0..4,095 (12 bits)
// 32-bit keys: 0..3 (2 bits)
```

### Custom Page Size
Adjust memory allocation granularity:

```cpp
// Default page size is 4096 elements
dod::slot_map<T, dod::slot_map_key64<T>, 8192> large_pages;
dod::slot_map<T, dod::slot_map_key64<T>, 1024> small_pages;
```

### Custom Free Indices Threshold
Control when slot indices are reused:

```cpp
// Default threshold is 64
dod::slot_map<T, dod::slot_map_key64<T>, 4096, 128> conservative_reuse;
```

## Performance

### Time Complexity
- **Insertion**: O(1) amortized
- **Removal**: O(1)
- **Access**: O(1)
- **Iteration**: O(n) where n is number of alive elements

### Memory Characteristics
- **Page-based allocation**: 4096 elements per page by default
- **Pointer stability**: Values never move once allocated
- **Memory efficiency**: Pages are released when all slots become inactive
- **Cache-friendly**: Sequential iteration over alive elements only

## Implementation Details

### Version Overflow Protection
When a slot's version counter reaches maximum value:
1. The slot is marked as inactive
2. No new keys will be generated for this slot
3. Guarantees no key collisions even with version wrap-around

### Free Index Management
- Recently freed slots aren't immediately reused
- Minimum threshold (default 64) prevents rapid version increments
- Balances memory usage with collision avoidance

### Page Management
- Elements are allocated in pages (default 4096 elements)
- Pages are released when all slots in a page become inactive
- Provides memory stability and prevents fragmentation

## Thread Safety

**Slot maps are NOT thread-safe.** External synchronization is required for:
- Concurrent access from multiple threads
- Reader-writer scenarios

For thread-safe usage:
```cpp
std::shared_mutex mutex;
dod::slot_map<T> slot_map;

// Reader
{
    std::shared_lock lock(mutex);
    auto value = slot_map.get(key);
}

// Writer  
{
    std::unique_lock lock(mutex);
    auto key = slot_map.emplace(value);
}
```

## Examples

### Entity Component System
```cpp
struct Transform { float x, y, z; };
struct Health { int hp, max_hp; };

dod::slot_map<Transform> transforms;
dod::slot_map<Health> healths;

// Create entity
auto entity_id = generate_entity_id();
auto transform_key = transforms.emplace(Transform{0, 0, 0});
auto health_key = healths.emplace(Health{100, 100});

// Store keys in entity
register_component(entity_id, transform_key);
register_component(entity_id, health_key);
```

### Resource Management
```cpp
dod::slot_map<Texture> textures;
dod::slot_map<Sound> sounds;

class ResourceManager {
    using TextureHandle = dod::slot_map<Texture>::key;
    using SoundHandle = dod::slot_map<Sound>::key;
    
    TextureHandle load_texture(const std::string& path) {
        return textures.emplace(load_texture_from_file(path));
    }
    
    const Texture* get_texture(TextureHandle handle) {
        return textures.get(handle);
    }
};
```

### Object Pool with Versioning
```cpp
class BulletPool {
    struct Bullet { float x, y, dx, dy; bool active; };
    dod::slot_map<Bullet> bullets;
    
public:
    using BulletHandle = dod::slot_map<Bullet>::key;
    
    BulletHandle spawn(float x, float y, float dx, float dy) {
        return bullets.emplace(Bullet{x, y, dx, dy, true});
    }
    
    void update() {
        for (auto& bullet : bullets) {
            if (bullet.active) {
                bullet.x += bullet.dx;
                bullet.y += bullet.dy;
            }
        }
    }
    
    void destroy(BulletHandle handle) {
        bullets.erase(handle);  // Handle becomes invalid automatically
    }
};
```

## References

- Sean Middleditch: [Data Structures for Game Developers: The Slot Map](https://web.archive.org/web/20180121142549/http://seanmiddleditch.com/data-structures-for-game-developers-the-slot-map/), 2013
- Niklas Gray: [Building a Data-Oriented Entity System](http://bitsquid.blogspot.com/2014/08/building-data-oriented-entity-system.html), 2014
- Noel Llopis: [Managing Data Relationships](https://gamesfromwithin.com/managing-data-relationships), 2010
- Stefan Reinalter: [Adventures in Data-Oriented Design - External References](https://blog.molecular-matters.com/2013/07/24/adventures-in-data-oriented-design-part-3c-external-references/), 2013
- Niklas Gray: [Managing Decoupling Part 4 - The ID Lookup Table](https://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html), 2011
- Sander Mertens: [Making the Most of ECS Identifiers](https://ajmmertens.medium.com/doing-a-lot-with-a-little-ecs-identifiers-25a72bd2647), 2020
- Michele Caini: [ECS back and forth. Part 9 - Sparse Sets and EnTT](https://skypjack.github.io/2020-08-02-ecs-baf-part-9/), 2020
- Andre Weissflog: [Handles are the Better Pointers](https://floooh.github.io/2018/06/17/handles-vs-pointers.html), 2018
- Allan Deutsch: [C++Now 2017: "The Slot Map Data Structure"](https://www.youtube.com/watch?v=SHaAR7XPtNU), 2017
- Jeff Gates: [Init, Update, Draw - Data Arrays](https://greysphere.tumblr.com/post/31601463396/data-arrays), 2012
- Niklas Gray: [Data Structures Part 1: Bulk Data](https://ourmachinery.com/post/data-structures-part-1-bulk-data/), 2019
