cmake_minimum_required(VERSION 3.15)
enable_testing()
add_executable(itu ictunit.cpp cloned.cpp)
add_test(itu itu)
