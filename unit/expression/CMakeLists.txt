cmake_minimum_required(VERSION 3.15)
enable_testing()
add_executable(expression expressionunit.cpp)
add_test(expression expression)
