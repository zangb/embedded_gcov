project("testlib" VERSION 0.0.1 LANGUAGES CXX C)

add_library(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_options(${PROJECT_NAME} PRIVATE "-fprofile-arcs" "-ftest-coverage" "-fcondition-coverage")
target_link_options(${PROJECT_NAME} PRIVATE "-fprofile-arcs" "-ftest-coverage" "-fcondition-coverage")
