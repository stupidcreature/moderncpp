cmake_minimum_required(VERSION 2.8)


project(ModernCpp)


set(CMAKE_CXX_FLAGS "--std=gnu++1z ${CMAKE_CXX_FLAGS}")


set(CMAKE_CXX_STANDARD 14)


add_subdirectory(learning_modern_cpp)
add_subdirectory(learning_gtest)
