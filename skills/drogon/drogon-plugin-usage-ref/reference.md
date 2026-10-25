# drogon-plugin-usage-ref Reference

The drogon behavior below was read from drogon 1.9.13 as installed by vcpkg with the features `core;ctl;mysql;orm;postgres;redis;sqlite3;yaml`, and with trantor 1.5.28. The source is under `$VCPKG_ROOT/buildtrees/drogon/src/v1.9.13-*/`, and paths are relative to that tree. The skeleton in `references-cmake-vcpkg/` was built and run against these versions with Clang 21 and CMake 4.2. The linking observations in section 3 were also checked with GCC 15.2.

## 1. Registration

- `PluginsManager::initializeAllPlugins` (`lib/src/PluginsManager.cc:31-88`) first creates every entry that has a `name` (`:37-43`). It then gives each one its `config` and `dependencies`, and initializes them in list order (`:83-87`). `PluginBase::initialize()` initializes declared dependencies depth-first before `initAndStart` (`lib/inc/drogon/plugins/Plugin.h:40-63`).
- `configs` is a `const Json::Value&` (`PluginsManager.cc:32`), so a missing `config` key yields a null value, not an empty object (`:54`). `get()` and `isMember()` work on a null value. drogon's [plugin guide](https://github.com/drogonframework/drogon-docs/blob/master/ENG/ENG-10-Plugins.md) says "an empty object", which matters only to code that checks `isObject()`.
- A dependency that is not loaded is `LOG_FATAL` plus `abort()` (`PluginsManager.cc:63-73`). A cycle is `LOG_FATAL` plus `abort()` (`Plugin.h:58-62`).
- An unknown name, or a class that is not registered, only logs `Plugin <name> undefined!` at ERROR level and is skipped (`PluginsManager.cc:90-100`).
- `addPlugin` and `addPlugins` assert `!isRunning()` (`lib/src/HttpAppFrameworkImpl.cc:375`, `:391`). `run()` appends their entries after the config-file entries (`:644-660`).
- config.yaml has the same `plugins` list with `name`, `dependencies`, and `config` keys (`drogon_ctl/templates/config_yaml.csp:290-302`). The skeleton does not exercise it.

## 2. Lifecycle and Access Timing

`HttpAppFrameworkImpl::run()` sets `running_ = true` (`HttpAppFrameworkImpl.cc:643`) and initializes the plugins (`:661-669`). It then sets `routersInit_` (`:671`), initializes the routers (`:672-673`), and queues the beginning advices followed by `startListening()` (`:674-682`).

- `HttpControllersRouter::init` creates filters and middlewares (`lib/src/HttpControllersRouter.cc:36`), so their constructors run after every plugin is initialized.
- Controller objects are created by tasks queued on the main loop when their routes are registered. `registerHandler` queues `createHandlerInstance` (`lib/inc/drogon/HttpAppFramework.h:545`), and simple and WebSocket controllers queue the lookup themselves (`HttpControllersRouter.cc:243-256`, `:275`, `:303`). These tasks run once the main loop starts, after the plugins are initialized and before the beginning advices. The skeleton confirmed this order (section 6).
- Route registration asserts `!routersInit_` (`HttpAppFrameworkImpl.cc:290-343`). A plugin may register routes in `initAndStart`, as PromExporter does (`lib/src/PromExporter.cc:18-35`). A beginning advice may not.
- `getPlugin<T>()` and `getSharedPlugin<T>()` assert `isRunning()` and keep their first result in a function-local `static` (`HttpAppFramework.h:218-247`). Before `run()`, the plugins manager is empty, so the cached value is `nullptr`. drogon's plugin guide says an early call returns an uninitialized instance, which does not match 1.9.13.
- `quit()` queues a task that stops listening, resets the routers, and then resets the plugins manager (`HttpAppFrameworkImpl.cc:1034-1053`). The manager's destructor calls `shutdown()` in reverse initialization order (`PluginsManager.cc:20-29`). The plugin objects are released afterwards from a `std::map` keyed by name (`lib/src/PluginsManager.h:38`), whose destruction order is unrelated to dependencies.

## 3. Static Linking and Registration

- `DrObject<T>` holds a static `DrAllocator alloc_` whose constructor registers `T` in `DrClassMap`, but only if `T` is default-constructible (`lib/inc/drogon/DrObject.h:109-149`). The plugin's own object file instantiates it. With a static library, the linker pulls that object file in only if something references a symbol it defines.
- A TU that includes a plugin header instantiates `DrObject<T>::alloc_` if the class declares an inline constructor such as `Greeter() {}`. That constructor references the class's VTT, which lives in the plugin's object file. With an implicit default constructor, the header instantiates nothing. `nm` on a TU that includes only the header gave the same result with Clang 21 and GCC 15. drogon's built-in plugins and the `drogon_ctl` plugin template (`drogon_ctl/templates/plugin_h.csp`) declare such a constructor.
- `<drogon/drogon.h>` includes the headers of every built-in plugin except Redirector (`lib/inc/drogon/drogon.h:29-36`). drogon's ORM sources include `drogon.h` as well (`orm_lib/src/DbClientImpl.cc:30`, `orm_lib/src/DbClientLockFree.cc:20`). A link map of an app whose source included only `<drogon/HttpAppFramework.h>` showed `DbClientImpl.cc.o` pulling in all seven, and `SecureSSLRedirector.cc.o` pulling in `Redirector.cc.o`. A drogon built without ORM relies on the app including `drogon.h`.
- An HttpController registers its routes from a static object in its own object file, the same way plugins register.

## 4. Built-in Plugins

Each header in `lib/inc/drogon/plugins/` documents its config block. The keys and defaults below are the ones the 1.9.13 sources read.

| Plugin | Keys (default) | Hook and notes |
| --- | --- | --- |
| `AccessLogger` | `log_format` (`""`: `$request_date $method $url [$body_bytes_received] ($remote_addr - $local_addr) $status $body_bytes_sent $processing_time`), `log_path` (`""`: main log), `log_file` (`access.log`), `log_size_limit` (0), `max_files` (0), `log_index` (0), `use_local_time` (true), `show_microseconds` (true), `custom_time_format` (`""`), `use_real_ip` (false), `path_exempt` (string or array), `use_spdlog` (false) | Pre-sending advice (`AccessLogger.cc:265`). Placeholders are listed in `AccessLogger.h:45-72`. `use_real_ip` calls `RealIpResolver::GetRealAddr` (`AccessLogger.cc:525-527`). |
| `GlobalFilters` | `filters` (array of filter class names), `exempt` (string or array of regexes) | Pre-routing advice (`GlobalFilters.cc:69`). Filters are created through `DrClassMap::getSingleInstance` (`:21`). |
| `Hodor` | `algorithm` (`token_bucket`, `fixed_window`, `sliding_window`), `time_unit` (60 s), `urls` (regex array; empty limits all), `capacity`, `ip_capacity`, `user_capacity` (0 = unlimited), `multi_threads` (true), `use_real_ip_resolver` (false), `rejection_message` (`Too many requests`), `limiter_expire_time` (600 s), `sub_limits` (array of `urls` plus capacities), `trust_ips` (array; these addresses are never limited) | Pre-handling advice (`Hodor.cc:119`). The default rejection is 429 and closes the connection (`:76-80`). `user_capacity` needs `setUserIdGetter`. The client address comes from `GetRealAddr` only with `use_real_ip_resolver` (`:225-227`). |
| `PromExporter` | `path` (`/metrics`), `collectors` (array of `name`, `help`, `type` = `counter`, `gauge`, or `histogram`, `labels`) | Registers a GET handler at `path` (`PromExporter.cc:18-36`). Update a metric with `getCollector<drogon::monitoring::Counter>(name)->metric({label values})->increment()` (`lib/inc/drogon/utils/monitoring/Collector.h:63-66`). |
| `RealIpResolver` | `trust_ips` (array of IPv4 addresses or CIDRs), `from_header` (`x-forwarded-for`), `attribute_key` (`real-ip`) | Pre-routing advice that stores the address in the request attributes (`RealIpResolver.cc:108-150`). An IPv6 entry throws (`:219`). `GetRealAddr` returns the peer address when the plugin is not loaded (`:157-166`). |
| `Redirector` | none | Sync advice (`Redirector.cc:24`). It runs redirect, path-rewrite, and forward handlers (`Redirector.h:31-52`). Redirects use `newRedirectionResponse`, whose default status is 302 (`lib/inc/drogon/HttpResponse.h:441-443`). |
| `SecureSSLRedirector` | `ssl_redirect_exempt` (string or array of regexes), `secure_ssl_host` (`""`) | Registers a handler with Redirector. Without Redirector it logs `Redirector plugin is not found!` and does nothing (`SecureSSLRedirector.cc:58-61`). |
| `SlashRemover` | `remove_trailing_slashes` (true), `remove_duplicate_slashes` (true), `redirect` (true; false forwards internally) | Registers handlers with Redirector, with the same missing-Redirector behavior (`SlashRemover.cc:198-208`). |

`Hodor::setUserIdGetter` and `setRejectResponseFactory` assign a `std::function` member without locking (`Hodor.h:99-114`). Their documentation says to call them after `run()`, for example from a beginning advice. Redirector's `register*Handler` methods append to a vector without locking (`Redirector.h:82-110`).

## 5. drogon_ctl

- `drogon_ctl create plugin demo::Counter` writes `demo_Counter.h` and `demo_Counter.cc` into the current directory (`drogon_ctl/create_plugin.cc`). The class is in namespace `demo`, has an inline `Counter() {}`, and has empty `initAndStart` and `shutdown` bodies.
- A `drogon_ctl create project` tree has a `plugins/` directory (`drogon_ctl/create_project.cc:126`). Its CMake template compiles that directory straight into the executable (`drogon_ctl/templates/cmake.csp:46`, `:67`), so those plugins are never dropped. Its config template lists PromExporter and AccessLogger (`drogon_ctl/templates/config_json.csp:319-344`).
- vcpkg installs the tool as `$VCPKG_ROOT/installed/<triplet>/tools/drogon/drogon_ctl` when the `ctl` feature is enabled.

## 6. Skeleton Verification

On 2026-10-25, a copy of `references-cmake-vcpkg/` gave these results:

| Check | Result |
| --- | --- |
| `just build` | App, stand-in library, and test built with no warnings |
| `clang-format --dry-run -Werror` on every C++ file | Clean |
| `just test test_plugins` | Exit 0. Output: `loaded: 7 plugins`; `signed: Hello, bob! #1 from 203.0.113.7`; `redirected: //guests//bob/ -> /guests/bob`; `rejected: request 4 with too many requests`. AccessLogger logged the resolved address. |
| `just run` with curl | `GET /guests/alice` with `X-Forwarded-For: 198.51.100.4` returned that address. `/guests/alice/` returned 302. `/config.json` returned 404 because `document_root` is outside the project. |
| `plugin_demo::GuestBook` entry removed | Startup check logged FATAL `Plugin plugin_demo::GuestBook is not loaded` and aborted (exit 134) |
| `drogon_greeter::Greeter` entry removed, dependency kept | `Dependent plugin drogon_greeter::Greeter is not loaded`, abort (exit 134) |
| `greeting` set to `""` | `terminate called after throwing an instance of 'std::invalid_argument'` with the plugin's message (exit 134) |
| Misspelled name added | `Plugin drogon_greeter::Greter undefined!` at ERROR level. The app kept running, and the test passed. |
| Core built as a STATIC library | All seven plugins loaded, but `Guests` was dropped and `/guests/bob` returned 404 |
| `Guests` constructor calling `getPlugin` | `isRunning()` was true, GuestBook was non-null, and Greeter was already initialized. The constructor logged before the beginning advice. |
| Executable linking the stand-in library without its header | `Plugin drogon_greeter::Greeter undefined!` |
| Same, including the header only | `undefined!`. With an added inline `Greeter() {}`, registered. |
| Same, calling `getPlugin<drogon_greeter::Greeter>()` | Registered |

## 7. Origin of the Conventions

The build setup (CMake >= 4.0, presets, Ninja, ccache, vcpkg classic mode, `just`, the `CMAKE_CXX_STANDARD` workaround, `tests/test_*.cpp` executables) and the error rules (throw in `initAndStart`, secrets only from environment variables) follow `drogon-plugin-lib-ref`, which records where they came from in `drogon-llm`. These parts are new to this skill:

- The OBJECT library, which addresses the static-link drop measured in section 6.
- The startup check, which turns drogon's ERROR-and-continue into the same abort drogon uses for a missing dependency.
- The `just test` recipe, which leaves `just run` free for the app.
