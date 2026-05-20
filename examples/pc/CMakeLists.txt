# PC example

# Set mininum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define project
project(PCExample VERSION 0.1.0 LANGUAGES C)

# General settings (language, output dir)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the source directory. This should call cmake script inside this directory.
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}/btreefy_lib_bin_dir
)

add_executable(BTreeFy-PCExample
    btreefy-examples.c
    btf_nodes_generated.c
    door_operator_controller.c
)

target_include_directories(BTreeFy-PCExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(BTreeFy-PCExample PRIVATE BTreeFy-Src -pthread)
