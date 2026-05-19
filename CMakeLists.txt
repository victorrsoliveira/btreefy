# BTreeFy top-level build script

# Set mininum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define project
project(BTreeFy VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# General settings (language, output dir)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

if(NOT ZEPHYR_BASE)
    set(BTREEFY_RUNNER "POSIX" CACHE STRING "BTreeFy Runner Selection")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the source directory. This should call cmake script inside this directory.
add_subdirectory(src)