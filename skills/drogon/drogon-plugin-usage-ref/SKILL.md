---
name: drogon-plugin-usage-ref
description: "Drogon plugin usage reference for drogon applications built with CMake + vcpkg. Covers the three ways an app gets plugins: drogon's built-in plugins (AccessLogger, SecureSSLRedirector, SlashRemover, Redirector, RealIpResolver, Hodor, GlobalFilters, PromExporter), third-party plugin libraries, and plugins implemented inside the project. Includes config.json registration, dependencies and initialization order, getPlugin timing, static-link registration, thread safety, startup checks, and tests, as implemented in drogon 1.9.13. Use when adding, configuring, calling, or reviewing plugins in a drogon app; use drogon-plugin-lib-ref to write a plugin library for other apps. Preserve existing project conventions unless changes are requested."
---

# drogon-plugin-usage-ref

Use plugins in a drogon application. There are three kinds: drogon's built-in plugins (`drogon::plugin::*`), third-party plugin libraries linked into the app, and in-project plugins defined in the app's own sources. All three are registered the same way and fetched with `app().getPlugin<T>()`. They differ in how they are obtained, linked, and configured. This skill records that behavior as implemented in drogon 1.9.13.

- Read [reference.md](reference.md) for drogon behavior with source locations, every built-in plugin's config keys, and verification results.
- Start from [references-cmake-vcpkg/](references-cmake-vcpkg/), a buildable app that uses all three kinds. Its test passes offline.
- Write C++ with the `cpp-style-ref` skill (CMake + vcpkg variant) and commit messages with the `git-commit-style-ref` skill. To write a plugin library for other apps, use `drogon-plugin-lib-ref`.

## Choosing the Kind

| Need | Kind |
| --- | --- |
| Access log, HTTPS redirect, slash cleanup, client IP behind a proxy, rate limit, filters on every route, Prometheus metrics | Built-in |
| A capability published as a drogon plugin library, such as `drogon-llm` | Third-party |
| An app-specific service that needs settings from config.json, drogon's init and shutdown, ordering against other plugins, or lookup from any handler | In-project |

A service that needs none of the in-project reasons is plain code, not a plugin. Move an in-project plugin into a library with `drogon-plugin-lib-ref` once a second app needs it.

## Registration

- List each plugin under `plugins` in config.json. Each entry has three keys:
  - `name`: the namespace-qualified class name, such as `drogon::plugin::AccessLogger`.
  - `config` (optional): the object passed to `initAndStart`. Omitted, the plugin gets a null value, and `get()` returns the defaults.
  - `dependencies` (optional): names of plugins to initialize first.
- config.yaml uses the same structure when drogon is built with yaml support. `app().addPlugin(name, dependencies, config)` and `addPlugins(array)` register plugins in code before `run()`. Drogon appends them after the config-file entries.
- Drogon creates every listed plugin, then initializes them in list order, each one's dependencies first. It shuts them down in reverse order inside `app().quit()`. List a plugin after its dependencies anyway, so the file reads in initialization order.
- Declare a dependency when a plugin uses another one in `initAndStart`, or when its documentation requires it: SlashRemover and SecureSSLRedirector need Redirector, and Hodor with `use_real_ip_resolver` needs RealIpResolver. A dependency missing from the list, or a dependency cycle, makes drogon log FATAL and `abort()`.
- An unknown or unlinked name only logs `Plugin <name> undefined!` at ERROR level. The app keeps running without the plugin.

## Linking

With static libraries, which the vcpkg triplet `x64-linux` uses, a plugin registers itself through a static object in `DrObject<T>`. That object is linked only if something in the executable references the class.

- **Built-in**: no action needed. `<drogon/drogon.h>` includes seven built-in plugin headers, whose inline constructors pull the plugins into any TU that includes it. Redirector comes in through SecureSSLRedirector's object file.
- **Third-party**: reference the class in the app; the startup check below does. Including the header is enough only if the class declares an inline constructor such as `Greeter() {}`. Without one, the skeleton's stand-in library logged `undefined!`.
- **In-project**: compile plugins and controllers into the executable, or into an OBJECT library shared by the executable and the tests. A STATIC library drops every class that nothing references. The skeleton's controller then answered 404.

## Accessing Plugins

- Use `app().getPlugin<T>()` for a `T*`, `getSharedPlugin<T>()` for a `std::shared_ptr<T>`, or `getPlugin(name)` for a `PluginBase*`. Include the plugin's header.
- Call these only after `run()` has started. Valid places are another plugin's `initAndStart`, the constructor of a controller, filter, or middleware, a beginning advice, or a handler. `getPlugin<T>()` asserts `isRunning()` in debug builds and caches its first result in a function-local static. In a release build, a call before `run()` therefore caches `nullptr` for the rest of the process.
- Inside `initAndStart`, another plugin is already created but initialized only if it is a declared dependency or listed earlier.
- Handlers run concurrently on `number_of_threads` IO threads, so every plugin method a handler calls must be thread-safe. Make state immutable after `initAndStart`, or guard it with atomics or a mutex.
- Setters that built-in plugins expose after start are not synchronized. These include `Hodor::setRejectResponseFactory`, `Hodor::setUserIdGetter`, and the `Redirector::register*Handler` methods. Call them in a beginning advice, which runs before any listener accepts connections.
- Plugins are shut down and released inside `app().quit()`. Work that can outlive `quit()` must not use them.

## Startup Check

Register one beginning advice, as `RegisterPluginSetup()` does in the skeleton. It fetches every plugin that code calls and logs FATAL and aborts if one is missing, matching drogon's reaction to a missing dependency. The same advice applies code-side configuration, such as Hodor's rejection response. Plugins used only through config.json, such as AccessLogger and SlashRemover, fail silently when missing. A test that looks them up by name covers them.

## Built-in Plugins

| Name (`drogon::plugin::`) | Use | Dependencies and notes |
| --- | --- | --- |
| `AccessLogger` | Log every response | `log_path` empty logs to the main log. `use_real_ip` takes effect only while RealIpResolver is loaded. |
| `SecureSSLRedirector` | Redirect HTTP requests to HTTPS | Redirector |
| `SlashRemover` | Redirect or forward paths with duplicate or trailing slashes | Redirector |
| `Redirector` | Run redirect, path-rewrite, and forward handlers that other code registers | Register handlers in a beginning advice. Redirects use 302. |
| `RealIpResolver` | Take the client IP from `x-forwarded-for` when the peer is in `trust_ips` | IPv4 only. Read it with `RealIpResolver::GetRealAddr(req)`, which falls back to the peer address. |
| `Hodor` | Rate-limit by URL regex, globally, per IP, or per user | RealIpResolver when `use_real_ip_resolver` is true. The default rejection is 429 with `rejection_message`. |
| `GlobalFilters` | Apply filters by class name to every request except `exempt` paths | The filters must be linked like any controller. |
| `PromExporter` | Serve metrics at `path` (default `/metrics`) | App code updates them via `getCollector<T>(name)->metric({labels})`. |

Copy each plugin's config block from its header in `drogon/plugins/`. [reference.md](reference.md) lists the keys and defaults that drogon 1.9.13 reads.

## Third-Party Plugins

- Obtain the library like any CMake package: a vcpkg port or overlay, CPM, `add_subdirectory`, or `find_package(<pkg> CONFIG REQUIRED)` from an install prefix. Then link its target, such as `drogon-llm::drogon-llm`.
- Build it with the same drogon installation as the app. Its target links `Drogon::Drogon`, and the app and plugin must share one drogon.
- Take these details from the library's documentation: the registered class name, the config keys, the environment variables that hold secrets (config.json holds only the variable name, such as `api_key_env`), the required dependencies, and the threading or coroutine rules of its API.
- An invalid config makes the plugin's `initAndStart` throw. The exception leaves `app().run()` and terminates the process with the plugin's message.

## In-Project Plugins

- Create the class with `drogon_ctl create plugin <ns::Name>`, which writes `ns_Name.h` and `ns_Name.cc` in drogon's style, or write it by hand following `cpp-style-ref`. Keep it in a named namespace; its registered name is the namespace-qualified class name.
- Derive from `drogon::Plugin<T>` and keep `T` default-constructible. Otherwise drogon silently leaves it unregistered.
- In `initAndStart`:
  - Read `config` with defaults.
  - Throw `std::invalid_argument` with a `<ns::Name>: ...` message for an invalid value.
  - Fetch dependencies and keep their pointers.
  - Optionally register handlers or advices. Routes are frozen after plugins initialize, so a beginning advice cannot add routes.
  - Do not block or await network I/O, because no event loop is running yet.
- Release resources in `shutdown()`. Destructors run after every plugin's `shutdown()`, in an order unrelated to dependencies, so a destructor must not use another plugin.
- Read secrets from an environment variable named in `config`, never from config.json.

## Tests

- Each `tests/test_*.cpp` builds into its own executable that links the OBJECT library, run as `just test test_<name>`. Tests are plain programs that report through their exit code, with no framework or CTest.
- A test loads config.json, calls the same startup registration as `main`, and waits briefly in a beginning advice for the listeners. It then sends requests through a per-request `HttpClient` and calls `app().quit()`.
- Check that every config entry loaded, by name. Check each plugin's observable effect, such as a header, redirect, or status. Exercise failure paths by editing a copy of config.json: a missing entry, a missing dependency, an invalid value, and a misspelled name.

## Starting From the Skeleton

```text
plugin-demo/
├── CMakeLists.txt, CMakePresets.json, justfile, .clang-format, .gitignore
├── config.json                       # all three kinds; no secrets
├── src/
│   ├── main.cpp                      # loadConfigFile, RegisterPluginSetup, run
│   ├── plugin_setup.hpp/.cpp         # startup check and code-side plugin settings
│   ├── plugins/guest_book.hpp/.cpp   # in-project plugin, depends on Greeter
│   └── controllers/guests.hpp/.cpp   # handler calling all three kinds
├── third_party/drogon-greeter/       # stand-in third-party plugin library
└── tests/
    ├── CMakeLists.txt                # one executable per test_*.cpp
    └── test_plugins.cpp
```

Copy the directory and rename `plugin-demo`, `plugin_demo`, and `PLUGIN_DEMO_`. Replace `third_party/drogon-greeter` with the real package, and `GuestBook` and `Guests` with the app's own code. Copy `CMakePresets.json`, `.clang-format`, and `tests/CMakeLists.txt` unchanged; the `.clang-format` is the user's shared config and is not edited. Then run `just build`, `clang-format --dry-run -Werror` on every C++ file, and `just test test_plugins`.

## Limitations

This skill covers using plugins in drogon applications. For writing a plugin library, use `drogon-plugin-lib-ref`. The drogon behavior was verified against drogon 1.9.13 and trantor 1.5.28, and the linking behavior with Clang 21 and GCC 15 on Linux; MSVC and clang-cl were not tested. Recheck [reference.md](reference.md) after a drogon upgrade. In an existing project, follow its established conventions unless the user asks for changes.
