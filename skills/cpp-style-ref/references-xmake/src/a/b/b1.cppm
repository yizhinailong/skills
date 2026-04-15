export module a.b:b1;

import std;
import :b2;

namespace a::b {
    export auto FuncB1() -> void {
        std::println("Function FuncB1 from module a.b:b1");
        FuncB2();
    }
} // namespace a::b
