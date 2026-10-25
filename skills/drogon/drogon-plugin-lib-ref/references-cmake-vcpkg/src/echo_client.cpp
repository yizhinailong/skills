/**
 * @file echo_client.cpp
 * @brief Implement the `Echo` client and the drogon plugin that holds its endpoint settings.
 */
#include "drogon_echo/echo_client.hpp"

#include <cstdlib>
#include <format>
#include <memory>
#include <stdexcept>
#include <utility>

#include "details/transport.hpp"

namespace {

    /**
     * @brief Read the API key from the named environment variable.
     * @param env_name Variable name from `api_key_env`; empty when the endpoint needs no key.
     * @return The variable's value, or an empty string when `env_name` is empty.
     * @throws std::invalid_argument If the variable is unset or empty.
     */
    auto read_api_key(std::string const& env_name) -> std::string {
        if (env_name.empty()) {
            return {};
        }
        char const* value{ std::getenv(env_name.c_str()) };
        if (value == nullptr || *value == '\0') {
            throw std::invalid_argument{ std::format("drogon_echo::EchoClient: environment variable {} is unset or empty", env_name) };
        }
        return value;
    }

} // namespace

namespace drogon_echo {

    /**
     * @brief Create a client for the endpoint in `config`.
     * @param config Endpoint settings; `api_key` may be empty for endpoints without auth.
     */
    Echo::Echo(EchoConfig config)
        : messages{ std::make_shared<details::Transport>(std::move(config)) } {}

    /**
     * @brief Read the endpoint settings from the plugin's `config` object.
     * @param config The plugin's `config` object from config.json.
     * @throws std::invalid_argument If `base_url` is missing or empty, or if
     * `api_key_env` names an unset or empty environment variable.
     */
    auto EchoClient::initAndStart(Json::Value const& config) -> void {
        m_config = EchoConfig{
            .base_url = config.get("base_url", "").asString(),
            .api_key = read_api_key(config.get("api_key_env", "").asString()),
        };
        if (m_config.base_url.empty()) {
            throw std::invalid_argument{ "drogon_echo::EchoClient: config needs a non-empty base_url" };
        }
    }

    /**
     * @brief Release resources on application shutdown.
     */
    auto EchoClient::shutdown() -> void {}

    /**
     * @brief Return the settings parsed from config.json.
     * @return Settings valid after drogon has initialized the plugin.
     */
    auto EchoClient::Config() const noexcept -> EchoConfig const& {
        return m_config;
    }

    /**
     * @brief Create an `Echo` client for the configured endpoint.
     * @return A new client; call only after drogon has initialized the plugin.
     */
    auto EchoClient::NewEchoClient() const -> Echo {
        return Echo{ m_config };
    }

} // namespace drogon_echo
