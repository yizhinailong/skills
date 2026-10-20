/**
 * @file main.cpp
 * @brief Consume public module entry points and a separated implementation.
 */
import std;
import a;
import error;

/**
 * @brief Run the module examples.
 * @return Zero after all public calls complete.
 */
auto main() -> int {
    a::func_a();
    // a::func_a1() is unavailable because partition a:a1 is not re-exported.
    a::func_a2();

    std::println("---");

    a::b::func_ab();
    a::b::func_b1();
    a::b::func_b2();

    Error error;
    error.Test();

    return 0;
}
