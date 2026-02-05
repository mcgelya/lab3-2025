cmake_minimum_required(VERSION 3.10)

project(lab3_2025)

set(CMAKE_CXX_STANDARD 23)

find_package(Catch2 3 REQUIRED)
include(CTest)
enable_testing()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
endif()

add_subdirectory(src)
add_subdirectory(tests)
