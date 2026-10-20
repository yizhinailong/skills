/**
 * @file b.cppm
 * @brief Re-export the nested module's public partitions.
 */
export module a.b;

export import :b1;
export import :b2;

import std;

namespace a::b {

    /**
     * @brief Run the nested module's public helpers.
     */
    export auto func_ab() -> void {
        std::println("Function func_ab from module a.b");
        func_b1();
        func_b2();
    }

} // namespace a::b
