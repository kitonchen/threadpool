cmake_minimum_required(VERSION 3.7)
project(threadpool)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ThreadPool.cpp ThreadPool.h)

link_libraries(-lpthread)
add_executable(threadpool ${SOURCE_FILES})