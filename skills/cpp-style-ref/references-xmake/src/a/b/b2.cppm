export module a.b:b2;

import std;

namespace a::b {
    export auto FuncB2() -> void {
        std::println("Function FuncB2 from module a.b:b2");
    }
} // namespace a::b
