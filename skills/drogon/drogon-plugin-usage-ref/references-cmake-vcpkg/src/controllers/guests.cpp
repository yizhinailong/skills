/**
 * @file guests.cpp
 * @brief Implement the controller that signs visitors into the guest book.
 */
#include "controllers/guests.hpp"

#include <drogon/HttpAppFramework.h>
#include <drogon/plugins/RealIpResolver.h>

#include "plugins/guest_book.hpp"

namespace plugin_demo {

    /**
     * @brief Sign `name` into the guest book.
     * @param req Incoming request, taken by value so it outlives the caller's frame.
     * @param name Visitor name from the path.
     * @return Task yielding a JSON body with `greeting`, `number`, and `ip`.
     */
    auto Guests::Sign(drogon::HttpRequestPtr req, std::string name) -> drogon::Task<drogon::HttpResponsePtr> {
        // getPlugin<T>() caches the pointer after its first call, so a lookup per request is cheap.
        // The beginning advice in plugin_setup.cpp has already checked that GuestBook is loaded.
        auto const entry{ drogon::app().getPlugin<GuestBook>()->Sign(name) };

        Json::Value body{ Json::objectValue };
        body["greeting"] = entry.greeting;
        body["number"] = Json::UInt64{ entry.number };
        // GetRealAddr falls back to the peer address when RealIpResolver is not loaded.
        body["ip"] = drogon::plugin::RealIpResolver::GetRealAddr(req).toIp();
        co_return drogon::HttpResponse::newHttpJsonResponse(body);
    }

} // namespace plugin_demo
