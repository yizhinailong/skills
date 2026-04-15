---
name: cpp-style-ref
description: 为 cpp 项目应用编码风格。适用于编写或审查 C++ 代码、命名标识符、组织 .cppm/.cpp 文件，或用户提及 C++ 风格时。
---

# cpp-style-ref

cpp 项目的 Modern/Module C++ 风格参考。C++23，使用 `import std`。

## 快速参考

### 命名

| 种类         | 风格                 | 示例                             |
| ------------ | -------------------- | -------------------------------- |
| 类型/类      | PascalCase（大驼峰） | `StyleRef`, `HttpServer`         |
| 对象/成员    | snake_case（下划线） | `file_name`, `config_text`       |
| 函数         | snake_case（下划线） | `load_config_file()`, `parse_()` |
| 私有         | `m_` 前缀            | `m_file_name`, `m_config_text`   |
| 常量         | UPPER_SNAKE          | `MAX_SIZE`, `DEFAULT_TIMEOUT`    |
| 全局         | `g_` 前缀            | `g_style_ref`                    |
| 静态         | `s_` 前缀            | `s_style_ref`                    |
| 命名空间     | 全小写               | `mcpplibs`, `mylib`              |
| 多级命名空间 | 全小写               | `mcpp::style_ref`, `mcpp::utils` |
| 枚举         | PascalCase（大驼峰） | `enum class StyleRef { ... };`   |

### 模块基础

- 使用 `import std` 替代 `#include <print>` 和 `#include <xxx>`
- 使用 `.cppm` 作为模块接口；分离实现时用 `.cpp`
- `export module module_name;` — 模块声明
- `export import :partition;` — 导出分区
- `import :partition;` — 内部分区（不导出）

### 模块结构

```
// .cppm
export module a;

export import a.b;
export import :a2;   // 可导出分区

import std;
import :a1;          // 内部分区
```

### 模块命名

- 模块：`topdir.subdir.filename`（如 `a.b`, `a.c`）
- 分区：`module_name:partition`（如 `a:a1`, `a.b:b1`）
- 用目录路径区分同名：`a/c.cppm` → `a.c`，`b/c.cppm` → `b.c`

### 类布局

```cpp
class StyleRef {
private:
    std::string m_file_name;  // 数据成员带 m_ 前缀

public:  // Big Five
    StyleRef() = default;
    StyleRef(const StyleRef&) = default;
    // ...

public:  // 公有接口
    void LoadConfigFile(std::string file_name);  // 函数大驼峰，参数下划线

private:
    void loadConfigFile(std::string config);  // 私有函数小驼峰
};
```

### 实践规则

- **初始化**：用 `{}` — `int n { 42 }`，`std::vector<int> v { 1, 2, 3 }`
- **字符串**：只读参数用 `std::string_view`
- **错误**：用 `std::optional` / `std::expected` 替代 int 错误码
- **内存**：用 `std::unique_ptr`、`std::shared_ptr`；避免裸 `new`/`delete`
- **RAII**：将资源与对象生命周期绑定
- **auto**：用于迭代器、lambda、复杂类型；需要明确表达意图时保留显式类型
- **宏**：优先用 `constexpr`、`inline`、`concept` 替代宏

### 接口与实现

两种写法均支持。

**写法 A：合并** — 接口与实现同在一个 `.cppm` 中：

```cpp
// mylib.cppm
export module mylib;

export auto add(int a, int b) -> int {
    return a + b;
}
```

**写法 B：分离** — 接口在 `.cppm`，实现在 `.cpp`（编译期隐藏实现）：

```cpp
// error.cppm（接口）
export module error;

export struct Error {
    void test();
};
```

```cpp
// error.cpp（实现）
module error;

import std;

auto Error::test() -> void {
    std::println("Hello");
}
```

简单模块用写法 A；需隐藏实现或减少编译依赖时用写法 B。

## 适用场景

- 编写新的 C++ 模块代码（`.cppm`、`.cpp`）
- 审查或重构 mcpp 项目中的 C++ 代码
- 用户询问「mcpp 风格」「module C++ 风格」或「现代 C++ 惯例」
