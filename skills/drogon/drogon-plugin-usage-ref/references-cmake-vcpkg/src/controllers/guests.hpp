/**
 * @file guests.hpp
 * @brief Declare the controller that signs visitors into the guest book.
 */
#pragma once

#include <string>

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

namespace plugin_demo {

    /**
     * @brief Serve `GET /guests/{name}` with plugins of all three kinds.
     *
     * The response combines `GuestBook` (in-project), which calls `Greeter`
     * (third-party), with the client address from `RealIpResolver` (built-in).
     */
    class Guests : public drogon::HttpController<Guests> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(Guests::Sign, "/guests/{name}", drogon::Get);
        METHOD_LIST_END

        /**
         * @brief Sign `name` into the guest book.
         * @param req Incoming request, taken by value so it outlives the caller's frame.
         * @param name Visitor name from the path.
         * @return Task yielding a JSON body with `greeting`, `number`, and `ip`.
         */
        auto Sign(drogon::HttpRequestPtr req, std::string name) -> drogon::Task<drogon::HttpResponsePtr>;
    };

} // namespace plugin_demo
