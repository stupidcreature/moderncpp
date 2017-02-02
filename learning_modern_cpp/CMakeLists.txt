cmake_minimum_required(VERSION 2.8)

project(learning_modern_cpp)


set(CMAKE_CXX_FLAGS "--std=gnu++1z -pthread ${CMAKE_CXX_FLAGS}")


set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp SomeClassToTest.cpp)


add_executable(ModernCpp ${SOURCE_FILES})
