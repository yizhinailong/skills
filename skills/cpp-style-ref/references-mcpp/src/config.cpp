/**
 * @file config.cpp
 * @brief Implement the configuration partition in a module implementation unit.
 */
module style_ref;

import std;

namespace style_ref {

    auto StyleRef::LoadConfig(std::string_view config) -> Result<void> {
        if (config.empty()) {
            return std::unexpected{
                Error{ ErrorCode::InvalidConfig, "Configuration text must not be empty." }
            };
        }
        loadConfig(config);
        return {};
    }

    auto StyleRef::Text() const noexcept -> std::string_view {
        return m_config_text;
    }

    auto StyleRef::loadConfig(std::string_view config) -> void {
        m_config_text.assign(config);
    }

} // namespace style_ref
