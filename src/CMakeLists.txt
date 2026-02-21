
if(ZEPHYR_BASE)
    zephyr_library_named(BTreeFy-Src)
    zephyr_library_sources(btreefy.c btreefy_policies.c)
    zephyr_library_sources(runners/btf_tree_runner_zephyr.c)
    zephyr_library_link_libraries(zephyr_interface)
else()
    add_library(BTreeFy-Src STATIC btreefy.c btreefy_policies.c)
endif()

target_include_directories(BTreeFy-Src PUBLIC "${PROJECT_SOURCE_DIR}/include")