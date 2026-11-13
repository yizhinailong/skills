/**
 * @file config.cppm
 * @brief Declare validated configuration text owned by a module interface.
 */
export module style_ref:config;

import std;
export import :error;

export namespace style_ref {

    /**
     * @brief Validate and own configuration text independently of input views.
     */
    class StyleRef {
    public:
        /**
         * @brief Construct an empty configuration holder.
         */
        StyleRef() = default;

        /**
         * @brief Validate and copy configuration text.
         * @param config Text to copy; must not be empty.
         * @return Success or an invalid-configuration error, preserving stored text on rejection.
         * @throws std::bad_alloc If text or diagnostic allocation fails.
         * @throws std::length_error If the input exceeds string capacity.
         */
        [[nodiscard]] auto LoadConfig(std::string_view config) -> Result<void>;

        /**
         * @brief Borrow the stored configuration text.
         * @return View valid until this holder is changed or destroyed.
         */
        [[nodiscard]] auto Text() const noexcept -> std::string_view;

    private:
        std::string m_config_text; ///< Owned configuration text.

        /**
         * @brief Copy text after boundary validation.
         * @param config Validated text to copy.
         */
        auto loadConfig(std::string_view config) -> void;
    };

} // namespace style_ref
