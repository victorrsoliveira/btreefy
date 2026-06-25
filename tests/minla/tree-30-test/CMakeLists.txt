# MinLA test

# Set mininum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define project
project(MinLATestProject VERSION 0.1.0 LANGUAGES C)

# General settings (language, output dir)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the source directory. This should call cmake script inside this directory.
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_BINARY_DIR}/btreefy_lib_bin_dir
)

add_executable(MinLATest
    test_bt_minla.c
    btf_nodes_generated.c
)

target_include_directories(MinLATest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(MinLATest PRIVATE BTreeFy-Src)
