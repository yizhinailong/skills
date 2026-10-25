/**
 * @file transport.cpp
 * @brief Implement the HTTP transport that the API groups of one `Echo` share.
 */
#include "details/transport.hpp"

#include <format>
#include <string_view>
#include <tuple>
#include <utility>

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>

namespace {

    /**
     * @brief Split a base URL into the host part for HttpClient and the path prefix.
     * @param base_url Base URL such as `https://api.example.com/v1`; a trailing `/` is ignored.
     * @return Host and path prefix, e.g. `https://api.example.com` and `/v1`; the prefix is empty without a path.
     */
    auto split_base_url(std::string_view base_url) -> std::pair<std::string, std::string> {
        if (base_url.ends_with('/')) {
            base_url.remove_suffix(1);
        }
        auto const scheme_end{ base_url.find("://") };
        auto const path_begin{ base_url.find('/', scheme_end == std::string_view::npos ? 0 : scheme_end + 3) };
        if (path_begin == std::string_view::npos) {
            return { std::string{ base_url }, std::string{} };
        }
        return {
            std::string{ base_url.substr(0, path_begin) },
            std::string{ base_url.substr(path_begin) }
        };
    }

    /**
     * @brief Describe a non-2xx response.
     * @param response Response from the endpoint.
     * @return The `error.message` field when the body has one, otherwise the raw body.
     */
    auto status_error_message(drogon::HttpResponse const& response) -> std::string {
        auto const& json{ response.getJsonObject() };
        if (json != nullptr && json->isObject()) {
            Json::Value const& error{ (*json)["error"] };
            if (error.isObject() && error["message"].isString()) {
                return error["message"].asString();
            }
        }
        return std::string{ response.body() };
    }

    /**
     * @brief Pick the event loop for a new HTTP client.
     * @return The calling thread's loop, or the app's main loop when the thread runs none.
     */
    auto current_loop() -> trantor::EventLoop* {
        auto* const loop{ trantor::EventLoop::getEventLoopOfCurrentThread() };
        return loop != nullptr ? loop : drogon::app().getLoop();
    }

} // namespace

namespace drogon_echo::details {

    /**
     * @brief Store the endpoint settings and split `config.base_url`.
     * @param config Endpoint settings; `api_key` may be empty for endpoints without auth.
     */
    Transport::Transport(EchoConfig config)
        : m_config{ std::move(config) } {
        std::tie(m_host, m_path_prefix) = split_base_url(m_config.base_url);
    }

    /**
     * @brief Return the endpoint settings.
     * @return Settings passed to the constructor.
     */
    auto Transport::Config() const noexcept -> EchoConfig const& {
        return m_config;
    }

    /**
     * @brief POST a JSON body and await the JSON response.
     * @param path Path below `base_url`, e.g. `/messages`.
     * @param body Request body.
     * @param timeout Seconds to wait for the response; 0 waits forever.
     * @return Task yielding the parsed body of a 2xx response, or why the request failed.
     * @note The caller keeps this object alive until the task completes.
     */
    auto Transport::Post(
        std::string path,
        Json::Value body,
        double timeout
    ) const -> drogon::Task<Result<Json::Value>> {
        auto request{ drogon::HttpRequest::newHttpJsonRequest(body) };
        request->setMethod(drogon::Post);
        request->setPath(m_path_prefix + path);
        if (!m_config.api_key.empty()) {
            request->addHeader("Authorization", "Bearer " + m_config.api_key);
        }

        // Keep the client in the coroutine frame until the response arrives.
        auto const client{ drogon::HttpClient::newHttpClient(m_host, current_loop()) };
        drogon::HttpResponsePtr response{};
        try {
            response = co_await client->sendRequestCoro(request, timeout);
        } catch (drogon::HttpException const& e) {
            co_return std::unexpected{ Error{
                .kind = e.code() == drogon::ReqResult::Timeout ? ErrorKind::Timeout : ErrorKind::Connection,
                .message = std::format("POST {}{}: {}", m_config.base_url, path, e.what()),
            } };
        }

        auto const status{ static_cast<int>(response->getStatusCode()) };
        if (status < 200 || status >= 300) {
            co_return std::unexpected{ Error{
                .kind = ErrorKind::Status,
                .message = std::format("HTTP {}: {}", status, status_error_message(*response)),
                .status_code = status,
                .body = std::string{ response->body() },
            } };
        }

        auto const& json{ response->getJsonObject() };
        if (json == nullptr) {
            co_return std::unexpected{ Error{
                .kind = ErrorKind::ResponseValidation,
                .message = std::format("response body is not JSON: {}", response->getJsonError()),
                .body = std::string{ response->body() },
            } };
        }
        co_return *json;
    }

} // namespace drogon_echo::details
