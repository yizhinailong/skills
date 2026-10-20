/**
 * @file test_config.cpp
 * @brief Verify owned text, error diagnostics, and state preservation through the public module.
 */
import std;
import style_ref;

namespace {

    /**
     * @brief Report a failed behavioral expectation.
     * @param condition Whether the expectation holds.
     * @param message Failure diagnostic.
     * @return Whether the expectation holds.
     */
    auto check(bool condition, std::string_view message) -> bool {
        if (!condition) {
            std::println("test_config: {}", message);
        }
        return condition;
    }

} // namespace

/**
 * @brief Verify configuration behavior without importing partitions directly.
 * @return Zero when all checks pass, otherwise one.
 */
auto main() -> int {
    style_ref::StyleRef config;
    bool passed{ check(config.Text().empty(), "default text must be empty") };

    std::string input{ "mode=debug" };
    auto const loaded = config.LoadConfig(input);
    passed &= check(loaded.has_value(), "nonempty text must load successfully");
    input.assign("changed");
    passed &= check(config.Text() == "mode=debug", "stored text must own its bytes");

    auto const rejected = config.LoadConfig("");
    passed &= check(!rejected, "empty text must be rejected");
    if (!rejected) {
        passed &= check(rejected.error().code == style_ref::ErrorCode::InvalidConfig, "failure must identify invalid configuration");
        passed &= check(!rejected.error().message.empty(), "failure must retain a diagnostic");
    }
    passed &= check(config.Text() == "mode=debug", "rejection must preserve the previous text");

    return passed ? 0 : 1;
}
