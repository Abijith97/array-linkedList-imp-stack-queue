cmake_minimum_required(VERSION 3.14)
project(ArrayStackQueue C CXX)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set C++ standard for tests
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/array-stack.c
)

set(LL_SOURCES
    src/linkedlist-stack.c
)

# Create a library from source files
add_library(array_stack ${SOURCES})
add_library(linkedlist_stack ${LL_SOURCES})

# Target include directories for the library
target_include_directories(array_stack PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(linkedlist_stack PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Optional: Create a simple demo executable
add_executable(demo src/main.c)
target_link_libraries(demo array_stack linkedlist_stack)

# Google Test setup
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add test executable
add_executable(
  array_stack_test
  tests/test_array_stack.cpp
)

target_link_libraries(
  array_stack_test
  array_stack
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(array_stack_test)
