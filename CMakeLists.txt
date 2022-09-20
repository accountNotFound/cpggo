cmake_minimum_required(VERSION 3.0.0)
project(cppgo VERSION 0.1.0)

set(CMAKE_CXX_COMPILE "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fcoroutines")
set(CMAKE_BUILD_TYPE "debug")

include(CTest)
enable_testing()

add_subdirectory("src/context")
add_subdirectory("src/synchronize")
add_subdirectory("test")

add_test("coroutine-nested-call" coroutine_test)
add_test("spinlock-benchmark" spinlock_test)
add_test("monitor-benchmark" context_test)
add_test("channel-benchmark" channel_test)
add_test("multi-channel-benchmark" multi_channel_test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
