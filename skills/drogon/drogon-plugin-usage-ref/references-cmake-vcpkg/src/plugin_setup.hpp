/**
 * @file plugin_setup.hpp
 * @brief Declare the startup step that checks required plugins and configures them in code.
 */
#pragma once

namespace plugin_demo {

    /**
     * @brief Register a beginning advice that checks and configures plugins.
     *
     * The advice aborts if a plugin the code calls is missing, because drogon
     * only logs `Plugin ... undefined!` and keeps running. It then installs
     * Hodor's rejection response, which Hodor accepts only after `run()` starts.
     * Call this after `loadConfigFile()` and before `run()`.
     */
    auto RegisterPluginSetup() -> void;

} // namespace plugin_demo
