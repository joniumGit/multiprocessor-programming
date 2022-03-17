cmake_minimum_required(VERSION 3.18)
project(week4)

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

add_executable(week4 main.cpp zncc.cpp)
target_include_directories(week4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(week4 ${OpenCL_LIBRARY})
target_link_libraries(week4 lodepng)

add_custom_target(
        images
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/images ${CMAKE_CURRENT_BINARY_DIR}/images
)
add_dependencies(week4 images)