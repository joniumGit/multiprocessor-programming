cmake_minimum_required(VERSION 3.18)
project(main C)
add_executable(main main.c addcl.c)

# OpenCL
find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(main ${OpenCL_LIBRARY})
