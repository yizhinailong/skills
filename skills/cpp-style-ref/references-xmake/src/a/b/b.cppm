export module a.b;

export import :b1;
export import :b2;

import std;

namespace a::b {
    export auto FuncAb() -> void {
        std::println("Function FuncAb from module a.b");
        FuncB1();
        FuncB2();
    }
} // namespace a::b
