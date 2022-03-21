#include <cstdlib>
#include <iostream>
#include "gpucalc.cpp"
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

int main() {
    clock_t start, end;
    out("Hello, World! Starting OpenCL Application")
    separate
    out(">>> ls")
    system("ls");
    separate
    inspect_context();
    Image image_0, image_1;
    load(image_0, "im0")
    load(image_1, "im1")
    separate
    out("Reduced image size")
    start = clock();
    downSample(&image_0, SCALE_FACTOR);
    downSample(&image_1, SCALE_FACTOR);
    end = clock();
    std::cout << " - took: " << (double) (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    out("a: " << image_0.str)
    out("b: " << image_1.str)
    separate
    start = clock();
    double wts = wt();
    //Image disp_0 = znccHorizontal_RPLUS(&image_1, &image_0, WINDOW_SIZE, MAX_DISPARITY);
    //Image disp_1 = znccHorizontal_RMINUS(&image_0, &image_1, WINDOW_SIZE, MAX_DISPARITY);

    //auto data = invokeThreaded(4, &image_1, &image_0, WINDOW_SIZE, MAX_DISPARITY);
    auto data = doZNCC(&image_1, &image_0, WINDOW_SIZE, MAX_DISPARITY);
    Image disp_0 = std::get<0>(data);
    Image disp_1 = std::get<1>(data);

    end = clock();
    double wte = wt();
    std::cout << " - took: " << (double) (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    std::cout << " - wall: " << wte - wts << "s" << std::endl;
    save(disp_0, "im1-im0-disp")
    save(disp_1, "im0-im1-disp")
    separate
    out("Cross check")
    start = clock();
    crossCheck(&disp_0, &disp_1, CS_THRESHOLD);
    end = clock();
    std::cout << " - took: " << (double) (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    save(disp_0, "cs-map")
    separate
    out("Oculus")
    start = clock();
    oculus(&disp_0, &disp_1);
    end = clock();
    std::cout << " - took: " << (double) (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    save(disp_1, "oculus-map")
    separate
    out("Done")
    return 0;
}
