# cpp-style-ref Reference

Detailed style rules, C++23, using `import std`.

## 1. Identifier Naming

### 1.0 Types — PascalCase

```cpp
struct StyleRef {
    using FileNameType = std::string;
};
enum class HttpStatus { Ok, NotFound };
```

### 1.1 Objects/Data Members — snake_case

```cpp
std::string file_name;
int max_retry_count;
```

### 1.2 Functions

Public functions use PascalCase, private functions use camelCase:

```cpp
class StyleRef {
public:
    void LoadConfigFile(std::string file_name);  // public: PascalCase
private:
    void loadConfigFile(std::string config);     // private: camelCase
};
```

Free functions use snake_case:

```cpp
void load_config_file(const std::string& file_name);
auto parse() -> void;
```

### 1.3 Private Members — `m_` Prefix

Private data members use the `m_` prefix:

```cpp
private:
    std::string m_file_name;
    int m_config_text;
```

### 1.4 Other Prefixes

- Static members: `s_` prefix — `s_style_ref`
- Global variables: `g_` prefix — `g_style_ref`, `g_debug`

### 1.5 Constants — UPPER_SNAKE

```cpp
constexpr int MAX_SIZE { 1024 };
constexpr auto DEFAULT_TIMEOUT { 30 };
```

### 1.6 Namespaces — All Lowercase

```cpp
namespace mcpplibs { /* ... */ }
namespace mcpp::style_ref { /* ... */ }
```

### 1.7 Spacing

Add spaces around operators for readability: `T x { ... }`, `int n { 42 }`.

### 1.8 Template Naming

Follow class/function naming conventions.

---

## 2. Modules

### Module File Structure

```cpp
module;              // optional global module fragment
#include <xxx>       // when traditional headers are needed

export module module_name;
// export import :partition;
// import :partition;

import std;
import xxx;

export auto Add(int a, int b) -> int {
    return a + b;
}
```

### .cppm and .cpp

Use `.cppm` for module interfaces. Use the `export` keyword to export:

```cpp
export module mcpplibs;

export auto Add(int a, int b) -> int {
    return a + b;
}
```

### Interface and Implementation

**Style A: Combined** — interface and implementation in the same `.cppm`:

```cpp
// mylib.cppm
export module mylib;

export auto Add(int a, int b) -> int {
    return a + b;
}
```

**Style B: Separated** — interface in `.cppm`, implementation in `.cpp` (hides implementation at compile time):

```cpp
// error.cppm (interface)
export module error;

export struct Error {
    auto Test() -> void;
};
```

```cpp
// error.cpp (implementation)
module error;

import std;

auto Error::Test() -> void {
    std::println("Hello");
}
```

Use Style A for simple modules; use Style B when hiding implementation or reducing compile dependencies.

### Multi-File Modules

```
a/
├── a1.cppm     # module a:a1 (internal partition)
├── a2.cppm     # export module a:a2
├── b/
│   ├── b1.cppm # export module a.b:b1
│   └── b2.cppm # export module a.b:b2
├── b.cppm      # export module a.b
a.cppm          # export module a
```

- **Exportable partition**: `export module a:a2;` — can be re-exported
- **Internal partition**: `module a:a1;` — not exported, internal use only

```cpp
// a.cppm
export module a;

export import a.b;
export import :a2;

import std;
import :a1;

namespace a {
    export auto FuncA() -> void {
        std::println("Function FuncA from module a");
        FuncA1();
        FuncA2();
    }
} // namespace a
```

```cpp
// a/a1.cppm — internal partition
export module a:a1;

import std;

namespace a {
    export auto FuncA1() -> void;

    auto FuncA1() -> void {
        std::println("Function FuncA1 from module a:a1");
    }
} // namespace a
```

```cpp
// a/a2.cppm — exportable partition
export module a:a2;

import :a1;
import std;

namespace a {
    export auto FuncA2() -> void {
        std::println("Function FuncA2 from module a:a2");
        FuncA1();
    }
} // namespace a
```

```cpp
// a/b/b.cppm
export module a.b;

export import :b1;
export import :b2;

import std;

namespace a::b {
    export auto FuncAb() -> void {
        std::println("Function FuncAb from module a.b");
        FuncB1();
        FuncB2();
    }
} // namespace a::b
```

### Module Naming Rules

- Modules: `topdir.subdir.filename` (e.g., `a.b`, `a.c`)
- Partitions: `module_name:partition` (e.g., `a:a1`, `a.b:b1`)
- Use directory paths to distinguish identical names: `a/c.cppm` → `a.c`, `b/c.cppm` → `b.c`

### Forward Compatibility

Wrap traditional C/C++ headers in a compatibility module:

```cpp
module;

#include <lua.h>
// ...

export module lua;

export namespace lua {
    using lua_State = ::lua_State;
    // ...
}
```

### Miscellaneous

- Prefer `constexpr`, `inline`, `concept` over macros
- Template static members: use `inline static` (C++17) to ensure a single definition

---

## 3. Class Layout

```cpp
class StyleRef {
private:
    std::string m_file_name;  // data members with m_ prefix

public:  // Big Five
    StyleRef() = default;
    StyleRef(const StyleRef&) = default;
    StyleRef(StyleRef&&) = default;
    auto operator=(const StyleRef&) -> StyleRef& = default;
    auto operator=(StyleRef&&) -> StyleRef& = default;
    ~StyleRef() = default;

public:  // public interface
    void LoadConfigFile(std::string file_name);  // PascalCase function, snake_case parameter

private:
    void loadConfigFile(std::string config);  // camelCase for private functions
};
```

---

## 4. Build Configuration

### xmake

```lua
add_rules("mode.debug", "mode.release")

set_languages("c++26")
set_rundir("$(projectdir)")
set_policy("build.c++.modules", true)

target("mylib", function ()
    set_kind("binary")

    add_files("src/main.cpp")
    add_files("**.cppm")
end)

-- separated modules need a separate target
target("error", function ()
    set_kind("static")

    add_files("src/error.cppm")
    add_files("src/error.cpp")
end)
```

### CMake + vcpkg (.hpp/.cpp)

**CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.25)

project(cpp-style-ref LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- error library (separated: .hpp declaration + .cpp implementation) ---
add_library(error STATIC
    src/error.cpp
)
target_include_directories(error PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- main program ---
add_executable(cpp-style-ref
    src/main.cpp
    src/a/a1.cpp
    src/a/a2.cpp
    src/a/b/b1.cpp
    src/a/b/b2.cpp
)
target_include_directories(cpp-style-ref PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(cpp-style-ref PRIVATE error)
```

**File Structure**

```
src/
├── main.cpp
├── error.hpp           # struct Error declaration
├── error.cpp           # Error implementation
└── a/
    ├── a1.hpp          # internal — not part of public API
    ├── a1.cpp
    ├── a2.hpp
    ├── a2.cpp
    └── b/
        ├── b1.hpp
        ├── b1.cpp
        ├── b2.hpp
        ├── b2.cpp
```

**error.hpp** — declaration

```cpp
#pragma once

struct Error {
    auto Test() -> void;
};
```

**error.cpp** — implementation

```cpp
#include "error.hpp"
#include <print>

auto Error::Test() -> void {
    std::println("Hello");
}
```

**vcpkg.json**

```json
{
    "name": "cpp-style-ref",
    "version-string": "0.1.0",
    "dependencies": []
}
```

**CMakePresets.json** (vcpkg toolchain integration)

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "vcpkg",
            "hidden": true,
            "cacheVariables": {
                "CMAKE_TOOLCHAIN_FILE": {
                    "value": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
                    "type": "FILEPATH"
                }
            }
        },
        {
            "name": "debug",
            "displayName": "Debug",
            "inherits": "vcpkg",
            "binaryDir": "${sourceDir}/build/debug",
            "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" }
        },
        {
            "name": "release",
            "displayName": "Release",
            "inherits": "vcpkg",
            "binaryDir": "${sourceDir}/build/release",
            "cacheVariables": { "CMAKE_BUILD_TYPE": "Release" }
        }
    ]
}
```

Build commands:

```bash
cmake --preset debug
cmake --build --preset debug
```

---

## 5. Practice Reference

### auto

Use for iterators, lambdas, and complex types. Avoid when explicit types improve clarity.

### Brace Initialization

`int n { 42 }`, `std::vector<int> v { 1, 2, 3 }`, `Point p { 10, 20 }`.

### Smart Pointers

`std::make_unique`, `std::make_shared`; avoid raw `new`/`delete`.

### string_view

Use for read-only string parameters. Does not own data; the caller must ensure the underlying data remains valid.

### optional / expected

- `std::optional`: values that may or may not be present
- `std::expected` (C++23): returns a value on success or an error

### RAII

Bind resources to object lifetimes. Use `std::fstream`, `std::lock_guard`, etc.
