/**
 * @file error.cppm
 * @brief Provide configuration errors and expected-based results.
 */
export module style_ref:error;

import std;

export namespace style_ref {

    /**
     * @brief Classify recoverable configuration failures.
     */
    enum class ErrorCode {
        InvalidConfig, ///< Configuration text is empty.
    };

    /**
     * @brief Own a configuration error and its diagnostic.
     */
    struct Error {
        ErrorCode code{ ErrorCode::InvalidConfig }; ///< Failure classification.
        std::string message;                        ///< Owned diagnostic text.
    };

    /**
     * @brief Represent a successful value or a recoverable error.
     * @tparam T Success value type; void means no value is returned.
     */
    template <typename T = void>
    using Result = std::expected<T, Error>;

} // namespace style_ref
