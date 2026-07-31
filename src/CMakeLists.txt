
set(BTREEFY_SOURCES btreefy.c btreefy_policies.c btreefy_always_actions.c)

if(ZEPHYR_BASE)
    zephyr_library_named(BTreeFy-Src)
    zephyr_library_sources(${BTREEFY_SOURCES})
    zephyr_library_link_libraries(zephyr_interface)
else()
    add_library(BTreeFy-Src STATIC ${BTREEFY_SOURCES})
endif()

target_include_directories(BTreeFy-Src PUBLIC "${PROJECT_SOURCE_DIR}/include")