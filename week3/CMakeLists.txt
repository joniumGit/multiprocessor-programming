cmake_minimum_required(VERSION 3.18)
project(week3)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
add_compile_definitions(CL_HPP_TARGET_OPENCL_VERSION=300)

add_library(
        lodepng SHARED
        include/lodepng.cpp
        include/lodepng.h
)
target_include_directories(lodepng PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR})

add_executable(week3 main.cpp)
target_include_directories(week3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(week3 ${OpenCL_LIBRARY})
target_link_libraries(week3 lodepng)
