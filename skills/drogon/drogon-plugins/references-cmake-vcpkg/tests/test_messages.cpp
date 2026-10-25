/**
 * @file test_messages.cpp
 * @brief Send messages through `Echo::messages` to an echo handler served by the same app.
 */
#include <print>
#include <string>
#include <string_view>

#include <drogon/drogon.h>
#include <drogon/utils/coroutine.h>

#include <drogon_echo/echo_client.hpp>

namespace {

    constexpr std::string_view TEXT{ "Hello!" }; ///< Message to send.
    constexpr double LISTEN_DELAY{ 0.2 };        ///< Seconds to wait for the listener after the beginning advices.

    /**
     * @brief Echo the JSON body back, or reject a missing or empty `text` with 400.
     * @param req Incoming request, taken by value so it outlives the caller's frame.
     * @return Task yielding the response.
     */
    auto handle_messages(drogon::HttpRequestPtr req) -> drogon::Task<drogon::HttpResponsePtr> {
        auto const& json{ req->getJsonObject() };
        if (json == nullptr || !(*json)["text"].isString() || (*json)["text"].asString().empty()) {
            Json::Value error{ Json::objectValue };
            error["error"]["message"] = "text must be a non-empty string";
            auto response{ drogon::HttpResponse::newHttpJsonResponse(error) };
            response->setStatusCode(drogon::k400BadRequest);
            co_return response;
        }
        co_return drogon::HttpResponse::newHttpJsonResponse(*json);
    }

    /**
     * @brief Send one valid and one rejected message and check both results.
     * @param echo Client created by the EchoClient plugin, copied so it outlives the caller's frame.
     * @return True if the valid message came back unchanged and the empty one failed with `Status`.
     */
    auto send_messages(drogon_echo::Echo echo) -> drogon::Task<bool> {
        auto const echoed{ co_await echo.messages.Create({
            .text = std::string{ TEXT },
            .sender = "test_messages",
        }) };
        if (!echoed) {
            std::println(stderr, "request failed: {}", echoed.error().message);
            co_return false;
        }
        if (echoed->text != TEXT) {
            std::println(stderr, "unexpected echo: {}", echoed->text);
            co_return false;
        }
        std::println("echoed: {} (sender {})", echoed->text, echoed->sender.value_or("(none)"));

        auto const rejected{ co_await echo.messages.Create({ .text = "" }) };
        if (rejected || rejected.error().kind != drogon_echo::ErrorKind::Status) {
            std::println(stderr, "empty text was not rejected with an HTTP status error");
            co_return false;
        }
        std::println("rejected: {}", rejected.error().message);
        co_return true;
    }

} // namespace

auto main() -> int {
    int exit_code{ 1 };
    drogon::app()
        .loadConfigFile("config.json")
        .registerHandler("/messages", &handle_messages, { drogon::Post })
        .registerBeginningAdvice([&exit_code] {
            drogon::async_run([&exit_code]() -> drogon::Task<> {
                // Beginning advices run before startListening(), and each IO loop starts
                // listening asynchronously, so a request sent right away is refused.
                co_await drogon::sleepCoro(drogon::app().getLoop(), LISTEN_DELAY);
                bool const ok{
                    co_await send_messages(drogon::app().getPlugin<drogon_echo::EchoClient>()->NewEchoClient())
                };
                exit_code = ok ? 0 : 1;
                drogon::app().quit();
            });
        })
        .run();
    return exit_code;
}
