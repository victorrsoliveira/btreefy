
option(BTREEFY_DEBUG_PRINTF "Enable debug printing" OFF)

set(BTREEFY_SOURCES btreefy.c btreefy_policies.c btreefy_always_actions.c)

if(ZEPHYR_BASE)
    zephyr_library_named(BTreeFy-Src)
    zephyr_library_sources(${BTREEFY_SOURCES})
    zephyr_library_link_libraries(zephyr_interface)
    if(BTREEFY_DEBUG_PRINTF)
        zephyr_library_compile_definitions(CONFIG_BTREEFY_DEBUG_PRINTF)
    endif()
else()
    add_library(BTreeFy-Src STATIC ${BTREEFY_SOURCES})
    if(BTREEFY_DEBUG_PRINTF)
        target_compile_definitions(BTreeFy-Src PUBLIC CONFIG_BTREEFY_DEBUG_PRINTF)
    endif()
endif()

target_include_directories(BTreeFy-Src PUBLIC "${PROJECT_SOURCE_DIR}/include")