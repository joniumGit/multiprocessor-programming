#pragma once

#include "image.cpp"

void initialize();

std::pair<Image, Image> zncc(Image* left, Image* right, int window, int max_disparity);

#ifdef ZNCC_OPENCL

#include "zncc_opencl.cpp"

#else

#ifdef ZNCC_OPENMP

#include "zncc_openmp.cpp"

#else

#ifdef ZNCC_PTHREAD

#include "zncc_pthread.cpp"

#else

#include "zncc_plain.cpp"

#endif
#endif
#endif
