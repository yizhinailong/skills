/**
 * @file error.cppm
 * @brief Declare and implement a class in a combined module interface.
 */
export module error;

import std;

/**
 * @brief Provide a diagnostic method.
 */
export struct Error {
    /**
     * @brief Print a greeting.
     */
    auto Test() -> void {
        std::println("Hello");
    }
};
