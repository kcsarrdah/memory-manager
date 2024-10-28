cmake_minimum_required(VERSION 3.29)  # Keep your version
project(memory_manager)

set(CMAKE_CXX_STANDARD 17)  # Using 17 for better features

# Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Your library
add_library(memory_manager src/memory_pool.cpp)
target_include_directories(memory_manager PUBLIC include)

# Tests
enable_testing()
add_executable(memory_manager_tests tests/memory_pool_test.cpp)
target_link_libraries(memory_manager_tests
        memory_manager
        GTest::gtest_main
)