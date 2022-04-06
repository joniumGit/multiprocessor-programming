cmake_minimum_required(VERSION 3.18)
project(multiprocessor_programming CXX)
set(CMAKE_CXX_STANDARD 17)
# Executable
add_executable(multiprocessor_programming main.cpp)
# Add Lodepng
add_library(
        lodepng SHARED
        include/lodepng.cpp
        include/lodepng.h
)
target_include_directories(lodepng PUBLIC ${PROJECT_SOURCE_DIR}/lodepng ${PROJECT_SOURCE_DIR})
target_link_libraries(multiprocessor_programming PUBLIC lodepng)
# Copy Images
add_custom_target(
        images
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/images ${CMAKE_CURRENT_BINARY_DIR}/images
)
add_dependencies(multiprocessor_programming images)
# Threads
find_package(Threads)
if (Threads_FOUND)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries(multiprocessor_programming PUBLIC Threads::Threads)
endif ()
# OpenCL
find_package(OpenCL)
if (OpenCL_FOUND)
    include_directories(${OpenCL_INCLUDE_DIRS})
    link_directories(${OpenCL_LIBRARY})
    add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
    add_compile_definitions(CL_HPP_TARGET_OPENCL_VERSION=300)
    target_link_libraries(multiprocessor_programming PUBLIC ${OpenCL_LIBRARY})
endif ()
# OpenMP
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    target_link_libraries(multiprocessor_programming PUBLIC OpenMP::OpenMP_CXX)
endif ()