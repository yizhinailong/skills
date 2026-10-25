/**
 * @file messages.hpp
 * @brief Declare the messages API group that `Echo::messages` exposes.
 */
#pragma once

#include <memory>

#include <drogon/utils/coroutine.h>

#include "drogon_echo/error.hpp"
#include "drogon_echo/types/message.hpp"

namespace drogon_echo::details {

    class Transport;

    /**
     * @brief Messages API group, reached through `echo.messages`.
     */
    class Messages {
    private:
        std::shared_ptr<Transport> m_transport; ///< Connection shared with the other API groups.

    public:
        /**
         * @brief Bind the group to an endpoint.
         * @param transport Connection shared by the API groups of one `Echo`.
         */
        explicit Messages(std::shared_ptr<Transport> transport);

        /**
         * @brief Send a message and await the echoed copy.
         * @param params Request fields, taken by value so they outlive the caller's frame.
         * @return Task yielding the echoed message, or why the request failed.
         * @note The request runs on the calling thread's event loop (the main loop when the
         * thread runs none), so the coroutine resumes on the thread that awaited it.
         */
        [[nodiscard]] auto Create(
            MessageCreateParams params
        ) const -> drogon::Task<Result<Message>>;
    };

} // namespace drogon_echo::details
