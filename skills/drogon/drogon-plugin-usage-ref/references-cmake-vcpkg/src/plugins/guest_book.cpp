/**
 * @file guest_book.cpp
 * @brief Implement the in-project `GuestBook` plugin.
 */
#include "plugins/guest_book.hpp"

#include <stdexcept>

#include <drogon/HttpAppFramework.h>

namespace plugin_demo {

    /**
     * @brief Read the first visitor number and fetch the `Greeter` dependency.
     * @param config The plugin's `config` object; `first_number` defaults to 1.
     * @throws std::invalid_argument If `Greeter` is not loaded.
     */
    auto GuestBook::initAndStart(Json::Value const& config) -> void {
        // Listing Greeter under "dependencies" makes drogon initialize it first.
        m_greeter = drogon::app().getPlugin<drogon_greeter::Greeter>();
        if (m_greeter == nullptr) {
            throw std::invalid_argument{ "plugin_demo::GuestBook: drogon_greeter::Greeter is not loaded" };
        }
        m_next = config.get("first_number", 1).asUInt64();
    }

    /**
     * @brief Release resources on application shutdown.
     */
    auto GuestBook::shutdown() -> void {}

    /**
     * @brief Greet a visitor and assign the next visitor number.
     * @param name Visitor name.
     * @return The greeting and the assigned number.
     */
    auto GuestBook::Sign(std::string_view name) -> Entry {
        return Entry{
            .greeting = m_greeter->Greet(name),
            .number = m_next.fetch_add(1),
        };
    }

} // namespace plugin_demo
