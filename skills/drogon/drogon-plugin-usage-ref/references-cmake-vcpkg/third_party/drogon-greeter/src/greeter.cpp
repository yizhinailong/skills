/**
 * @file greeter.cpp
 * @brief Implement the `Greeter` plugin of the stand-in third-party library.
 */
#include "drogon_greeter/greeter.hpp"

#include <format>
#include <stdexcept>

namespace drogon_greeter {

    /**
     * @brief Read the greeting word from the plugin's `config` object.
     * @param config The plugin's `config` object; `greeting` defaults to `Hello`.
     * @throws std::invalid_argument If `greeting` is empty.
     */
    void Greeter::initAndStart(Json::Value const& config) {
        m_greeting = config.get("greeting", "Hello").asString();
        if (m_greeting.empty()) {
            throw std::invalid_argument{ "drogon_greeter::Greeter: greeting must not be empty" };
        }
    }

    /**
     * @brief Release resources on application shutdown.
     */
    void Greeter::shutdown() {}

    /**
     * @brief Greet a visitor; safe to call from any thread.
     * @param name Visitor name.
     * @return Text such as `Hello, bob!`.
     */
    std::string Greeter::Greet(std::string_view name) const {
        return std::format("{}, {}!", m_greeting, name);
    }

} // namespace drogon_greeter
