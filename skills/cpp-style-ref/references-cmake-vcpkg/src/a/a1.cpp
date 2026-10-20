/**
 * @file a1.cpp
 * @brief Implement the internal diagnostic helper.
 */
#include "a/a1.hpp"

#include <print>

namespace a {

    /**
     * @brief Print a diagnostic for the internal helper.
     */
    auto func_a1() -> void {
        std::println("Function func_a1 from a/a1 (internal)");
    }

} // namespace a
