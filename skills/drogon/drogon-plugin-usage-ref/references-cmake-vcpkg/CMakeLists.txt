cmake_minimum_required(VERSION 4.0.0)

project(
    plugin-demo
    LANGUAGES CXX
    VERSION 0.1.0
)

option(PLUGIN_DEMO_BUILD_TESTS "Build the test executables" ${PROJECT_IS_TOP_LEVEL})

find_package(Drogon CONFIG REQUIRED)

# Drogon's FindFilesystem.cmake restores an unset CMAKE_CXX_STANDARD as "",
# which target_compile_features rejects as an invalid CXX_STANDARD.
if(CMAKE_CXX_STANDARD STREQUAL "")
    unset(CMAKE_CXX_STANDARD)
endif()

# Third-party plugin library. This stand-in is vendored; for a real package use
# find_package(drogon-<x> CONFIG REQUIRED) or CPMAddPackage instead.
add_subdirectory(third_party/drogon-greeter)

file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
list(REMOVE_ITEM SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# An OBJECT library puts every object file into each executable that links it,
# so no controller or in-project plugin is dropped by the linker.
add_library(${PROJECT_NAME}-core OBJECT ${SOURCE})

target_include_directories(
    ${PROJECT_NAME}-core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    ${PROJECT_NAME}-core
    PUBLIC
    Drogon::Drogon
    drogon-greeter::drogon-greeter
)

target_compile_features(${PROJECT_NAME}-core PUBLIC cxx_std_23)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-core)

if(PLUGIN_DEMO_BUILD_TESTS)
    add_subdirectory(tests)
endif()
