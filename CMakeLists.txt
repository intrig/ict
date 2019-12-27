cmake_minimum_required(VERSION 3.15)

project(ict VERSION 0.0.1)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
message("compiler id: " ${CMAKE_CXX_COMPILER_ID})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -Wall -Wextra -Wpedantic -fdiagnostics-color=always
    )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic -fdiagnostics-color=always
        #-Weverything
        -Wno-implicit-fallthrough -Wno-keyword-macro
        -Wno-unneeded-internal-declaration
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-shadow-field-in-constructor
        -Wno-padded
        -Wno-sign-conversion
        -Wno-weak-vtables
        #-Werror
    )
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(unit)
add_subdirectory(tools)
add_subdirectory(perf)
enable_testing()
