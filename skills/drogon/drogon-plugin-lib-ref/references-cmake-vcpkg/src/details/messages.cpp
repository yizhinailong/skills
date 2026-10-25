/**
 * @file messages.cpp
 * @brief Implement the messages API group that `Echo::messages` exposes.
 */
#include "drogon_echo/details/messages.hpp"

#include <utility>

#include "details/transport.hpp"

namespace drogon_echo::details {

    namespace {

        /**
         * @brief Read a JSON string.
         * @param value Field that should hold a string.
         * @return The string, or unset if `value` is missing or not a string.
         */
        auto optional_string(Json::Value const& value) -> std::optional<std::string> {
            if (!value.isString()) {
                return std::nullopt;
            }
            return value.asString();
        }

        /**
         * @brief Build the `/messages` request body.
         * @param params Request fields.
         * @return Request body; unset optional fields are left out.
         */
        auto to_request_body(MessageCreateParams const& params) -> Json::Value {
            Json::Value body{ Json::objectValue };
            body["text"] = params.text;
            if (params.sender) {
                body["sender"] = *params.sender;
            }
            return body;
        }

        /**
         * @brief Build a `Message` from the `/messages` response body.
         * @param root Parsed response body.
         * @return The message, or a `ResponseValidation` error if `text` is not a string.
         */
        auto parse_message(Json::Value const& root) -> Result<Message> {
            if (!root.isObject() || !root["text"].isString()) {
                return std::unexpected{ Error{
                    .kind = ErrorKind::ResponseValidation,
                    .message = "message has no text string",
                    .body = root.toStyledString(),
                } };
            }
            return Message{
                .text = root["text"].asString(),
                .sender = optional_string(root["sender"]),
            };
        }

    } // namespace

    /**
     * @brief Bind the group to an endpoint.
     * @param transport Connection shared by the API groups of one `Echo`.
     */
    Messages::Messages(std::shared_ptr<Transport> transport)
        : m_transport{ std::move(transport) } {}

    /**
     * @brief Send a message and await the echoed copy.
     * @param params Request fields, taken by value so they outlive the caller's frame.
     * @return Task yielding the echoed message, or why the request failed.
     * @note The request runs on the calling thread's event loop (the main loop when the
     * thread runs none), so the coroutine resumes on the thread that awaited it.
     */
    auto Messages::Create(
        MessageCreateParams params
    ) const -> drogon::Task<Result<Message>> {
        // Keep the transport in the coroutine frame in case this object dies mid-request.
        auto const transport{ m_transport };
        auto const response{ co_await transport->Post("/messages", to_request_body(params), params.timeout) };
        co_return response.and_then(parse_message);
    }

} // namespace drogon_echo::details
