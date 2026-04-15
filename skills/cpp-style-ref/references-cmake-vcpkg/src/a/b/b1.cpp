#include "a/b/b1.hpp"
#include "a/b/b2.hpp"

namespace a::b {

auto FuncB1() -> void {
    std::println("Function FuncB1 from a/b/b1");
    FuncB2();
}

} // namespace a::b
