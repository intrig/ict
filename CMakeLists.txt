cmake_minimum_required(VERSION 3.15)

project(ict VERSION 0.0.1)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT MSVC)
    add_compile_options(-Wall -Wextra -Wpedantic -fdiagnostics-color=always)
endif ()

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(unit)
add_subdirectory(tools)
add_subdirectory(perf)
enable_testing()
