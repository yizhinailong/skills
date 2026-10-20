/**
 * @file a2.cppm
 * @brief Provide a public interface partition that calls an internal helper.
 */
export module a:a2;

import :a1;

import std;

namespace a {

    /**
     * @brief Print a diagnostic and call the internal partition helper.
     */
    export auto func_a2() -> void {
        std::println("Function func_a2 from module a:a2");
        func_a1();
    }

} // namespace a
