/**
 * @file b2.cppm
 * @brief Provide the nested module's second public helper.
 */
export module a.b:b2;

import std;

namespace a::b {

    /**
     * @brief Print a diagnostic for the second helper.
     */
    export auto func_b2() -> void {
        std::println("Function func_b2 from module a.b:b2");
    }

} // namespace a::b
