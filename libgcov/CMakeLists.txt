cmake_minimum_required(VERSION 3.26)
project(libgcov VERSION 0.0.1 LANGUAGES C)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/gcov.c)

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_options(${PROJECT_NAME} PRIVATE "-std=c23")
