/**
 * @file b2.cpp
 * @brief Implement the nested namespace's second helper.
 */
#include "a/b/b2.hpp"

#include <print>

namespace a::b {

    /**
     * @brief Print a diagnostic for the second helper.
     */
    void func_b2() {
        std::println("Function func_b2 from a/b/b2");
    }

} // namespace a::b
