set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.26)
project("coverage" VERSION 0.0.1 LANGUAGES CXX C)

add_subdirectory(libgcov)
add_subdirectory(libtest)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC
    testlib libgcov)
target_compile_options(${PROJECT_NAME} PUBLIC "-std=c++17")
