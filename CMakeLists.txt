# BTreeFy top-level build script

# Set mininum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define project
project(BTreeFy VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# General settings (language, output dir)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BTREEFY_BUILD_TESTS "Build BTreeFy unit tests" OFF)

# Add the source directory. This should call cmake script inside this directory.
add_subdirectory(src)

if(BTREEFY_BUILD_TESTS)
    include(CTest)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
      unity
      GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
      GIT_TAG        v2.6.0
    )
    FetchContent_MakeAvailable(unity)

    add_subdirectory(tests)
endif()