/**
 * @file plugin_setup.cpp
 * @brief Implement the startup step that checks required plugins and configures them in code.
 */
#include "plugin_setup.hpp"

#include <cstdlib>

#include <drogon/HttpAppFramework.h>
#include <drogon/plugins/Hodor.h>

#include "plugins/guest_book.hpp"

namespace {

    /**
     * @brief Fetch a plugin the code calls, or abort if it is not loaded.
     * @tparam T Plugin class.
     * @return The plugin; never null.
     */
    template <typename T>
    auto require_plugin() -> T* {
        auto* plugin{ drogon::app().getPlugin<T>() };
        if (plugin == nullptr) {
            // Same reaction as drogon's own check for a missing dependency (PluginsManager.cc).
            LOG_FATAL << "Plugin " << T::classTypeName() << " is not loaded; check config.json and the startup log";
            std::abort();
        }
        return plugin;
    }

    /**
     * @brief Build the response Hodor sends when a client exceeds its limit.
     * @return A 429 response with a JSON error body.
     */
    auto make_rejection(drogon::HttpRequestPtr const&) -> drogon::HttpResponsePtr {
        Json::Value body{ Json::objectValue };
        body["error"]["message"] = "too many requests";
        auto response{ drogon::HttpResponse::newHttpJsonResponse(body) };
        response->setStatusCode(drogon::k429TooManyRequests);
        return response;
    }

} // namespace

namespace plugin_demo {

    /**
     * @brief Register a beginning advice that checks and configures plugins.
     */
    auto RegisterPluginSetup() -> void {
        drogon::app().registerBeginningAdvice([] {
            require_plugin<GuestBook>();
            require_plugin<drogon::plugin::Hodor>()->setRejectResponseFactory(make_rejection);
        });
    }

} // namespace plugin_demo
