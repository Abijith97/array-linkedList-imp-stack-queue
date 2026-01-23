cmake_minimum_required(VERSION 3.10)
project(ArrayStackQueue C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/array-stack.c
)

# Create a library from source files
add_library(array_stack ${SOURCES})

# Target include directories for the library
target_include_directories(array_stack PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Optional: Create a simple demo executable
add_executable(demo src/main.c)
target_link_libraries(demo array_stack)
