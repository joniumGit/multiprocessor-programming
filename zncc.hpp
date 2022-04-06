#pragma once

#include "image.cpp"

void initialize();

std::pair<Image, Image> zncc(Image* left, Image* right, int window, int max_disparity);

#ifdef ZNCC_OPENCL

const char* driver = "OpenCL";

#include "zncc_opencl.cpp"

#else

#ifdef ZNCC_OPENMP

const char* driver = "OpenMP";

#include "zncc_openmp.cpp"

#else

#ifdef ZNCC_PTHREAD

const char* driver = "PThread";

#include "zncc_pthread.cpp"

#else

const char* driver = "Plain";

#include "zncc_plain.cpp"

#endif
#endif
#endif
