/**
 * @file config.cppm
 * @brief Own validated configuration text in a combined module interface.
 */
export module style_ref:config;

export import :error;
import std;

export namespace style_ref {

    /**
     * @brief Validate and own configuration text independently of input views.
     */
    class StyleRef {
    private:
        std::string m_config_text; ///< Owned configuration text.

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
        [[nodiscard]] auto LoadConfig(std::string_view config) -> Result<void> {
            if (config.empty()) {
                return std::unexpected{
                    Error{ ErrorCode::InvalidConfig, "Configuration text must not be empty." }
                };
            }
            loadConfig(config);
            return {};
        }

        /**
         * @brief Borrow the stored configuration text.
         * @return View valid until this holder is changed or destroyed.
         */
        [[nodiscard]] auto Text() const noexcept -> std::string_view {
            return m_config_text;
        }

    private:
        /**
         * @brief Copy text after boundary validation.
         * @param config Validated text to copy.
         */
        auto loadConfig(std::string_view config) -> void {
            m_config_text.assign(config);
        }
    };

} // namespace style_ref
