# drogon-plugins Reference

The drogon behavior below was read from drogon 1.9.13 as installed by vcpkg (source under `$VCPKG_ROOT/buildtrees/drogon/src/v1.9.13-*/`), with trantor 1.5.28. Paths are relative to that source tree. Changes in drogon 1.9.14 are noted where they affect plugins. The skeleton in `references-cmake-vcpkg/` was built and run against these versions with Clang 21.1.8 and CMake 4.2.

The preferences come from the user's `drogon-llm` repository (commits and sessions of 2026-10-24 and 2026-10-25). Section 7 records where each one came from.

## 1. Registration

- `Plugin<T>` derives from `PluginBase` and `DrObject<T>` (`lib/inc/drogon/plugins/Plugin.h:130`). `PluginBase` is `trantor::NonCopyable` and declares the pure virtual functions `initAndStart(const Json::Value &)` and `shutdown()` (`Plugin.h:67`, `:71`).
- `DrObject<T>` has a static `DrAllocator alloc_` (`lib/inc/drogon/DrObject.h:145`, defined at `:149`). Its constructor registers `T` in `DrClassMap` under `DrClassMap::demangle(typeid(T).name())`, but only if `T` is default-constructible (`DrObject.h:109-142`). A non-default-constructible plugin is silently left unregistered. The registered name is the namespace-qualified class name, such as `drogon_llm::LlmClient`.
- `PluginsManager::createPlugin` builds each listed plugin through `DrClassMap::newSharedObject`. An unknown name only logs `Plugin <name> undefined!` at ERROR level, and the app keeps running (`lib/src/PluginsManager.cc:90-100`).
- With a static library, the object file that holds the plugin's vtable, and therefore `alloc_`, is linked only when the app references the class. An app that lists the plugin in config.json but never names the class gets the `undefined!` message. The skeleton's consumer check reproduced this for both `find_package` and `add_subdirectory`.
- Each `plugins` entry has `name`, `config` (the object passed to `initAndStart`), and an optional `dependencies` array of plugin names (`PluginsManager.cc:31-88`). An unloaded dependency is `LOG_FATAL` plus `abort()` (`:70-72`); a dependency cycle is `LOG_FATAL` plus `abort()` (`Plugin.h:58-61`).
- `app().addPlugin(name, dependencies, config)` and `app().addPlugins(array)` register plugins in code. Both assert `!isRunning()` (`lib/src/HttpAppFrameworkImpl.cc:370-397`), and `run()` appends them after the config-file entries (`:645-660`).

## 2. Lifecycle and Threads

`HttpAppFrameworkImpl::run()` (`lib/src/HttpAppFrameworkImpl.cc:520-689`) does, in order:

1. Moves the main loop to the calling thread, creates the IO loop pool (`number_of_threads` loops, not yet looping), creates the listeners without listening (`:623`), and creates DB and Redis clients.
2. Sets `running_ = true` (`:643`).
3. Calls `initializeAllPlugins` (`:663`). It creates every listed plugin first, then initializes them in list order. `PluginBase::initialize()` initializes declared dependencies depth-first before calling `initAndStart` (`Plugin.h:40-63`).
4. Initializes the routers, then queues one task on the main loop that runs every beginning advice and afterwards calls `listenerManagerPtr_->startListening()` (`:674-682`).
5. Starts the IO loops (`:688`) and runs the main loop (`:689`).

Consequences:

- `initAndStart` runs synchronously on the thread that called `run()`, before any loop is looping. Nothing it awaits or `sync_wait`s on a drogon loop can complete there.
- An exception from `initAndStart` propagates out of `run()`; nothing catches it.
- `getPlugin<T>()` asserts `isRunning()` in debug builds (`lib/inc/drogon/HttpAppFramework.h:224`) and caches its result in a function-local `static` (`:225-226`). A call before plugins exist caches `nullptr` permanently in a release build, so call it only from `initAndStart`, a beginning advice, or a handler.
- Inside `initAndStart`, `getPlugin<Other>()` returns an object, because every plugin is created before any is initialized. That object's `initAndStart` has run only if `Other` is a declared dependency or appears earlier in the list.
- Beginning advices run on the main loop before `startListening()`. On Linux, `ListenerManager` creates one `HttpServer` per IO loop (`lib/src/ListenerManager.cc:111`), and trantor's `TcpServer::start()` runs `listen()` through `runInLoop` on that IO loop. A request that a beginning advice sends to the app's own listener is refused (`Bad server address`) unless it waits first. Without `LISTEN_DELAY`, the skeleton's `test_messages` failed this way in three of three runs.
- HTTP handlers run on the IO loop that owns the connection, not on the main loop.
- `app().quit()` queues a task on the main loop that stops listening, resets the routers, and then resets the plugins manager (`HttpAppFrameworkImpl.cc:1034-1053`). Its destructor calls `shutdown()` in reverse initialization order (`PluginsManager.cc:20-29`), before the Redis and DB clients are destroyed and the loops quit. Only plugins whose `initAndStart` returned are in that list. If `quit()` never runs, the manager is destroyed with the static app instance at exit.

## 3. HttpClient

- `HttpClient::newHttpClient(hostString, loop = nullptr, ...)` runs the client on the app's main loop when no loop is passed (`lib/inc/drogon/HttpClient.h:61`, `:344`). The coroutine that awaits `sendRequestCoro` resumes on the client's loop.
- `pipeliningDepth_` defaults to 0 (`lib/src/HttpClientImpl.h:160`), and a buffered request is sent only while `pipeliningCallbacks_.size() <= pipeliningDepth_` (`lib/src/HttpClientImpl.cc:509`). A client therefore has one request in flight and queues the rest. A shared client serializes concurrent calls; a client per request on the caller's loop keeps them concurrent and resumes each coroutine on its caller's thread. In `drogon-llm`, two 2-second requests started together on one IO thread both finished in about 2.0 seconds.
- `sendRequestCoro` throws `drogon::HttpException`, whose `code()` is the `ReqResult`, such as `ReqResult::Timeout` or `BadServerAddress` (`HttpClient.h:359-382`).
- In 1.9.13, only a request with `timeout > 0` keeps a strong reference to its client until the callback (`RequestCallbackParams`, `HttpClientImpl.cc:290-360`). With `timeout <= 0`, nothing else owns the client, so the transport keeps the `HttpClientPtr` in the coroutine frame. drogon 1.9.14 keeps the client alive for every request.
- The 1.9.13 client buffers the complete response before calling back, so it cannot stream server-sent events; streaming needs another transport.

## 4. Coroutines

- drogon's [coroutine guide](https://github.com/drogonframework/drogon-docs/blob/master/ENG/ENG-17-Coroutines.md) passes coroutine parameters by value, because a referenced object can be destroyed while the coroutine is suspended. Handlers take `drogon::HttpRequestPtr` by value.
- drogon's [FAQ](https://github.com/drogonframework/drogon-docs/blob/master/ENG/ENG-FAQ.md) recommends asynchronous APIs or coroutines, and its [threading model](https://github.com/drogonframework/drogon-docs/blob/master/ENG/ENG-FAQ-1-Understanding-drogon-threading-model.md) calls synchronous APIs "evil for both performance and safety".
- `async_run` moves the coroutine callable into an `AsyncTask` frame before invoking it (`lib/inc/drogon/utils/coroutine.h:749-762`), so a capturing lambda stays alive; anything it captures by reference must outlive the task.
- `sync_wait` runs the awaitable on a new `std::thread` and blocks the caller on a condition variable (`coroutine.h:500`). That thread runs no loop, so a per-request transport falls back to the main loop: calling `sync_wait` on the main loop thread deadlocks, and calling it on an IO thread blocks every connection on that thread.
- With drogon 1.9.13 and Clang 21 or newer, `Task<T>::promise_type` is an aggregate (`coroutine.h:250-251`), and Clang aggregate-initializes it from the coroutine's arguments. A single-parameter `Task<T>` coroutine whose argument converts implicitly to `T` runs that conversion before the body, and a throwing conversion crashes the process. drogon 1.9.14 fixes this (commit `bfb3ae2e`, issue #2579). Member coroutines with one parameter receive two arguments (object and parameter) and are not affected.

## 5. CMake

- `cmake_modules/FindFilesystem.cmake` saves `CMAKE_CXX_STANDARD`, sets it to 17 (`:135-136`), and restores it with `set(CMAKE_CXX_STANDARD "${BACKUP_CXX_STANDARD}")` (`:261`). The installed `DrogonConfig.cmake` runs it on every `find_package(Drogon)` (`share/drogon/DrogonConfig.cmake:71`). An unset standard comes back as `""`, and `target_compile_features` then fails with `CXX_STANDARD property ... contained an invalid value: ""`. The workaround unsets the empty value after `find_package(Drogon)` and after `find_dependency(Drogon)`.
- `target_compile_features(PUBLIC cxx_std_23)` is exported as `INTERFACE_COMPILE_FEATURES`, so `find_package` consumers inherit the minimum. A parent's higher `CMAKE_CXX_STANDARD` still applies in `add_subdirectory` mode; in `drogon-llm`, a C++26 parent built the library with `-std=gnu++26`.
- With `PROJECT_IS_TOP_LEVEL` false, the tests and install rules default to OFF, so `add_subdirectory` and CPM consumers build only the library.

## 6. Skeleton Verification

On 2026-10-25, a copy of `references-cmake-vcpkg/` gave these results:

| Check | Result |
| --- | --- |
| `just build` | Library and both tests built with no warnings |
| `just run test_config` | Printed `base_url` and `api_key: (empty)` |
| `just run test_messages` | Echoed `Hello!`; the empty message failed with `HTTP 400: text must be a non-empty string`; exit 0 |
| `api_key_env` with the variable unset or set | Threw `environment variable ... is unset or empty`, or printed `api_key: (set)` |
| `base_url` removed | Threw `config needs a non-empty base_url` |
| `just publish` + `find_package`, and `add_subdirectory` | Consumer built and fetched the plugin; tests were not built in subdirectory mode |
| Consumer that never references the class | `Plugin drogon_echo::EchoClient undefined!` |
| `clang-format --dry-run -Werror` | Clean |

## 7. Origin of the Preferences

| Preference | Origin in `drogon-llm` |
| --- | --- |
| Library with two consumption modes, tests and install gated on `PROJECT_IS_TOP_LEVEL` | The goal was a plugin "可以给其他项目使用"; the user chose to support both source and installed consumption. |
| Secrets only from the environment, through `api_key_env` | The user asked for "只从系统环境中获取"; config.json then held no secrets and became tracked. |
| No `CMAKE_CXX_STANDARD` in the library | The user asked "库需要设置这个吗"; compile features replaced it, which exposed the `FindFilesystem.cmake` problem. |
| Throw in `initAndStart`, `std::expected` for requests | The user asked whether `std::expected` could replace the throw. The override returns `void` and drogon ignores the outcome, so startup kept the exception; the user then chose `std::expected` and parameter structs for requests. |
| Interface shaped like the reference SDK | The user asked for the OpenAI Python SDK's interface. |
| Coroutine-only API | The first `Create()` was synchronous and needed a worker thread. The user asked why and recalled that drogon recommends coroutines, so commit `1905601` removed the synchronous API. |
| One `HttpClient` per request on the caller's loop | A per-thread cached client was proposed, then rejected after reading drogon's pipelining code (`1905601`). |
| Value handle with plain API group members | The user asked for `llm.chat.completions.Create` instead of `->` and accepted the value-handle recommendation (`1c846f1`). |
| `tests/test_*.cpp` executables and `just run test_<name>` | User request. |
| Skeletons with `TODO` bodies | User request: "不需要具体的实现，使用 TODO 来标记就行". |
| Fine-grained commits; code and `AGENTS.md` separate; no attribution trailers | The user asked to split commits "细一点" and forbade `Co-Authored-By`; the history keeps `docs(AGENTS.md)` commits separate. |
| `docs/drogon-plugin.md` | User request: "在 docs 整理一下当前项目关于 drogon 插件相关的". |
| Recommendation before a design change | The user repeatedly asked "推荐使用什么格式" or replied "按建议" and "开始" after a recommendation. |

## 8. Differences from `drogon-llm`

- `drogon-llm` declares private data after the public interface. The skeleton follows `cpp-style-ref` and puts private data first.
- `drogon-llm` names the plugin `LlmClient`, the handle `Llm`, and the factory `NewLlmClient()`. The skeleton keeps that pattern as `EchoClient`, `Echo`, and `NewEchoClient()`.
- `drogon-llm` requires a `model` key and uses DeepSeek in its tests. The skeleton requires only `base_url` and tests against a handler served by the same app, so it runs offline.
