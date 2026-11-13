/**
 * @file test_config.cpp
 * @brief Verify owned text, error diagnostics, and state preservation through the public module.
 */
import std;
import mcpplibs.mtest;
import style_ref;

using namespace mcpplibs::mtest;

/**
 * @brief Verify a default holder owns no text.
 */
UTest default_text{"a default holder owns no text", [] {
    style_ref::StyleRef config;
    expect(config.Text().empty());
}};

/**
 * @brief Verify accepted text is copied into owned storage.
 */
UTest owns_text{"accepted text is copied, not borrowed", [] {
    style_ref::StyleRef config;
    std::string input{ "mode=debug" };
    expect(config.LoadConfig(input).has_value());
    input.assign("changed");
    expect(eq(config.Text(), std::string_view{ "mode=debug" }));
}};

/**
 * @brief Verify rejection reports invalid configuration and preserves the previous text.
 */
UTest rejects_empty{"empty text is rejected and the previous text preserved", [] {
    style_ref::StyleRef config;
    expect(config.LoadConfig("mode=debug").has_value());
    auto const rejected{ config.LoadConfig("") };
    expect(!rejected.has_value());
    if (!rejected) {
        expect(rejected.error().code == style_ref::ErrorCode::InvalidConfig);
        expect(!rejected.error().message.empty());
    }
    expect(eq(config.Text(), std::string_view{ "mode=debug" }));
}};
