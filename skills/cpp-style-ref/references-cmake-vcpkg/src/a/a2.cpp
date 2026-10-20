/**
 * @file a2.cpp
 * @brief Implement the public helper and its internal dependency.
 */
#include "a/a2.hpp"

#include <print>

#include "a/a1.hpp"

namespace a {

    /**
     * @brief Print a diagnostic and call the internal helper.
     */
    auto func_a2() -> void {
        std::println("Function func_a2 from a/a2");
        func_a1();
    }

} // namespace a
