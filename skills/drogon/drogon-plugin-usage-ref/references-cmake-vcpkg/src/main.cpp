/**
 * @file main.cpp
 * @brief Start the plugin-demo application.
 */
#include <drogon/drogon.h>

#include "plugin_setup.hpp"

auto main() -> int {
    // Including <drogon/drogon.h> also registers drogon's built-in plugins in a static build.
    drogon::app().loadConfigFile("config.json");
    plugin_demo::RegisterPluginSetup();
    drogon::app().run();
    return 0;
}
