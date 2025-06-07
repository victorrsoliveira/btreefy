add_library(BTreeFy-Src STATIC btreefy.c btreefy_policies.c)

target_include_directories(BTreeFy-Src PUBLIC "${PROJECT_SOURCE_DIR}/include")