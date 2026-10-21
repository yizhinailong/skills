# cpp-style-ref Reference

Detailed style rules based on mcr's C++23 module code. Keep the same naming, formatting, and documentation conventions in each reference; select the source organization that matches the project's build system.

## 1. Identifier Naming

### 1.0 Types — PascalCase

Classes, structs, aliases, concepts, and enum types use PascalCase. Preserve standard-required aliases such as `value_type` and `iterator`.

```cpp
struct StyleRef;
using FileNameType = std::string;
enum class HttpStatus { Ok, NotFound };
```

### 1.1 Objects/Data Members — snake_case

```cpp
std::string file_name;
int max_retry_count{ 3 };
```

### 1.2 Functions

Public methods use PascalCase; private methods use camelCase. Free helpers use snake_case. Use trailing return types when a function has an ordinary return type.

```cpp
/**
 * @brief Store configuration text.
 */
class StyleRef {
public:
    /**
     * @brief Validate and copy configuration text.
     * @param config Text to copy.
     * @return Success or an invalid-configuration error.
     */
    [[nodiscard]] auto LoadConfig(std::string_view config) -> Result<void>;

private:
    /**
     * @brief Copy validated configuration text.
     * @param config Validated text to copy.
     */
    auto loadConfig(std::string_view config) -> void;
};

/**
 * @brief Parse configuration text into owned storage.
 * @param config Input to parse.
 * @return Parsed text or an error.
 */
auto parse_config(std::string_view config) -> Result<std::string>;
```

Preserve established public free-function names, such as mcr's `Get()` and `GetAsync()`. Standard protocol methods such as `begin()`, `end()`, `allocate()`, and coroutine hooks retain their required spelling.

### 1.3 Private Members — `m_` Prefix

```cpp
private:
    std::string m_file_name; ///< Owned configuration path.
    std::string m_config_text; ///< Owned configuration contents.
```

### 1.4 Other Prefixes

- Static data members and persistent function-local statics: `s_`, such as `s_instance`.
- Global variables: `g_`, such as `g_debug`.

### 1.5 Constants — UPPER_SNAKE

```cpp
constexpr int MAX_SIZE{ 1024 };
constexpr auto DEFAULT_TIMEOUT{ 30 };
```

Enum types use PascalCase. Preserve established enumerator spelling, including upstream constants such as `ErrorCode::OK` and `ErrorCode::FAILED_INIT`.

### 1.6 Namespaces — All Lowercase

Use lowercase names with underscores where needed. Indent the contents of every namespace by four spaces and retain the closing namespace comment.

```cpp
namespace style_ref::utils {

    constexpr int MAX_SIZE{ 1024 };

} // namespace style_ref::utils
```

### 1.7 Spacing and Formatting

- Use four spaces, never tabs. Opening braces remain on the declaration or control-statement line.
- Align `public:`, `protected:`, and `private:` with the class declaration; indent their contents one level.
- Attach pointer and reference symbols to the type: `T*`, `T&`, `T const&`, `char const*`.
- Write `int n{ 42 }`, `Type{ value }`, and `Type{}`. There is no space before an initializer brace; nonempty initializer braces have inner spaces.
- Put spaces around binary operators and after commas. Write `if (condition)`, `for (...)`, and `Parse(value)`.
- Place constructor initializer lists after `:` using braced member initialization. Follow the existing formatter when a list wraps.
- Do not impose a general line limit; mcr uses `ColumnLimit: 0`. Wrap long calls and expressions when needed for readability. Function signatures follow the 100-column wrapping rule in the next section.

An existing `.clang-format` is authoritative. Treat it as read-only: never create, modify, overwrite, or delete a `.clang-format` file.

### 1.8 Function Signature Wrapping

Wrap a function signature when its single-line form exceeds 100 columns, including leading indentation. Measure from the return type through the opening `{`, or through the terminating `;` for declarations; exclude trailing comments and an empty `{}` body. Apply the rule to definitions, declarations, constructors, destructors, and operators.

Use block style for the wrapped signature:

- Break immediately after `(`.
- Put exactly one parameter per line, indented one level from the declaration. Do not pack parameters together and do not add a trailing comma.
- Put `)` on its own line at the declaration's indentation, followed by qualifiers, the trailing return type, and `{` or `;`.
- Keep default arguments and inline parameter comments on their parameter's line. Keep the trailing return type on the `)` line; do not break before `->`.

```cpp
auto Result::get_header_value(
    std::string const& key,
    char const* def
) const -> std::string {
    ...
}
```

Do not wrap signatures that fit within 100 columns, empty parameter lists, lambda expressions, calls, or control statements. Leave `// clang-format off` regions unchanged. `.clang-format` sets `ColumnLimit: 0`, so clang-format preserves this layout but does not create it; apply the wrapping manually and run `clang-format -i <file>` to verify it is stable.

### 1.9 Template and File Naming

Template type parameters use descriptive PascalCase names or conventional `T`/`Ts`; value parameters follow object naming. Put Doxygen comments before the template declaration.

Source filenames use lowercase words separated by underscores, such as `secure_string.cppm`, `test_config.cpp`, and `error.hpp`.

---

## 2. Modules

### Module File Structure

Use `import std;` for the standard library in module projects. Traditional third-party headers, when required, belong in a global module fragment before the module declaration.

```cpp
/**
 * @file backend.cppm
 * @brief Provide a backend module partition.
 */
module;

#include <backend.h>

export module style_ref:backend;

import std;
import :error;

export namespace style_ref {

    // Public declarations and implementations.

} // namespace style_ref
```

Omit `module;` when no global-fragment headers or macros are needed. Use `export namespace` to group public declarations, and an ordinary namespace for internal helpers. Do not export implementation helpers just because they share a file with public types.

### .cppm and .cpp

- **mcpp:** keep library declarations and implementations together in `src/**/*.cppm`, as in mcr. Tests and executable entry points remain `.cpp` files.
- **xmake:** use `.cppm` for interfaces and partitions; a separate `.cpp` implementation unit is supported when the project's organization calls for it.
- **CMake + vcpkg:** follow the existing `.hpp`/`.cpp` layout and use standard headers; do not introduce modules into this reference.

### Interface and Implementation

**Style A: Combined** — the mcpp default, with definitions in the interface unit. See [references-mcpp/src/config.cppm](references-mcpp/src/config.cppm).

**Style B: Separated** — a module interface and a non-template implementation unit. See [references-xmake/src/error.cppm](references-xmake/src/error.cppm) and [references-xmake/src/error.cpp](references-xmake/src/error.cpp).

Keep the selected project's layout. Do not split mcpp library code into independent `.cpp` implementation units or move templates out of their required definition context.

### Multi-File Modules

Organize directories by responsibility. Partition names are independent of file paths:

```text
src/
├── style_ref.cppm       # export module style_ref;
├── config.cppm          # export module style_ref:config;
└── utils/
    └── error.cppm       # export module style_ref:error;
tests/
└── test_config.cpp      # import style_ref;
```

The primary interface re-exports the library's public partitions:

```cpp
/**
 * @file style_ref.cppm
 * @brief Re-export the library's public interfaces.
 */
export module style_ref;

export import :config;
export import :error;
```

Within a partition, use `import :other;` for implementation dependencies. Use `export import :other;` when the dependency must be visible through that partition's public interface. The mcr convention exposes all library component interfaces through its primary module; follow the chosen library's intended public surface.

External consumers use `import style_ref;`, never `import style_ref:config;`. Partitions can only be imported within their own named module.

An internal implementation partition declares `module style_ref:detail;` and is imported with `import :detail;`; it cannot be re-exported. An interface partition declares `export module style_ref:config;` and may be re-exported. The xmake example's `a:a1` is an interface partition used internally without being re-exported, rather than an implementation partition.

### Module Naming Rules

- Name a library's primary module after its public identity, such as `style_ref`.
- Name partitions `module_name:component`, such as `style_ref:error`, even when the file is under `src/utils/`.
- Preserve partition names when moving files; a directory reorganization does not require public module renames.
- Dotted names such as `a.b` identify separate named modules, as demonstrated by the xmake reference. Use them when the project intentionally exposes distinct modules.

### Header Interoperability

Keep C library includes and required macros in the global module fragment, as shown above. Preserve platform-specific handling required by the backend. Do not create compatibility wrappers solely to rename a namespace already available through `import std;`; use `std::filesystem` directly.

---

## 3. Class Layout

Group private data first, followed by constructors and special members, the public interface, and private helpers. Access sections may repeat when that grouping improves readability.

```cpp
/**
 * @brief Own validated configuration text.
 */
class StyleRef {
private:
    std::string m_config_text; ///< Owned text, independent of the input view.

public:
    /**
     * @brief Construct an empty configuration holder.
     */
    StyleRef() = default;

    /**
     * @brief Validate and copy configuration text.
     * @param config Text to copy; must not be empty.
     * @return Success or an invalid-configuration error.
     */
    [[nodiscard]] auto LoadConfig(std::string_view config) -> Result<void>;

    /**
     * @brief Borrow the currently stored text.
     * @return View valid until the holder is changed or destroyed.
     */
    [[nodiscard]] auto Text() const noexcept -> std::string_view;

private:
    /**
     * @brief Copy already validated text.
     * @param config Validated text to copy.
     */
    auto loadConfig(std::string_view config) -> void;
};
```

Default or delete copy/move operations according to ownership. Do not add all five special members when ordinary member types already provide the correct behavior. Use `explicit`, `[[nodiscard]]`, `const`, and `noexcept` when they express the actual contract; do not mark potentially allocating operations `noexcept`.

---

## 4. Build Configuration

Use the matching complete example rather than copying a different build system into an existing project.

### mcpp

See [references-mcpp/mcpp.toml](references-mcpp/mcpp.toml). The library entry is declared with `[lib]` and `path = "src/style_ref.cppm"`. The example has no external dependencies and follows mcr's C++23 `import std;` organization.

From `references-mcpp/`:

```sh
mcpp build
mcpp test
```

Tests are independent `tests/test_*.cpp` programs with their own `main()`. Successful tests return zero; failures return nonzero.

### xmake

See [references-xmake/xmake.lua](references-xmake/xmake.lua). The sample configures C++26 for its `import std;` toolchain and enables modules. The binary owns the `a`/`a.b` module files; the separated `error` module has its own static-library target, exposes its interface with `add_files("src/error.cppm", {public = true})`, and is linked using `add_deps("error")`.

From `references-xmake/`:

```sh
xmake
xmake run cpp-style-ref
```

### CMake + vcpkg (.hpp/.cpp)

See [references-cmake-vcpkg/CMakeLists.txt](references-cmake-vcpkg/CMakeLists.txt), [CMakePresets.json](references-cmake-vcpkg/CMakePresets.json), and [vcpkg.json](references-cmake-vcpkg/vcpkg.json).

Keep declarations in headers using `#pragma once` and definitions in `.cpp`. Include each implementation's own header and the standard headers it needs. The sample uses C++23 and provides matching configure/build presets.

With `VCPKG_ROOT` pointing to the existing vcpkg installation, run from `references-cmake-vcpkg/`:

```sh
cmake --preset debug
cmake --build --preset debug
```

---

## 5. Practice Reference

### auto and Return Types

Use `auto` for iterators, lambdas, and complex types; retain explicit types when they clarify intent. Use `auto Name(...) -> ReturnType` for functions with ordinary return types. Constructors, destructors, and conversion operators retain their language-required syntax.

### Brace Initialization

Prefer `int n{ 42 }`, `std::vector<int> v{ 1, 2, 3 }`, and `Point p{ 10, 20 }`. Preserve parentheses when needed to select the intended overload, such as constructing a vector of a given size rather than an initializer list.

### Smart Pointers and RAII

Use `std::make_unique` for exclusive ownership and `std::make_shared` for shared ownership. Bind resources to object lifetimes, using RAII wrappers and custom deleters for C handles. Raw pointers and views may express borrowing; avoid raw `new`/`delete` unless an existing lifecycle contract requires them.

### Conditional and Defensive Checks

Before adding a conditional, guard clause, null check, bounds check, state check, or other defensive validation, inspect the execution path and relevant contracts to determine whether the check can affect behavior.

Do not repeat checks already guaranteed by construction, prior validation, ownership, type constraints, or API invariants. Validate external inputs and fallible backend results at the boundary, and preserve required cleanup and platform checks.

### string_view

Use `std::string_view` for borrowed read-only string parameters. Document lifetimes when a view is stored or returned. Copy into owned storage when the data must outlive the call; do not pass `view.data()` to APIs requiring null termination without satisfying that contract.

### optional / expected

- Use `std::optional<T>` for a value that may be absent.
- Use `std::expected<T, Error>` or a `Result<T>` alias for recoverable operation failures, including `Result<void>`.
- Return `std::unexpected{ Error{ ... } }` on failure and `{}` for successful `Result<void>`.
- Inspect a result before accessing its value. Preserve diagnostics when propagating an error, using `std::unexpected{ std::move(result.error()) }` when ownership can transfer.
- Document exceptions that remain part of the contract; `std::expected` does not make allocations non-throwing.

### `T const&` — Consistent const Placement

Place `const` after the type it qualifies. Write `std::string const&`, `char const*`, and `T* const` as appropriate.

```cpp
using Pointer = int*;
// Pointer const is a const pointer; int const* points to a const int.
```

### Macros and Static Members

Prefer `constexpr`, `inline`, templates, and concepts over macros when they express the same behavior. Preserve macros required for platform and C-library interoperability. Use `inline static` for class static data defined in the class when appropriate.

---

## 6. Doxygen Documentation

Start C++ source, module, and header files with `@file` and `@brief`. Document public interfaces, internal helpers, and test helpers using the same multiline block layout:

```cpp
/**
 * @file result.cppm
 * @brief Provide an expected-based result alias.
 */
export module style_ref:result;

import std;
import :error;

export namespace style_ref {

    /**
     * @brief Represent an operation's value or error.
     * @tparam T Success value type; void means no value is returned.
     */
    template <typename T = void>
    using OperationResult = std::expected<T, Error>;

} // namespace style_ref
```

Keep `/**` and `*/` on their own lines, and begin content lines with ` * `. Even a single `@brief` uses the multiline form. Place the comment immediately before the declaration and before `template` or attributes when present.

Use `@tparam`, `@param`, `@return`, `@throws`, and `@note` only when relevant, with each tag on its own line. Describe ownership, borrowed lifetimes, preconditions, and remaining exceptions where they affect callers. Use trailing `///<` for members and enumerators; implementation explanations and namespace closing labels may use ordinary `//` comments.
