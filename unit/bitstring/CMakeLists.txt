cmake_minimum_required(VERSION 3.15)
enable_testing()
add_executable(bitstring bitstringunit.cpp convert.cpp)
add_test(bitstring bitstring)
