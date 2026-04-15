export module a:a2;

import :a1;

import std;

namespace a {
    export auto FuncA2() -> void {
        std::println("Function FuncA2 from module a:a2");
        FuncA1();
    }
} // namespace a
