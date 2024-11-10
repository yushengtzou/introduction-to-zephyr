cmake_minimum_required(VERSION 3.22.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_SOURCE_DIR}/../../modules/say_hello")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_project)

target_sources(app PRIVATE src/main.c)