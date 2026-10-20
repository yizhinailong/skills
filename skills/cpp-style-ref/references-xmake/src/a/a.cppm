/**
 * @file a.cppm
 * @brief Re-export public components and use an internal interface partition.
 */
export module a;

export import a.b;
export import :a2;

import std;
import :a1;

namespace a {

    /**
     * @brief Run the primary module's public and internal helpers.
     */
    export auto func_a() -> void {
        std::println("Function func_a from module a");
        func_a1();
        func_a2();
    }

} // namespace a
