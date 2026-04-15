// internal partition a1 of module a
export module a:a1;

import std;

namespace a {
    export auto FuncA1() -> void;

    auto FuncA1() -> void {
        std::println("Function FuncA1 from module a:a1");
    }
} // namespace a
