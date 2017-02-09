cmake_minimum_required(VERSION 3.2)
project(StatisticsStorage)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp TempStatistics.cpp StatisticsStorage.cpp StatisticsStorage.h)
add_executable(StatisticsStorage ${SOURCE_FILES})
