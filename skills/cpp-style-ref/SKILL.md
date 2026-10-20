---
name: cpp-style-ref
description: "C++ coding style reference for mcpp C++23 modules, xmake modules, and CMake + vcpkg header/source projects. Covers naming, formatting, Doxygen comments, and module organization. Use for new C++ code, requested style reviews or format changes, or questions about C++ style; preserve existing project conventions unless style changes are requested."
---

# cpp-style-ref

C++ coding style reference. Read [reference.md](reference.md) for detailed rules, then select only the reference directory matching the project's build system:

- **mcpp** → [references-mcpp/](references-mcpp/) (C++23 modules, combined declarations and implementations in `.cppm`)
- **xmake** → [references-xmake/](references-xmake/) (modules, `.cppm` / `.cpp`; the example targets C++26 for `import std;` support)
- **CMake + vcpkg** → [references-cmake-vcpkg/](references-cmake-vcpkg/) (traditional headers, `.hpp` / `.cpp`)

## Quick Reference

### Naming

| Category          | Style         | Examples                         |
| ----------------- | ------------- | -------------------------------- |
| Types/Classes     | PascalCase    | `StyleRef`, `HttpServer`         |
| Objects/Members   | snake_case    | `file_name`, `config_text`       |
| Public methods    | PascalCase    | `LoadConfigFile()`, `Parse()`    |
| Private functions | camelCase     | `loadConfigFile()`, `parse()`    |
| Free helpers      | snake_case    | `load_config_file()`, `parse()`  |
| Private members   | `m_` prefix   | `m_file_name`, `m_config_text`   |
| Constants         | UPPER_SNAKE   | `MAX_SIZE`, `DEFAULT_TIMEOUT`    |
| Globals           | `g_` prefix   | `g_style_ref`                    |
| Statics           | `s_` prefix   | `s_style_ref`                    |
| Namespaces        | all lowercase | `mcpplibs`, `mylib`              |
| Nested namespaces | all lowercase | `mcpp::style_ref`, `mcpp::utils` |
| Enums             | PascalCase    | `enum class StyleRef { ... };`   |
| Files             | snake_case    | `secure_string.cppm`, `error.hpp` |

Preserve established public API names, including PascalCase free entry points such as `Get()` and `GetAsync()`, standard protocol names such as `begin()` and `allocate()`, and upstream enum values such as `ErrorCode::OK`.

### Formatting and Documentation

- Use four spaces, no tabs, attached opening braces, and indentation inside namespaces; align access labels with the class declaration.
- Attach `*` and `&` to the type: `char const*`, `StyleRef&`. Use `T const&` for references to const.
- Use `value{ 42 }`, `Type{ value }`, and `value{}`; do not add a space before the opening initializer brace.
- Prefer trailing return types: `[[nodiscard]] auto Parse() const noexcept -> Result<void>`.
- Follow the existing `.clang-format`; do not impose a separate line limit. The reference style uses `ColumnLimit: 0`.
- Start C++ files with a multiline Doxygen block containing `@file` and `@brief`.
- Document types, functions, templates, and test helpers with multiline `/** ... */` blocks immediately before the declaration; put template documentation before `template`.
- Put `@brief`, `@tparam`, `@param`, `@return`, `@throws`, and `@note` on separate lines as applicable. Use trailing `///<` for members and enumerators, and ordinary `//` for implementation notes and namespace endings.

### Build Systems

- **mcpp** — `mcpp.toml`, C++23 modules with `import std;`, library code in `.cppm`, standalone `.cpp` tests (see `references-mcpp/`)
- **xmake** — module support, uses `.cppm` / `.cpp`; use the standard and toolchain configured by the project (see `references-xmake/`)
- **CMake + vcpkg** — traditional header organization, uses `.hpp` / `.cpp` (see `references-cmake-vcpkg/`)

### Module Basics

- Use `import std;` for the standard library in module projects; use standard headers in the CMake + vcpkg examples
- Use `.cppm` for module interfaces; keep mcpp library implementations in those files, and use `.cpp` implementation units only when the selected project supports that organization
- `export module module_name;` — module declaration
- `export import :partition;` — export partition
- `import :partition;` — import a partition without re-exporting it

### Module Structure

```cpp
/**
 * @file style_ref.cppm
 * @brief Re-export the library's public partitions.
 */
export module style_ref;

export import :config;
export import :error;
```

Inside a partition, use `import :other;` for implementation dependencies and `export import :other;` when the dependency is part of its public interface. Consumers import the primary module (`import style_ref;`), never a partition directly.

### Module Naming

- A module name and its partitions need not mirror directory paths: `src/utils/error.cppm` can declare `style_ref:error`
- Use dotted module names for separate modules when appropriate (e.g., `a.b`, `a.c` in the xmake example)
- Partitions: `module_name:partition` (e.g., `a:a1`, `a.b:b1`)
- Keep existing partition names when moving files between responsibility-based directories

### Class Layout

```cpp
/**
 * @brief Store owned configuration text.
 */
class StyleRef {
private:
    std::string m_config_text; ///< Owned configuration text.

public:
    /**
     * @brief Validate and copy configuration text.
     * @param config Text to copy; must not be empty.
     * @return Success or an invalid-configuration error.
     */
    [[nodiscard]] auto LoadConfig(std::string_view config) -> Result<void>;

private:
    /**
     * @brief Copy configuration text after validation.
     * @param config Validated text to copy.
     */
    auto loadConfig(std::string_view config) -> void;
};
```

Keep data and ownership state together, then constructors/special members, the public interface, and private helpers. Default or delete special members according to ownership; do not add them solely to fill out the Big Five.

### Practice Rules

- **Initialization**: use `{}` — `int n{ 42 }`, `std::vector<int> v{ 1, 2, 3 }`
- **Strings**: use `std::string_view` for read-only parameters
- **Errors**: use `std::optional` for an absent value and `std::expected<T, Error>` for recoverable failures; a `Result<T>` alias may name the latter
- **Memory**: use `std::unique_ptr`, `std::shared_ptr`; avoid raw `new`/`delete`
- **Checks**: before adding any conditional or defensive check, first verify whether the execution path can actually require it; avoid meaningless checks that cannot affect behavior
- **RAII**: bind resources to object lifetimes
- **Filesystem**: use `std::filesystem` directly when `import std;` already provides it
- **auto**: use for iterators, lambdas, complex types; keep explicit types when intent clarity matters
- **`const` placement**: prefer `T const&` over `const T&` — a single left-binding rule covers all types
- **Macros**: prefer `constexpr`, `inline`, `concept` over macros

### Interface and Implementation

Choose according to the project. The mcpp reference follows the combined style used by mcr; the xmake reference also demonstrates a separated implementation.

**Style A: Combined** — interface and implementation in a single `.cppm`:

```cpp
/**
 * @file mylib.cppm
 * @brief Provide a small arithmetic module.
 */
export module mylib;

/**
 * @brief Add two integers.
 * @param a First operand.
 * @param b Second operand.
 * @return Sum of the operands.
 */
export auto add(int a, int b) -> int {
    return a + b;
}
```

**Style B: Separated** — interface in `.cppm`, implementation in `.cpp` (hides implementation at compile time):

```cpp
/**
 * @file error.cppm
 * @brief Declare a diagnostic example.
 */
export module error;

/**
 * @brief Print a diagnostic message.
 */
export struct Error {
    /**
     * @brief Print a greeting.
     */
    auto Test() -> void;
};
```

```cpp
/**
 * @file error.cpp
 * @brief Implement the diagnostic example.
 */
module error;

import std;

/**
 * @brief Print a greeting.
 */
auto Error::Test() -> void {
    std::println("Hello");
}
```

Use Style A for mcpp library code. In other module projects, use Style B when the existing organization separates non-template implementations.

## Applicable Scenarios

- Writing new C++ code (modules `.cppm`/`.cpp` or headers `.hpp`/`.cpp`)
- Reviewing or refactoring C++ code in a project
- When the user asks about "C++ style", "module C++ style", or "modern C++ conventions"

## Limitations

For existing projects, this skill does not apply unless the user explicitly requests format changes. Do not proactively apply this style to existing code or suggest renames/refactoring.

Never create, modify, overwrite, or delete a `.clang-format` file. Treat any existing `.clang-format` as read-only and follow it when formatting C++ code.
