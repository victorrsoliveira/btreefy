# tests/CMakeLists.txt

# Create an executable for the core tests
add_executable(test_btreefy_core test_btreefy_core.c btf_nodes_generated.c)

# Link against the library under test and the Unity framework
# Note: BTreeFy-Src is the library target defined in src/CMakeLists.txt
target_link_libraries(test_btreefy_core PRIVATE BTreeFy-Src unity)

# Register the test with CTest
add_test(NAME btreefy_core_tests COMMAND test_btreefy_core)
