cmake_minimum_required(VERSION 3.15)

project(ict VERSION 0.0.1)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -Wall -Wextra -Wpedantic -fdiagnostics-color=always
        -Werror
    )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -fdiagnostics-color=always
        -Weverything
        -Wno-implicit-fallthrough -Wno-keyword-macro
        -Wno-unneeded-internal-declaration
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-shadow-field-in-constructor
        -Wno-sign-conversion
        -Wno-implicit-float-conversion

        # These are more informational
        -Wno-padded
        -Wno-weak-vtables
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-switch-enum

        -Werror
    )
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(unit)
add_subdirectory(tools)
add_subdirectory(perf)
enable_testing()
