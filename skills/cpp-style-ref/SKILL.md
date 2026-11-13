---
name: cpp-style-ref
description: "C++ coding style reference for mcpp C++23 modules, xmake modules, and CMake + vcpkg header/source projects. Covers naming, formatting, module organization, and the Doxygen comment format used only when comments are requested. Use for new C++ code, requested style reviews or format changes, or questions about C++ style; preserve existing project conventions unless style changes are requested."
---

# cpp-style-ref

C++ coding style reference. Read [reference.md](reference.md) for detailed rules, then select only the reference directory matching the project's build system:

- **mcpp** → [references-mcpp/](references-mcpp/) (C++23 modules, interface `.cppm` and implementation `.cpp` units)
- **xmake** → [references-xmake/](references-xmake/) (modules, `.cppm` / `.cpp`; the example targets C++26 for `import std;` support)
- **CMake + vcpkg** → [references-cmake-vcpkg/](references-cmake-vcpkg/) (traditional headers, `.hpp` / `.cpp`)

The reference sources are fully commented to show the format for requested comments. Follow their code, and leave out their comments unless comments are requested (see [Comments](#comments)).

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

### Formatting

- Use four spaces, no tabs, attached opening braces, and indentation inside namespaces; align access labels with the class declaration.
- Attach `*` and `&` to the type: `char const*`, `StyleRef&`. Use `T const&` for references to const.
- Use `value{ 42 }`, `Type{ value }`, and `value{}`; do not add a space before the opening initializer brace.
- Prefer trailing return types: `[[nodiscard]] auto Parse() const noexcept -> Result<void>`.
- Follow the existing `.clang-format`; do not impose a general line limit. The reference style uses `ColumnLimit: 0`, but function signatures and `requires` clauses still follow the 100-column rules below.

### Comments

Write code without comments by default: no `@file` headers, Doxygen blocks, `///<` member comments, or `//` implementation notes. Add comments only when the user, project instructions such as `AGENTS.md`, or another active skill explicitly asks for them, and comment only what the request covers. Closing namespace comments such as `} // namespace style_ref` are part of the formatting and stay; so do `// clang-format off` directives. Do not delete existing comments, and keep them accurate when changing the code they describe.

When comments are requested, use this format:

- Start C++ files with a multiline Doxygen block containing `@file` and `@brief`.
- Document types, functions, templates, and test helpers with multiline `/** ... */` blocks immediately before the declaration; put template documentation before `template`.
- Put `@brief`, `@tparam`, `@param`, `@return`, `@throws`, and `@note` on separate lines as applicable. Use trailing `///<` for members and enumerators, and ordinary `//` for implementation notes. Never document a declaration, including a constant, with a `///` line comment.
- Align the `///<` comments of consecutive same-indentation lines one space after the longest code in the group; a blank line or a line without `///<` starts a new group.

```cpp
/**
 * @brief Store owned configuration text.
 */
class StyleRef {
public:
    /**
     * @brief Validate and copy configuration text.
     * @param config Text to copy; must not be empty.
     * @return Success or an invalid-configuration error.
     */
    [[nodiscard]] auto LoadConfig(std::string_view config) -> Result<void>;

private:
    std::string m_config_text; ///< Owned configuration text.
};
```

### Function Signature Wrapping

Wrap a signature when its single-line form, measured from the return type through the opening `{` or terminating `;` (including indentation, excluding trailing comments and an empty `{}` body), exceeds 100 columns. This applies to definitions, declarations, constructors, destructors, and operators. Break immediately after `(`, put one parameter per line indented one level with no trailing comma, and put `)` on its own line at the declaration's indentation followed by qualifiers, the trailing return type, and `{` or `;`. Keep default arguments and inline parameter comments with their parameter, and keep `->` on the `)` line.

```cpp
auto Result::get_header_value(
    std::string const& key,
    char const* def
) const -> std::string {
    ...
}
```

Constructors measure through the body's `{`, so the initializer list counts. A constructor that fits stays on one line. Otherwise block-wrap the parameters and continue the `)` line with qualifiers, ` : `, the whole initializer list, and `{` or `{}`. Never start a line with `:` or split the list.

```cpp
FormData(
    FormData&& other
) noexcept : fields{ std::move(other.fields) }, boundary{ std::move(other.boundary) }, params{ other.params } {
    other.params.remove_uploaded_files = false;
}
```

Do not wrap signatures that fit within 100 columns, empty parameter lists, lambdas, calls, or control statements. Leave `// clang-format off` regions unchanged. `ColumnLimit: 0` means clang-format preserves this layout but will not create it: wrap manually, then run `clang-format -i <file>` to verify the result is stable. Without a `.clang-format`, skip that run, because clang-format would fall back to LLVM style and rewrite the file.

### Requires Clauses

Put a `requires` clause on its own line at the indentation of its `template` line. When the clause exceeds 100 columns, break after each top-level `||` or `&&`, one operand per line, with continuation lines indented one level.

```cpp
template <typename T = std::string_view>
requires std::integral<T> ||
    std::floating_point<T> ||
    std::same_as<T, std::string> ||
    std::same_as<T, std::string_view>
[[nodiscard]] auto At(std::string_view key) const -> T;
```

### Build Systems

- **mcpp** — `mcpp.toml`, C++23 modules with `import std;`, library interfaces in `.cppm` with implementations in `.cpp`, and `mcpplibs.mtest` cases in `tests/*.cpp` (see `references-mcpp/`)
- **xmake** — module support, uses `.cppm` / `.cpp`; use the standard and toolchain configured by the project (see `references-xmake/`)
- **CMake + vcpkg** — traditional header organization, uses `.hpp` / `.cpp` (see `references-cmake-vcpkg/`)

### Module Basics

- Use `import std;` for the standard library in module projects; use standard headers in the CMake + vcpkg examples
- Use `.cppm` for module interfaces and `.cpp` module implementation units that declare `module style_ref;` for non-template definitions; this is the latest `mcpp new -t lib` layout
- `export module module_name;` — module declaration
- `export import :partition;` — export partition
- `import :partition;` — import a partition without re-exporting it

### Module Structure

```cpp
export module style_ref;

export import :config;
export import :error;
```

Inside a partition, use `import :other;` for implementation dependencies and `export import :other;` when the dependency is part of its public interface. Consumers import the primary module (`import style_ref;`), never a partition directly.

### Module Naming

- A module name and its partitions need not mirror directory paths: a source file grouped under `src/utils/` can still declare `style_ref:error`
- Use dotted module names for separate modules when appropriate (e.g., `a.b`, `a.c` in the xmake example)
- Partitions: `module_name:partition` (e.g., `a:a1`, `a.b:b1`)
- Keep existing partition names when moving files between responsibility-based directories

### Class Layout

```cpp
class StyleRef {
public:
    StyleRef() = default;

    [[nodiscard]] auto LoadConfig(std::string_view config) -> Result<void>;

    [[nodiscard]] auto Text() const noexcept -> std::string_view;

private:
    std::string m_config_text;

    auto loadConfig(std::string_view config) -> void;
};
```

Put the public section first: constructors/special members, then the public interface. Follow it with the private section: data and ownership state, then private helpers. Access sections may repeat when that grouping improves readability. Default or delete special members according to ownership; do not add them solely to fill out the Big Five.

### Practice Rules

- **Initialization**: use `{}` — `int n{ 42 }`, `std::vector<int> v{ 1, 2, 3 }`
- **Strings**: use `std::string_view` for read-only parameters
- **Errors**: use `std::optional` for an absent value and `std::expected<T, Error>` for recoverable failures; a `Result<T>` alias may name the latter
- **Memory**: use `std::unique_ptr`, `std::shared_ptr`; avoid raw `new`/`delete`
- **Checks**: before adding any conditional or defensive check, first verify whether the execution path can actually require it; avoid meaningless checks that cannot affect behavior
- **Function length**: keep a function around 100 lines; when one grows well beyond that, extract cohesive steps into named helpers, but do not split code only to meet the count
- **RAII**: bind resources to object lifetimes
- **Filesystem**: use `std::filesystem` directly when `import std;` already provides it
- **auto**: use for iterators, lambdas, complex types; keep explicit types when intent clarity matters
- **`const` placement**: prefer `T const&` over `const T&` — a single left-binding rule covers all types
- **Macros**: prefer `constexpr`, `inline`, `concept` over macros

### Interface and Implementation

Choose according to the project. The latest mcpp layout separates interface and implementation; the xmake reference also demonstrates a separated implementation.

**Style A: Separated** — declarations in `.cppm`, non-template definitions in a `.cpp` module implementation unit (hides implementation at compile time). This is the latest `mcpp new -t lib` layout:

```cpp
export module error;

export struct Error {
    auto Test() -> void;
};
```

```cpp
module error;

import std;

auto Error::Test() -> void {
    std::println("Hello");
}
```

**Style B: Combined** — interface and implementation in a single `.cppm`:

```cpp
export module mylib;

export auto add(int a, int b) -> int {
    return a + b;
}
```

Use Style A for mcpp library code. Use the combined style only for a project that already organizes its modules that way.

## Applicable Scenarios

- Writing new C++ code (modules `.cppm`/`.cpp` or headers `.hpp`/`.cpp`)
- Reviewing or refactoring C++ code in a project
- When the user asks about "C++ style", "module C++ style", or "modern C++ conventions"

## Limitations

For existing projects, this skill does not apply unless the user explicitly requests format changes. Do not proactively apply this style to existing code or suggest renames/refactoring.

Never create, modify, overwrite, or delete a `.clang-format` file. Treat any existing `.clang-format` as read-only and follow it when formatting C++ code.
