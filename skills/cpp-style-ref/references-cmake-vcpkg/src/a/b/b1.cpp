/**
 * @file b1.cpp
 * @brief Implement the nested namespace's first helper.
 */
#include "a/b/b1.hpp"

#include <print>

#include "a/b/b2.hpp"

namespace a::b {

    /**
     * @brief Print a diagnostic and call the second helper.
     */
    void func_b1() {
        std::println("Function func_b1 from a/b/b1");
        func_b2();
    }

} // namespace a::b
