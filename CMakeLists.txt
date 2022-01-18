cmake_minimum_required(VERSION 3.18)
project(multi_programming)

set(CMAKE_CXX_STANDARD 14)

#
# ----- START -----
#

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
add_executable(multi_programming main.cpp)
target_include_directories(multi_programming PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(multi_programming ${OpenCL_LIBRARY})
