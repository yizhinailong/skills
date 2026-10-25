/**
 * @file error.hpp
 * @brief Declare the error type that failed requests return through `Result`.
 */
#pragma once

#include <expected>
#include <string>

namespace drogon_echo {

    /**
     * @brief Category of a failed request.
     */
    enum class ErrorKind {
        Connection,         ///< No response arrived, e.g. DNS, TLS, or network failure.
        Timeout,            ///< No response arrived within the timeout.
        Status,             ///< The endpoint answered with a non-2xx status.
        ResponseValidation, ///< A 2xx response did not have the expected shape.
    };

    /**
     * @brief Describe why a request failed.
     */
    struct Error {
        ErrorKind kind;      ///< Failure category.
        std::string message; ///< Human-readable description; for `Status`, the endpoint's `error.message` when present.
        int status_code{};   ///< HTTP status for `ErrorKind::Status`; 0 otherwise.
        std::string body;    ///< Response body for `Status` and `ResponseValidation`; empty otherwise.
    };

    /**
     * @brief Value of a successful request, or the error that prevented it.
     * @tparam T Value type on success.
     */
    template <typename T>
    using Result = std::expected<T, Error>;

} // namespace drogon_echo
