cmake_minimum_required(VERSION 3.2)
project(StatisticsStorage)

#set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-variable -Wno-reorder -Wno-unknown-pragmas -std=c++03")


set(SOURCE_FILES main.cpp StatisticsStorage.h)
add_executable(StatisticsStorage ${SOURCE_FILES})
