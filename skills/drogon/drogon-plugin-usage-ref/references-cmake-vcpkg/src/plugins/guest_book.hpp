/**
 * @file guest_book.hpp
 * @brief Declare the in-project `GuestBook` plugin, which numbers visitors and greets them.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>

#include <drogon/plugins/Plugin.h>

#include <drogon_greeter/greeter.hpp>

namespace plugin_demo {

    /**
     * @brief One signature in the guest book.
     */
    struct Entry {
        std::string greeting; ///< Greeting from the `Greeter` plugin.
        std::uint64_t number; ///< Visitor number, unique per process.
    };

    /**
     * @brief In-project plugin that numbers visitors and greets them through `Greeter`.
     *
     * Register it in config.json after the plugin it depends on:
     * @code{.json}
     * {
     *     "name": "plugin_demo::GuestBook",
     *     "dependencies": ["drogon_greeter::Greeter"],
     *     "config": { "first_number": 1 }
     * }
     * @endcode
     *
     * Handlers on any IO thread may call `Sign()` concurrently.
     */
    class GuestBook : public drogon::Plugin<GuestBook> {
    private:
        drogon_greeter::Greeter const* m_greeter{}; ///< Dependency fetched in `initAndStart()`.
        std::atomic<std::uint64_t> m_next{ 1 };     ///< Number of the next visitor.

    public:
        /**
         * @brief Read the first visitor number and fetch the `Greeter` dependency.
         * @param config The plugin's `config` object; `first_number` defaults to 1.
         * @throws std::invalid_argument If `Greeter` is not loaded.
         */
        auto initAndStart(Json::Value const& config) -> void override;

        /**
         * @brief Release resources on application shutdown.
         */
        auto shutdown() -> void override;

        /**
         * @brief Greet a visitor and assign the next visitor number.
         * @param name Visitor name.
         * @return The greeting and the assigned number.
         */
        [[nodiscard]] auto Sign(std::string_view name) -> Entry;
    };

} // namespace plugin_demo
