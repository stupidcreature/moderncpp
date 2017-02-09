cmake_minimum_required(VERSION 3.2)
project(StatisticsStorage)

#set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
#OVERWRITE the CXX-Flags for this project
set(CMAKE_CXX_FLAGS -O3)

set(SOURCE_FILES main.cpp TempStatistics.cpp StatisticsStorage.cpp StatisticsStorage.h)
add_executable(StatisticsStorage ${SOURCE_FILES})
