/**
 * @file b1.cppm
 * @brief Provide the nested module's first public helper.
 */
export module a.b:b1;

import std;
import :b2;

namespace a::b {

    /**
     * @brief Print a diagnostic and call the second helper.
     */
    export auto func_b1() -> void {
        std::println("Function func_b1 from module a.b:b1");
        func_b2();
    }

} // namespace a::b
