/**
 * @file a1.cppm
 * @brief Provide an interface partition used internally without re-exporting it.
 */
export module a:a1;

import std;

namespace a {

    /**
     * @brief Print a diagnostic for the internal helper.
     */
    export auto func_a1() -> void {
        std::println("Function func_a1 from module a:a1");
    }

} // namespace a
