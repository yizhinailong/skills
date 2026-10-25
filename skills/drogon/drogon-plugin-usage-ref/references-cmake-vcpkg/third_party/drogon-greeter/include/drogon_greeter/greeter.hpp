/**
 * @file greeter.hpp
 * @brief Declare the `Greeter` plugin of the stand-in third-party library.
 */
#pragma once

#include <string>
#include <string_view>

#include <drogon/plugins/Plugin.h>

namespace drogon_greeter {

    /**
     * @brief Drogon plugin that formats greetings.
     *
     * Register it in the application's config.json:
     * @code{.json}
     * "plugins": [{
     *     "name": "drogon_greeter::Greeter",
     *     "config": { "greeting": "Hello" }
     * }]
     * @endcode
     *
     * Then fetch it once `app().run()` has started:
     * `drogon::app().getPlugin<drogon_greeter::Greeter>()`.
     *
     * @note With a static build, the plugin registers itself only if the
     * application includes this header or references the class. Otherwise the
     * linker drops it and drogon logs `Plugin drogon_greeter::Greeter undefined!`.
     */
    class Greeter : public drogon::Plugin<Greeter> {
    private:
        std::string m_greeting; ///< Greeting word parsed in `initAndStart()`.

    public:
        /**
         * @brief Read the greeting word from the plugin's `config` object.
         * @param config The plugin's `config` object; `greeting` defaults to `Hello`.
         * @throws std::invalid_argument If `greeting` is empty.
         */
        auto initAndStart(Json::Value const& config) -> void override;

        /**
         * @brief Release resources on application shutdown.
         */
        auto shutdown() -> void override;

        /**
         * @brief Greet a visitor; safe to call from any thread.
         * @param name Visitor name.
         * @return Text such as `Hello, bob!`.
         */
        [[nodiscard]] auto Greet(std::string_view name) const -> std::string;
    };

} // namespace drogon_greeter
