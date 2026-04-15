#include <print>

#include "a/a2.hpp"
#include "a/b/b1.hpp"
#include "a/b/b2.hpp"
#include "error.hpp"

auto main() -> int {
    // a::FuncA1();  // internal — not part of the public API
    a::FuncA2();

    std::println("---");

    a::b::FuncB1();
    a::b::FuncB2();

    Error err;
    err.Test();

    return 0;
}
