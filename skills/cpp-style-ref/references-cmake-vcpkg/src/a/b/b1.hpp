/**
 * @file b1.hpp
 * @brief Declare the first helper in a nested namespace.
 */
#pragma once

namespace a::b {

    /**
     * @brief Print a diagnostic and call the second helper.
     */
    auto func_b1() -> void;

} // namespace a::b
