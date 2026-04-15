export module a;

export import a.b;
export import :a2;

import std;
import :a1;

namespace a {

    // error: exporting 'void a::FuncA1@a:a1()' that does not have external linkage
    // export using a::FuncA1;

    export auto FuncA() -> void {
        std::println("Function FuncA from module a");
        FuncA1();
        FuncA2();
    }

} // namespace a
