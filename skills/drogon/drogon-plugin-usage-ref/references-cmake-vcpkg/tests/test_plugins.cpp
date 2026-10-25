/**
 * @file test_plugins.cpp
 * @brief Start the app with config.json and check plugins of all three kinds over HTTP.
 */
#include <array>
#include <exception>
#include <print>
#include <string>
#include <string_view>

#include <drogon/drogon.h>
#include <drogon/utils/coroutine.h>

#include "plugin_setup.hpp"

namespace {

    constexpr std::string_view BASE_URL{ "http://127.0.0.1:18080" }; ///< The listener in config.json.
    constexpr double LISTEN_DELAY{ 0.2 };                            ///< Seconds to wait for the listener after the beginning advices.
    constexpr double TIMEOUT{ 5.0 };                                 ///< Request timeout in seconds.
    constexpr int HODOR_IP_CAPACITY{ 3 };                            ///< `ip_capacity` of Hodor in config.json.

    /**
     * @brief Plugin names listed in config.json.
     *
     * Looked up by name, so plugins used only through config.json are covered as well.
     */
    constexpr std::array<std::string_view, 7> PLUGIN_NAMES{
        "drogon::plugin::Redirector",
        "drogon::plugin::SlashRemover",
        "drogon::plugin::RealIpResolver",
        "drogon::plugin::Hodor",
        "drogon::plugin::AccessLogger",
        "drogon_greeter::Greeter",
        "plugin_demo::GuestBook",
    };

    /**
     * @brief Check that drogon created every plugin listed in config.json.
     * @return True if `getPlugin(name)` found all of them.
     */
    auto check_loaded() -> bool {
        bool ok{ true };
        for (auto const name : PLUGIN_NAMES) {
            if (drogon::app().getPlugin(std::string{ name }) == nullptr) {
                std::println(stderr, "plugin {} is not loaded", name);
                ok = false;
            }
        }
        if (ok) {
            std::println("loaded: {} plugins", PLUGIN_NAMES.size());
        }
        return ok;
    }

    /**
     * @brief Send `GET path` to the app as if a proxy at 127.0.0.1 forwarded it for `client_ip`.
     * @param path Request path.
     * @param client_ip Address placed in `X-Forwarded-For`.
     * @return Task yielding the response.
     * @throws drogon::HttpException If the request fails.
     */
    auto get(std::string path, std::string client_ip) -> drogon::Task<drogon::HttpResponsePtr> {
        auto client{ drogon::HttpClient::newHttpClient(std::string{ BASE_URL }, trantor::EventLoop::getEventLoopOfCurrentThread()) };
        auto request{ drogon::HttpRequest::newHttpRequest() };
        request->setPath(path);
        request->addHeader("X-Forwarded-For", client_ip);
        co_return co_await client->sendRequestCoro(request, TIMEOUT);
    }

    /**
     * @brief Check GuestBook, Greeter, and RealIpResolver through `GET /guests/bob`.
     * @return Task yielding true if the greeting, number, and resolved address match.
     */
    auto check_sign() -> drogon::Task<bool> {
        auto const response{ co_await get("/guests/bob", "203.0.113.7") };
        auto const json{ response->getJsonObject() };
        if (response->statusCode() != drogon::k200OK || json == nullptr) {
            std::println(stderr, "sign: HTTP {}: {}", static_cast<int>(response->statusCode()), response->body());
            co_return false;
        }
        auto const greeting{ (*json)["greeting"].asString() };
        auto const number{ (*json)["number"].asUInt64() };
        auto const ip{ (*json)["ip"].asString() };
        if (greeting != "Hello, bob!" || number != 1 || ip != "203.0.113.7") {
            std::println(stderr, "sign: unexpected body {}", response->body());
            co_return false;
        }
        std::println("signed: {} #{} from {}", greeting, number, ip);
        co_return true;
    }

    /**
     * @brief Check SlashRemover and Redirector through a path with extra slashes.
     * @return Task yielding true if the app redirects to the cleaned path.
     */
    auto check_redirect() -> drogon::Task<bool> {
        auto const response{ co_await get("//guests//bob/", "203.0.113.7") };
        auto const& location{ response->getHeader("location") };
        if (response->statusCode() != drogon::k302Found || !location.ends_with("/guests/bob")) {
            std::println(stderr, "redirect: HTTP {} to {}", static_cast<int>(response->statusCode()), location);
            co_return false;
        }
        std::println("redirected: //guests//bob/ -> {}", location);
        co_return true;
    }

    /**
     * @brief Check Hodor and the rejection response installed by the beginning advice.
     * @return Task yielding true if one client gets `ip_capacity` answers and then a 429.
     */
    auto check_rate_limit() -> drogon::Task<bool> {
        for (int i{ 0 }; i < HODOR_IP_CAPACITY; ++i) {
            auto const response{ co_await get("/guests/carol", "203.0.113.8") };
            if (response->statusCode() != drogon::k200OK) {
                std::println(stderr, "rate limit: request {} got HTTP {}", i + 1, static_cast<int>(response->statusCode()));
                co_return false;
            }
        }
        auto const rejected{ co_await get("/guests/carol", "203.0.113.8") };
        auto const json{ rejected->getJsonObject() };
        if (rejected->statusCode() != drogon::k429TooManyRequests || json == nullptr) {
            std::println(stderr, "rate limit: HTTP {}: {}", static_cast<int>(rejected->statusCode()), rejected->body());
            co_return false;
        }
        std::println("rejected: request {} with {}", HODOR_IP_CAPACITY + 1, (*json)["error"]["message"].asString());
        co_return true;
    }

    /**
     * @brief Run every check, continuing after a failure.
     * @return Task yielding true if all checks passed.
     */
    auto run_checks() -> drogon::Task<bool> {
        bool ok{ check_loaded() };
        ok = co_await check_sign() && ok;
        ok = co_await check_redirect() && ok;
        ok = co_await check_rate_limit() && ok;
        co_return ok;
    }

} // namespace

auto main() -> int {
    int exit_code{ 1 };
    drogon::app().loadConfigFile("config.json");
    plugin_demo::RegisterPluginSetup();
    drogon::app().registerBeginningAdvice([&exit_code] {
        drogon::async_run([&exit_code]() -> drogon::Task<> {
            // Beginning advices run before startListening(), and each IO loop starts
            // listening asynchronously, so a request sent right away is refused.
            co_await drogon::sleepCoro(drogon::app().getLoop(), LISTEN_DELAY);
            try {
                exit_code = co_await run_checks() ? 0 : 1;
            } catch (std::exception const& error) {
                std::println(stderr, "request failed: {}", error.what());
            }
            drogon::app().quit();
        });
    });
    drogon::app().run();
    return exit_code;
}
