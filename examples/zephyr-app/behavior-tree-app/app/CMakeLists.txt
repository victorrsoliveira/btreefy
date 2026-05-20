# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# 1. Tell Zephyr where your out-of-tree module is
set(ZEPHYR_EXTRA_MODULES /home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app LANGUAGES C)

target_sources(app
               PRIVATE
               src/main.c
               src/door_operator_controller.c
               src/btf_nodes_generated.c)

target_link_libraries(app PUBLIC BTreeFy-Src)