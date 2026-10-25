# Stand-in for a third-party drogon plugin library, vendored so the example
# builds offline. A real package is consumed through find_package or CPM; see
# the top-level CMakeLists.txt.
cmake_minimum_required(VERSION 4.0.0)

project(
    drogon-greeter
    LANGUAGES CXX
    VERSION 0.1.0
)

find_package(Drogon CONFIG REQUIRED)

# Drogon's FindFilesystem.cmake restores an unset CMAKE_CXX_STANDARD as "",
# which target_compile_features rejects as an invalid CXX_STANDARD.
if(CMAKE_CXX_STANDARD STREQUAL "")
    unset(CMAKE_CXX_STANDARD)
endif()

add_library(${PROJECT_NAME} src/greeter.cpp)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    Drogon::Drogon
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
