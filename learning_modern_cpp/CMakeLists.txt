cmake_minimum_required(VERSION 3.2)

project(learning_modern_cpp)


set(CMAKE_CXX_FLAGS "-march=native -O3 --std=gnu++1z -pthread ${CMAKE_CXX_FLAGS}")


set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp ../learning_gtest/SomeClassToTest.cpp)


add_executable(ModernCpp ${SOURCE_FILES})
