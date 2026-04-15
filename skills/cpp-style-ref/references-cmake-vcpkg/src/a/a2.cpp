#include "a/a2.hpp"
#include "a/a1.hpp"

namespace a {

auto FuncA2() -> void {
    std::println("Function FuncA2 from a/a2");
    FuncA1();
}

} // namespace a
