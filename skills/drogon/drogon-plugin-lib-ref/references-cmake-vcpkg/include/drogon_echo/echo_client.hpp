/**
 * @file echo_client.hpp
 * @brief Declare the `Echo` client and the drogon plugin that holds its endpoint settings.
 */
#pragma once

#include <string>

#include <drogon/plugins/Plugin.h>

#include "drogon_echo/details/messages.hpp"

namespace drogon_echo {

    /**
     * @brief Endpoint settings of an `Echo`; the plugin reads them from its `config` object.
     */
    struct EchoConfig {
        std::string base_url; ///< API base URL, e.g. `http://127.0.0.1:18080`; may carry a path prefix.
        std::string api_key;  ///< Bearer token from the `api_key_env` variable; empty without it.
    };

    /**
     * @brief Client for one echo endpoint.
     *
     * @code
     * auto const message{ co_await echo.messages.Create({ .text = "Hello!" }) };
     * if (message) {
     *     std::println("{}", message->text);
     * }
     * @endcode
     *
     * A cheap handle: copies share one transport, which is safe to use from any thread.
     * Pass it to coroutines by value.
     */
    class Echo final {
    public:
        details::Messages messages; ///< Messages API group.

        /**
         * @brief Create a client for the endpoint in `config`.
         * @param config Endpoint settings; `api_key` may be empty for endpoints without auth.
         */
        explicit Echo(EchoConfig config);
    };

    /**
     * @brief Drogon plugin that owns the echo endpoint settings.
     *
     * Register it in the application's config.json:
     * @code{.json}
     * "plugins": [{
     *     "name": "drogon_echo::EchoClient",
     *     "config": { "base_url": "http://127.0.0.1:18080", "api_key_env": "ECHO_API_KEY" }
     * }]
     * @endcode
     *
     * The API key is never read from config.json: `api_key_env` names the
     * environment variable that holds it. Omit `api_key_env` for endpoints
     * without auth.
     *
     * Then fetch it once `app().run()` has started, e.g. from a beginning advice
     * or a handler: `drogon::app().getPlugin<drogon_echo::EchoClient>()`, and
     * call `NewEchoClient()` for an `Echo` bound to these settings.
     *
     * @note With a static build, the plugin registers itself only if the
     * application references this class (calling `getPlugin<EchoClient>()` is
     * enough). Otherwise the linker drops it and drogon cannot find the name.
     */
    class EchoClient : public drogon::Plugin<EchoClient> {
    private:
        EchoConfig m_config; ///< Settings parsed in `initAndStart()`.

    public:
        /**
         * @brief Read the endpoint settings from the plugin's `config` object.
         * @param config The plugin's `config` object from config.json.
         * @throws std::invalid_argument If `base_url` is missing or empty, or if
         * `api_key_env` names an unset or empty environment variable.
         */
        auto initAndStart(Json::Value const& config) -> void override;

        /**
         * @brief Release resources on application shutdown.
         */
        auto shutdown() -> void override;

        /**
         * @brief Return the settings parsed from config.json.
         * @return Settings valid after drogon has initialized the plugin.
         */
        [[nodiscard]] auto Config() const noexcept -> EchoConfig const&;

        /**
         * @brief Create an `Echo` client for the configured endpoint.
         * @return A new client; call only after drogon has initialized the plugin.
         */
        [[nodiscard]] auto NewEchoClient() const -> Echo;
    };

} // namespace drogon_echo
