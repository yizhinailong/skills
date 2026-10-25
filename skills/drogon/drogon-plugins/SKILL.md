---
name: drogon-plugins
description: "Drogon plugin library reference for drogon::Plugin<T> libraries built with CMake + vcpkg and consumed by other drogon apps. Covers plugin registration, lifecycle, and threading as implemented in drogon 1.9.13, config.json and secret handling, coroutine-only APIs returning std::expected, per-request HttpClient transports, add_subdirectory and find_package packaging, test programs, AGENTS.md and docs, and the implementation workflow. Use when creating, extending, or reviewing a drogon plugin library; preserve existing project conventions unless changes are requested."
---

# drogon-plugins

Build drogon plugin libraries: `drogon::Plugin<T>` classes that other drogon apps link, register in their config.json, and fetch with `app().getPlugin<T>()`. This skill records drogon's plugin behavior as implemented in drogon 1.9.13, and the conventions and workflow the user established in `drogon-llm`.

- Read [reference.md](reference.md) for drogon behavior with source locations, version notes, and the origin of each preference.
- Start a new plugin from [references-cmake-vcpkg/](references-cmake-vcpkg/), a buildable `drogon-echo` plugin whose tests pass offline.
- Write C++ with the `cpp-style-ref` skill (CMake + vcpkg variant) and commit messages with the `git-commit-style-ref` skill. This skill does not repeat their rules.

## Starting a New Plugin

Copy `references-cmake-vcpkg/` into the new repository, then rename:

| Skeleton | New plugin `drogon-<x>` |
| --- | --- |
| `drogon-echo` | Project, target, package, and `cmake/drogon-<x>-config.cmake.in` |
| `drogon_echo` | Namespace and `include/drogon_<x>/` |
| `DROGON_ECHO_` | CMake option prefix |
| `EchoClient`, `Echo`, `NewEchoClient()`, `EchoConfig` | `<X>Client` (plugin), `<X>` (client handle), `New<X>Client()`, `<X>Config` |
| `details::Messages`, `types/message.hpp`, `/messages` | The real API groups, types, and endpoints |

Replace the echo API, then update `config.json`, `AGENTS.md`, and `docs/drogon-plugin.md` to describe the real plugin. Copy `CMakePresets.json`, `justfile`, `.gitignore`, `.clang-format`, and `tests/CMakeLists.txt` unchanged; the `.clang-format` is the user's shared config and is not edited. Then run `just build`, `clang-format --dry-run -Werror` on every C++ file, and `just run` for each test.

```text
drogon-<x>/
├── CMakeLists.txt, CMakePresets.json, justfile, .clang-format, .gitignore
├── config.json                 # tracked; no secrets
├── AGENTS.md, CLAUDE.md        # CLAUDE.md only imports @AGENTS.md
├── cmake/drogon-<x>-config.cmake.in
├── docs/drogon-plugin.md
├── include/drogon_<x>/
│   ├── <x>_client.hpp          # <X>Config, <X> handle, <X>Client plugin
│   ├── error.hpp               # ErrorKind, Error, Result<T>
│   ├── details/<group>.hpp     # API groups held by value in the handle
│   └── types/<group>.hpp       # plain request and response structs
├── src/
│   ├── <x>_client.cpp
│   └── details/                # <group>.cpp, transport.hpp/.cpp (private)
└── tests/
    ├── CMakeLists.txt          # one executable per test_*.cpp
    └── test_config.cpp, test_<group>.cpp
```

## Plugin Class

- Derive `drogon::Plugin<<X>Client>` and keep the class default-constructible. Drogon registers it under its demangled, namespace-qualified name (`drogon_<x>::<X>Client`), and apps list that name under `plugins` with an optional `config` object and `dependencies` array.
- Keep drogon's override names `initAndStart` and `shutdown`; other public methods are PascalCase. Provide `Config() const noexcept -> <X>Config const&` and a `const` factory `New<X>Client() -> <X>` that returns the handle by value.
- `initAndStart` parses and validates the `config` object into the `m_config` member. Throw `std::invalid_argument` with a `drogon_<x>::<X>Client: ...` message for a missing or invalid value. Do not return `std::expected` here: the override returns `void`, and drogon ignores the outcome; the exception propagates out of `app().run()`.
- Do not block or await network I/O in `initAndStart`: it runs inside `run()` before any event loop is looping.
- The plugin class doc comment shows a config.json registration example, states that `getPlugin` works only after `run()` starts, and carries a `@note` on static linking: the app must reference the class, or the linker drops it and drogon logs `Plugin ... undefined!` and continues.

## Configuration

- Never read a secret from config.json. An optional `<name>_env` key (for example `api_key_env`) names the environment variable that holds it. Throw if that key is set and the variable is unset or empty; omitting the key means the endpoint needs no secret.
- Validate required keys in `initAndStart`. When adding a key, update `<X>Config`, `initAndStart`, the plugin class doc comment, `config.json`, and the configuration table in `docs/drogon-plugin.md`.
- Keep a tracked `config.json` at the repository root. Tests load it from the working directory, which `just run` sets to the repository root.

## Runtime API

- Offer only coroutines returning `drogon::Task<Result<T>>`. Provide no synchronous variant and add no `Coro` suffix. A caller that must block uses `drogon::sync_wait()` from a thread that runs no event loop.
- Take coroutine parameters by value. Before the first `co_await`, copy shared state such as `m_transport` into the coroutine frame, so a request survives the object that started it.
- Make the client handle a cheap value type. Its only shared state is one `std::shared_ptr<details::Transport>`; API groups are plain members, giving calls such as `llm.chat.completions.Create(...)`. Use pointers only when a group needs polymorphism or non-copyable state.
- When a well-known SDK exists for the remote API, mirror its group nesting, method names, and type names; `drogon-llm` mirrors the OpenAI Python SDK, including `extra_body` for vendor fields.
- Put request and response types in `types/` as plain structs filled with designated initializers. Use `std::optional` for optional fields, leave unset ones out of the request body, and give requests a `timeout` in seconds.
- Return runtime failures through `Result<T> = std::expected<T, Error>` from `error.hpp`. Catch drogon's exceptions inside the library and convert them; no exception leaves a runtime API.

## HTTP Transport

- Create a new `HttpClient` for each request on `trantor::EventLoop::getEventLoopOfCurrentThread()`, or on `app().getLoop()` when the calling thread runs no loop. Keep the `HttpClientPtr` in the coroutine frame until the response arrives. Never cache a shared client: drogon allows one in-flight request per client by default.
- Split `base_url` into the host passed to `newHttpClient()` and a path prefix, so URLs such as `https://host/v1` work. Send `Authorization: Bearer <key>` only when the key is non-empty.
- Map failures to `ErrorKind`: `HttpException` with `ReqResult::Timeout` to `Timeout` and other codes to `Connection`; non-2xx to `Status` with the HTTP status, the body, and the body's `error.message` when present; a non-JSON or malformed 2xx body to `ResponseValidation`.

## Tests

- Each `tests/test_*.cpp` builds into its own executable, run as `just run test_<name>`. Tests are plain programs that report through their exit code, with no framework or CTest.
- Load `config.json`, fetch the plugin in a beginning advice, run coroutines through `drogon::async_run`, record the result, and call `app().quit()`.
- A test may call the real service; document that it needs network access, credentials, or quota. Exercise failure paths as well, such as a bad key or an unreachable address.
- A test that serves its own endpoint must wait briefly before the first request, because listeners start only after the beginning advices.

## Build and Packaging

- Build a static library by default (`BUILD_SHARED_LIBS=ON` for shared) with the alias `drogon-<x>::drogon-<x>`. Install public headers through `FILE_SET HEADERS`; keep private headers in `src/` on the library's PRIVATE include path.
- Support both `add_subdirectory`/CPM and `just publish` + `find_package(drogon-<x> CONFIG REQUIRED)`. `DROGON_<X>_BUILD_TESTS` and `DROGON_<X>_INSTALL` default to `PROJECT_IS_TOP_LEVEL`. The package config calls `find_dependency(Drogon CONFIG)` and uses `SameMinorVersion`; `Drogon::Drogon` is linked PUBLIC.
- Never set `CMAKE_CXX_STANDARD` in the library; declare `target_compile_features(PUBLIC cxx_std_23)`. Right after finding Drogon, in both `CMakeLists.txt` and the package config, unset the empty `CMAKE_CXX_STANDARD` that drogon's `FindFilesystem.cmake` leaves behind.
- Collect sources and headers with `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`. Use CMake >= 4.0, Ninja, ccache, clang++ on Linux and clang-cl on Windows, presets named `<os>-<profile>`, and vcpkg in classic mode.

## Documentation

- `AGENTS.md` is English, with the sections Project state, Consuming the library, Tests, Commands, Build notes, and Code style. Record each design decision with its reason and the drogon source or documentation behind it, and update the file with every behavior change.
- `docs/drogon-plugin.md` is for developers who use the library: overview, integration, registration and configuration, lifecycle, usage, error handling, threads and event loops, coroutine rules, known limitations, and test programs. `drogon-llm` writes it in Chinese with identifiers in English.

## Implementation Workflow

- At a design fork, such as the error model, sync versus coroutine, pointer versus value, or which loop a coroutine resumes on, present the options with a recommendation and reasons, then wait for the user's choice.
- Verify claims about drogon against the installed drogon source or the official drogon documentation, and cite the file and line. When drogon's guidance contradicts an earlier choice, say so and follow drogon.
- When asked for a skeleton, write declarations, documentation, and empty bodies marked with `TODO`, and make it compile.
- If the user's code has a defect, such as a dangling reference, fix it and call it out instead of changing it silently.
- Before reporting, run `just build` without warnings, `clang-format --dry-run -Werror`, and the affected tests, including failure paths. State anything that was not run.
- Split commits finely, with the code commit followed by a separate `docs(AGENTS.md): ...` commit, and make each commit build on its own. Mark public API breaks, including a raised `cxx_std_*`, with `!` and `BREAKING CHANGE:`.

## Limitations

This skill covers plugin libraries, not the applications that consume them. Its drogon behavior was verified against drogon 1.9.13 and trantor 1.5.28; recheck [reference.md](reference.md) after a drogon upgrade. In an existing plugin project, follow its established conventions unless the user asks for changes.
