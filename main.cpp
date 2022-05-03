#define ZNCC_OPENCL

#include <iostream>
#include "image.cpp"
#include "common.cpp"
#include "zncc.hpp"
#include "sys/time.h"


double wt() {
    struct timeval time;
    if (gettimeofday(&time, nullptr)) {
        return 0;
    }
    return (double) time.tv_sec + (double) time.tv_usec * .000001;
}

#define CS_THRESHOLD 16
#define SCALE_FACTOR 4
#define MAX_DISPARITY 100
#define WINDOW_SIZE   12


#define base "images/"
#define out(message) std::cout << message << std::endl;
#define separate for (int i = 0; i < 80; i++) std::cout << std::string("-"); std::cout << std::endl;
#define load(image, file) \
    separate                                                                                                           \
    out(base file ".png")                                                                                              \
    image.load(base file ".png");                                                                                      \
    out("Loaded an Image")                                                                                             \
    out(" - Image:        " << image.str)                                                                              \
    out(" - Values/Pixel: " << image.data.size() / image.width / image.height)                                         \
    separate                                                                                                           \
    out("Making the image gray...")                                                                                    \
    toGrayScale(&image);                                                                                               \
    out("Done")                                                                                                        \
    out(" - Image:        " << image.str)                                                                              \
    out(" - Values/Pixel: " << image.data.size() / image.width / image.height)                                         \
    lodepng::encode(std::string(base "output/" file "-gray.png"), image.data, image.width, image.height, LCT_GREY);    \
    out("Saved " base "output/" file "-gray.png")
#define save(image, file) \
    separate                                                                                                           \
    lodepng::encode(std::string(base "output/" file ".png"), image.data, image.width, image.height, LCT_GREY);         \
    out("Saved " base "output/" file ".png")

#define START_TIME wt_start = wt(); start = clock();
#define END_TIME(wt_store) wt_end = wt(); end = clock(); wt_store = wt_end - wt_start; \
    std::cout << " - took: " << (double) (end - start) / CLOCKS_PER_SEC << "s" << std::endl; \
    std::cout << " - wall: " << wt_store << "s" << std::endl;

int main() {
    clock_t start, end;
    double wt_start, wt_end, reduce_step, zncc_step, cs_step, occlusion_step;
    out("Hello, World! Starting OpenCL Application")
    separate
    out(">>> ls")
    system("ls");
    separate
    initialize();
    Image image_0, image_1;
    load(image_0, "im0")
    load(image_1, "im1")
    separate
    out("Reduced image size")
    START_TIME
    downSample(&image_0, SCALE_FACTOR);
    downSample(&image_1, SCALE_FACTOR);
    END_TIME(reduce_step)
    out("a: " << image_0.str)
    out("b: " << image_1.str)
    separate
    START_TIME
    // Actual Run
    auto data = zncc(&image_1, &image_0, WINDOW_SIZE, MAX_DISPARITY);
    Image disp_0 = data.first;
    Image disp_1 = data.second;
    END_TIME(zncc_step)
    save(disp_0, "im1-im0-disp")
    save(disp_1, "im0-im1-disp")
    separate
    out("Cross check")
    START_TIME
    crossCheck(&disp_0, &disp_1, CS_THRESHOLD);
    END_TIME(cs_step)
    save(disp_0, "cs-map")
    separate
    out("Oculus")
    START_TIME
    oculus(&disp_0, &disp_1);
    END_TIME(occlusion_step)
    save(disp_1, "oculus-map")
    separate
    out("Done")
    separate
    std::cout << "RESULTS    " << driver << std::endl;
    std::cout << "REDUCE:    " << reduce_step << "s" << std::endl;
    std::cout << "ZNCC:      " << zncc_step << "s" << std::endl;
    std::cout << "CS:        " << cs_step << "s" << std::endl;
    std::cout << "OCCLUSION: " << occlusion_step << "s" << std::endl;
    return 0;
}
