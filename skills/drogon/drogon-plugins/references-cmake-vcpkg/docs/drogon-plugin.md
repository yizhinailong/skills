# drogon 插件：`drogon_echo::EchoClient`

本文整理 drogon-echo 作为 drogon 插件的接入方式、配置、生命周期、使用方法，以及与 drogon 线程模型相关的设计约束。

## 概览

`drogon_echo::EchoClient` 继承 `drogon::Plugin<EchoClient>`，职责只有两件：

1. 在 drogon 启动时从 config.json 读取并校验端点配置（`EchoConfig`）。
2. 通过 `NewEchoClient()` 生成绑定该配置的 `Echo` 客户端。

真正发请求的是 `Echo`：

```cpp
auto const echo{ drogon::app().getPlugin<drogon_echo::EchoClient>()->NewEchoClient() };
auto const message{ co_await echo.messages.Create({ .text = "Hello!" }) };
```

## 接入

库需要 C++23（公开头文件使用 `std::expected`），并通过 `drogon-echo::drogon-echo` 传递 `Drogon::Drogon` 依赖，消费方不需要再 `find_package(Drogon)`。两种方式任选：

```cmake
# 源码方式
add_subdirectory(path/to/drogon-echo)

# 安装方式：先在 drogon-echo 中执行 just publish，再把 publish/ 加进 CMAKE_PREFIX_PATH
find_package(drogon-echo CONFIG REQUIRED)

target_link_libraries(app PRIVATE drogon-echo::drogon-echo)
```

## 注册与配置

```json
"plugins": [
    {
        "name": "drogon_echo::EchoClient",
        "config": {
            "base_url": "http://127.0.0.1:18080",
            "api_key_env": "ECHO_API_KEY"
        }
    }
]
```

| 配置项 | 必填 | 说明 |
|---|---|---|
| `base_url` | 是 | 端点的基础 URL，可带路径前缀。请求发往 `<base_url>/messages`。 |
| `api_key_env` | 否 | 存放 API key 的**环境变量名**。省略时不发送 `Authorization` 头。 |

- API key 永远不写进 config.json，只从 `api_key_env` 指定的环境变量读取，以 `Authorization: Bearer <key>` 发送。
- `base_url` 缺失或为空，或设置了 `api_key_env` 但该环境变量未设置或为空时，`initAndStart` 抛 `std::invalid_argument`。异常从 `app().run()` 抛出，不捕获时程序终止。

## 生命周期

```
app().loadConfigFile("config.json")    读取 plugins 配置段
app().run()
 ├─ EchoClient::initAndStart(config)   解析并校验配置，失败时抛异常
 ├─ beginning advices                  从这里开始 getPlugin<EchoClient>() 可用
 ├─ startListening()                   各 IO loop 异步开始监听
 └─ 事件循环运行，处理请求
app().quit()                           drogon 调用 EchoClient::shutdown()
```

- `getPlugin` 返回 `EchoClient*`，由 drogon 持有，不要 delete。
- 静态链接时，应用代码必须引用 `EchoClient`（调用一次 `getPlugin<drogon_echo::EchoClient>()` 即可），否则链接器丢掉 `echo_client.o`，drogon 打印 `Plugin drogon_echo::EchoClient undefined!` 后继续运行。

## 使用

- `Echo` 是值句柄，内部唯一的共享状态是一个 `std::shared_ptr<details::Transport>`。拷贝只是引用计数加一，传进协程时按值传递。
- `Create` 返回 `drogon::Task<Result<Message>>`，`Result<T>` 是 `std::expected<T, Error>`。请求失败**不会抛异常**。

## 错误处理

| `ErrorKind` | 含义 | `status_code` | `body` |
|---|---|---|---|
| `Connection` | 没有收到响应：DNS、TLS、连接被拒等 | 0 | 空 |
| `Timeout` | 在 `timeout` 秒内没有收到响应 | 0 | 空 |
| `Status` | 端点返回非 2xx 状态 | HTTP 状态码 | 原始响应体 |
| `ResponseValidation` | 2xx 响应不是 JSON，或缺少 `text` | 0 | 响应内容 |

## 线程与事件循环

- 只提供协程接口。drogon 文档建议优先使用异步接口或协程，同步 API 会阻塞 IO 线程，在 client 所在的 loop 上同步等待还会死锁。
- `Transport::Post` 为每个请求在调用线程的事件循环上新建 `HttpClient`（调用线程没有 loop 时用主 loop），并在响应到达前一直留在协程帧里。drogon 的 `HttpClient` 默认 pipelining 深度为 0，共享一个 client 会让并发请求串行执行。
- 协程按值接收参数；`Messages::Create` 在第一次 `co_await` 之前把 `m_transport` 拷进协程帧，即使 `Echo` 在请求途中被销毁，请求也能完成。

## 测试程序

| 程序 | 作用 |
|---|---|
| `tests/test_config.cpp` | 加载 config.json，打印插件配置后退出。`just run test_config` |
| `tests/test_messages.cpp` | 由同一个 app 提供 `/messages`，发送一条正常消息和一条空消息，检查回显和 `Status` 错误。`just run test_messages` |
