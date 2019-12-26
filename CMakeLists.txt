cmake_minimum_required(VERSION 3.15)

project(ict VERSION 0.0.1)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT MSVC)
    add_compile_options(
        -Wall -Wextra -Weverything -Wpedantic -fdiagnostics-color=always
        -Wno-implicit-fallthrough -Wno-keyword-macro
        -Wno-unneeded-internal-declaration
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-shadow-field-in-constructor
    )
endif ()

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(unit)
add_subdirectory(tools)
add_subdirectory(perf)
enable_testing()
