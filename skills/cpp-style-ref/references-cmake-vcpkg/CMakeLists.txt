cmake_minimum_required(VERSION 3.25)

project(cpp-style-ref LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- error 库（分离式：.hpp 声明 + .cpp 实现）---
add_library(error STATIC
    src/error.cpp
)
target_include_directories(error PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- 主程序 ---
add_executable(cpp-style-ref
    src/main.cpp
    src/a/a1.cpp
    src/a/a2.cpp
    src/a/b/b1.cpp
    src/a/b/b2.cpp
)
target_include_directories(cpp-style-ref PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(cpp-style-ref PRIVATE error)
