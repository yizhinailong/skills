# cpp-style-ref 参考

详细风格规则，C++23，使用 `import std`。

## 一、标识符命名

### 1.0 类型 — PascalCase（大驼峰）

```cpp
struct StyleRef {
    using FileNameType = std::string;
};
enum class HttpStatus { Ok, NotFound };
```

### 1.1 对象/数据成员 — snake_case（下划线）

```cpp
std::string file_name;
int max_retry_count;
```

### 1.2 函数

公有函数使用 PascalCase（大驼峰），私有函数使用 camelCase（小驼峰）：

```cpp
class StyleRef {
public:
    void LoadConfigFile(std::string file_name);  // 公有大驼峰
private:
    void loadConfigFile(std::string config);     // 私有小驼峰
};
```

自由函数使用 snake_case（下划线）：

```cpp
void load_config_file(const std::string& file_name);
auto parse() -> void;
```

### 1.3 私有 — `m_` 前缀

私有数据成员使用 `m_` 前缀：

```cpp
private:
    std::string m_file_name;
    int m_config_text;
```

### 1.4 其他前缀

- 静态成员：`s_` 前缀 — `s_style_ref`
- 全局变量：`g_` 前缀 — `g_style_ref`、`g_debug`

### 1.5 常量 — UPPER_SNAKE

```cpp
constexpr int MAX_SIZE { 1024 };
constexpr auto DEFAULT_TIMEOUT { 30 };
```

### 1.6 命名空间 — 全小写

```cpp
namespace mcpplibs { /* ... */ }
namespace mcpp::style_ref { /* ... */ }
```

### 1.7 空格

运算符两侧加空格以增强可读性：`T x { ... }`、`int n { 42 }`。

### 1.8 模板命名

遵循类/函数命名风格。

---

## 二、模块化

### 模块文件结构

```cpp
module;              // 可选的全局模块片段
#include <xxx>       // 需要传统头文件时

export module module_name;
// export import :partition;
// import :partition;

import std;
import xxx;

export auto Add(int a, int b) -> int {
    return a + b;
}
```

### .cppm 与 .cpp

使用 `.cppm` 作为模块接口。用 `export` 关键字导出：

```cpp
export module mcpplibs;

export auto Add(int a, int b) -> int {
    return a + b;
}
```

### 接口与实现

**写法 A：合并** — 接口与实现在同一 `.cppm`：

```cpp
// mylib.cppm
export module mylib;

export auto Add(int a, int b) -> int {
    return a + b;
}
```

**写法 B：分离** — 接口在 `.cppm`，实现在 `.cpp`（编译期隐藏实现）：

```cpp
// error.cppm（接口）
export module error;

export struct Error {
    auto Test() -> void;
};
```

```cpp
// error.cpp（实现）
module error;

import std;

auto Error::Test() -> void {
    std::println("Hello");
}
```

简单模块用写法 A；需隐藏实现或减少编译依赖时用写法 B。

### 多文件模块

```
a/
├── a1.cppm     # module a:a1（内部分区）
├── a2.cppm     # export module a:a2
├── b/
│   ├── b1.cppm # export module a.b:b1
│   └── b2.cppm # export module a.b:b2
├── b.cppm      # export module a.b
a.cppm          # export module a
```

- **可导出分区**：`export module a:a2;` — 可被重新导出
- **内部分区**：`module a:a1;` — 不导出，仅模块内部使用

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
// a/a1.cppm — 内部分区
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
// a/a2.cppm — 可导出分区
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

### 模块命名规则

- 模块：`topdir.subdir.filename`（如 `a.b`, `a.c`）
- 分区：`module_name:partition`（如 `a:a1`, `a.b:b1`）
- 用目录路径区分同名：`a/c.cppm` → `a.c`，`b/c.cppm` → `b.c`

### 向前兼容

将传统 C/C++ 头文件封装到兼容模块中：

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

### 其他

- 优先用 `constexpr`、`inline`、`concept` 替代宏
- 模板的静态成员：使用 `inline static`（C++17）确保单一定义

---

## 三、类布局

```cpp
class StyleRef {
private:
    std::string m_file_name;  // 数据成员带 m_ 前缀

public:  // Big Five
    StyleRef() = default;
    StyleRef(const StyleRef&) = default;
    StyleRef(StyleRef&&) = default;
    auto operator=(const StyleRef&) -> StyleRef& = default;
    auto operator=(StyleRef&&) -> StyleRef& = default;
    ~StyleRef() = default;

public:  // 公有接口
    void LoadConfigFile(std::string file_name);  // 函数大驼峰，参数下划线

private:
    void loadConfigFile(std::string config);  // 私有函数小驼峰
};
```

---

## 四、构建配置

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

-- 分离式模块需要单独 target
target("error", function ()
    set_kind("static")

    add_files("src/error.cppm")
    add_files("src/error.cpp")
end)
```

### CMake + vcpkg（.hpp/.cpp）

**CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.25)

project(cpp-style-ref LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- error 库（分离式：.hpp 声明 + .cpp 实现）---
add_library(error STATIC
    src/error.cpp
)
target_include_directories(error PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- 主程序 ---
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

**文件结构**

```
src/
├── main.cpp
├── error.hpp           # struct Error 声明
├── error.cpp           # Error 实现
└── a/
    ├── a1.hpp          # internal — 不属于公共 API
    ├── a1.cpp
    ├── a2.hpp
    ├── a2.cpp
    └── b/
        ├── b1.hpp
        ├── b1.cpp
        ├── b2.hpp
        └── b2.cpp
```

**error.hpp** — 声明

```cpp
#pragma once

struct Error {
    auto Test() -> void;
};
```

**error.cpp** — 实现

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

**CMakePresets.json**（vcpkg 工具链集成）

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

构建命令：

```bash
cmake --preset debug
cmake --build --preset debug
```

---

## 五、实践参考

### auto

用于迭代器、lambda、复杂类型。显式类型更清晰时避免使用。

### 花括号初始化

`int n { 42 }`、`std::vector<int> v { 1, 2, 3 }`、`Point p { 10, 20 }`。

### 智能指针

`std::make_unique`、`std::make_shared`；避免裸 `new`/`delete`。

### string_view

用于只读字符串参数。不拥有数据，调用方需保证底层数据有效。

### optional / expected

- `std::optional`：可有可无的值
- `std::expected`（C++23）：成功返回值或错误

### RAII

将资源与对象生命周期绑定。使用 `std::fstream`、`std::lock_guard` 等。
