/**
 * @file message.hpp
 * @brief Declare the request and response types of the messages API.
 */
#pragma once

#include <optional>
#include <string>

namespace drogon_echo {

    /**
     * @brief Arguments of `Messages::Create()`.
     *
     * Unset optional fields are left out of the request body, so the endpoint's defaults apply.
     */
    struct MessageCreateParams {
        std::string text;                    ///< Message text; the endpoint rejects an empty one.
        std::optional<std::string> sender{}; ///< Optional sender name.
        double timeout{ 30.0 };              ///< Seconds to wait for the response; 0 waits forever.
    };

    /**
     * @brief Message the endpoint sent back.
     */
    struct Message {
        std::string text;                    ///< Echoed message text.
        std::optional<std::string> sender{}; ///< Echoed sender; unset when the request had none.
    };

} // namespace drogon_echo
