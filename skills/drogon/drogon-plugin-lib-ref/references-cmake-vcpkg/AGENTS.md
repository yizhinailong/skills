# AGENTS.md

## Project state

Drogon plugin library, consumed by other Drogon apps.

- The library target is `drogon-echo` (alias `drogon-echo::drogon-echo`). It is static by default; pass `-DBUILD_SHARED_LIBS=ON` for a shared build.
- Public headers live in `include/drogon_echo/` and are installed through the `HEADERS` file set. Consumers include `<drogon_echo/...>`. Private headers go in `src/`, which is only on the library's own include path.
- The plugin class is `drogon_echo::EchoClient` (`drogon::Plugin<EchoClient>`). Apps register it by class name under `plugins` in their config.json. Drogon calls `initAndStart(config)` inside `app().run()`, before beginning advices, so `app().getPlugin<drogon_echo::EchoClient>()` only works after `run()` starts.
- Registration relies on drogon's static `DrObject<T>::alloc_` initializer. With the static library, the linker only pulls in `echo_client.o` when the app references the class. If the app lists the plugin in config.json but never calls `getPlugin<EchoClient>()` (or otherwise uses the class), drogon logs `Plugin drogon_echo::EchoClient undefined!` and keeps running without it.
- Plugin config keys: `base_url` is required, and `initAndStart` throws `std::invalid_argument` if it is empty. The API key is never read from config.json. The optional `api_key_env` names the environment variable that holds it, and `initAndStart` throws if that variable is unset or empty. Without `api_key_env`, `api_key` stays empty (endpoints without auth). Add new keys to `EchoConfig`, `initAndStart`, the class doc comment, `config.json`, and the table in `docs/drogon-plugin.md`.
- `EchoClient::NewEchoClient()` returns an `Echo` by value, and `co_await echo.messages.Create(MessageCreateParams)` yields `Result<Message>` (`std::expected<T, Error>`, in `error.hpp`). `Create` returns `drogon::Task<...>`; there is no sync API. `Echo` is a value handle: `messages` is a plain member, and the only shared state is one `std::shared_ptr<details::Transport>`, so copies are cheap and go into coroutines by value. Request and response structs live in `types/`. The private `details::Transport` (`src/details/`) adds the auth header and maps failures, including the `drogon::HttpException` that `sendRequestCoro` throws, to `ErrorKind`.
- Coroutines take their parameters by value, and `Messages::Create` copies `m_transport` into its frame before the first `co_await`. Keep both rules in new API methods.
- `Transport::Post` creates a new `HttpClient` for each request on the calling thread's loop, or on the main loop when the thread runs no loop, and keeps it in the coroutine frame until the response arrives. Don't cache one client for reuse: drogon's `HttpClient` defaults to pipelining depth 0, so a shared client would serialize concurrent requests.

## Consuming the library

Keep both ways working when you change `CMakeLists.txt`.

- From source: `add_subdirectory(<path>)`, or CPM with `SOURCE_DIR`/`GIT_REPOSITORY`. `PROJECT_IS_TOP_LEVEL` is false then, so `DROGON_ECHO_BUILD_TESTS` and `DROGON_ECHO_INSTALL` default to OFF.
- Installed: `just publish` installs into `publish/`. Consumers set `CMAKE_PREFIX_PATH` to that directory and call `find_package(drogon-echo CONFIG REQUIRED)`. The package config calls `find_dependency(Drogon CONFIG)`, so consumers don't need their own `find_package(Drogon)`. Version compatibility is `SameMinorVersion`.
- Either way, link `drogon-echo::drogon-echo`. It carries `Drogon::Drogon` publicly.

## Tests

`tests/CMakeLists.txt` globs `tests/test_*.cpp` (non-recursive, `CONFIGURE_DEPENDS`) and builds each file into its own executable named after the file stem (`tests/test_config.cpp` → `build/<preset>/tests/test_config`). Adding a `tests/test_*.cpp` file is enough. They are plain programs that report through their exit code: there is no test framework and no CTest registration.

- `tests/test_config.cpp` loads `config.json` relative to the working directory (`just run` uses the repo root), prints the plugin's settings from a beginning advice, then calls `app().quit()`.
- `tests/test_messages.cpp` serves `/messages` from the same app (the listener in `config.json`) and sends a valid and an empty message through `co_await NewEchoClient().messages.Create(...)` inside `drogon::async_run`. It waits `LISTEN_DELAY` first: beginning advices run before `startListening()`, so an immediate request fails with `Bad server address`.

## Commands

`just` drives the build. Recipes map onto CMake presets named `<os>-<profile>` (`linux-debug`, `linux-release`, `windows-debug`, `windows-release`), with build trees in `build/<preset>/`.

- `just init`: `vcpkg install drogon` (classic mode, no `vcpkg.json`; `VCPKG_ROOT` must be set).
- `just build [debug|release]`: configure only if `build/<preset>/CMakeCache.txt` is missing, then build, then copy `compile_commands.json` to `build/`. After a failed configure, run `just configure debug --fresh`.
- `just run <target> [profile]`: build everything, then run `build/<os>-<profile>/tests/<target>`, e.g. `just run test_messages`.
- `just publish [profile]`: build (default `release`) and `cmake --install` into `publish/`.
- `just clean [profile]`: delete `build/<os>-<profile>/`, or `build/` and `publish/` when no profile is given.

## Build notes

- Requires CMake >= 4.0, Ninja, and ccache (hard-wired as `CMAKE_CXX_COMPILER_LAUNCHER`). Compiler is `clang++` on Linux, `clang-cl` on Windows.
- Don't set `CMAKE_CXX_STANDARD` here. The library declares `target_compile_features(PUBLIC cxx_std_23)` (the public headers use `std::expected`), which is exported to `find_package` consumers.
- Drogon 1.9.13's `FindFilesystem.cmake` leaves an unset `CMAKE_CXX_STANDARD` defined as `""`, which `target_compile_features` rejects. Both `CMakeLists.txt` and `cmake/drogon-echo-config.cmake.in` unset the empty value right after finding Drogon. Keep those blocks.
- Sources and public headers are collected with `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`, so adding a file triggers a re-configure on the next build.

## Code style

New C++ code follows the `cpp-style-ref` skill (CMake + vcpkg variant). Drogon's virtual overrides (`initAndStart`, `shutdown`) keep their upstream names.

Format with `clang-format -i <files>` (root `.clang-format`). Check with `clang-format --dry-run -Werror <files>`.
