---
name: cpp-result-status-ref
description: Design or review C++23 function return contracts for recoverable failures using std::expected and an error-only Status. Use for requests about 返回状态、错误返回、失败传播, or choosing between a value, optional, Result, and exceptions. Not for modeling the states of a business object itself.
---

# C++23 Result and Status

Use `template <typename T> using Result = std::expected<T, Status>;` when a caller needs either a value or a reason the operation could not finish. Reuse the target project's existing result type if it already expresses that contract; do not introduce a competing one or migrate unrelated APIs.

## Choose the return type

| What can the caller observe? | Return |
| --- | --- |
| The operation completes and produces a domain outcome, even a negative one | `T`, e.g. `Verdict::Reject` |
| A value may legitimately be absent; no failure reason is needed | `std::optional<T>` |
| The operation can fail instead of producing a value | `Result<T>` |
| An action can fail but has no success payload | `Result<void>`; `return {};` on success |
| A lookup can succeed without a match **and** fail to complete | `Result<std::optional<T>>`; `return std::nullopt;` means a successful miss |

Decide from the caller's contract: a missing *required* file may be `NotFound`; a missing *optional* setting may be a successful miss. If the API promises `Result` for a failure, do not also throw for that same condition. `std::expected` does not catch exceptions. Translate specific exceptions at an integration boundary only if the API promises that translation.

## Implement and propagate

For a project without an equivalent type, use [assets/status.cppm](assets/status.cppm) as the reference implementation. In a module project adapt the module name/namespace to its conventions. In a header project keep the same API, replace module declarations with the corresponding standard headers (`<expected>`, `<format>`, `<string>`, `<string_view>`), and use the project's header guard convention.

`Status` describes **one failure**, never success. Do not add `OK`, a default constructor, or a boolean conversion to it. Its scoped codes are `kNotFound` (1), `kCorruption` (2), `kNotSupported` (3), `kInvalidArgument` (4), and `kIOError` (5). Choose by cause, not by what the caller might do next: inability to read bytes is not corruption of successfully read bytes.

```cpp
enum class Verdict { Pass, Reject };

[[nodiscard]] Result<Verdict> Inspect(std::span<int const> samples, int limit) {
    if (samples.empty()) {
        return std::unexpected{ Status::InvalidArgument("samples", "empty") };
    }
    return std::ranges::max(samples) > limit ? Verdict::Reject : Verdict::Pass;
}
```

Test `if (!result)` before accessing `.error()`; on success use `*result`. If an intermediate layer cannot act on the error, return the original result when its type matches. Across different success types, use `return std::unexpected{ std::move(result).error() };` for a non-const local that is no longer needed. Let the caller decide whether to retry, skip, log, or stop. Check `code()` or `IsNotFound()` and its siblings; never branch on `ToString()`.

## Preserve the diagnostic contract

- `Status` owns its message. `message()` borrows a `std::string_view` from an lvalue only. Keep the owning status/result alive and unchanged, or copy the text. `ToString()` returns an owning string with the category prefix.
- Each factory takes either `(message)` or `(message, detail)`. Supplying `""` as the detail still appends `": "`; omitting it does not. When adding a code, extend the factory, predicate, and name mapping together. Keep the unknown-code fallback **after** the `switch` so missing cases can be diagnosed.
- If a caller needs every field validation failure at once, give it a structured error collection rather than joining diagnostics into one string. When adapting the reference, test success, each caller-visible error category, and propagation without loss of category or detail; keep temporary-message access rejected at compile time.
