/**
 * @file main.cpp
 * @brief Consume public headers and a separately implemented class.
 */
#include <print>

#include "a/a2.hpp"
#include "a/b/b1.hpp"
#include "a/b/b2.hpp"
#include "error.hpp"

/**
 * @brief Run the header/source examples.
 * @return Zero after all public calls complete.
 */
auto main() -> int {
    // a/a1.hpp is an internal header and is not included by public headers.
    a::func_a2();

    std::println("---");

    a::b::func_b1();
    a::b::func_b2();

    Error error;
    error.Test();

    return 0;
}
