#include <cstdlib>
#include <iostream>
#include "gpucalc.cpp"
#include "image.cpp"

#define base "../images/"
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
    lodepng::encode(std::string(base "output/" file "-gray.png"), image.data, image.width, image.height, LCT_GREY);       \
    out("Saved " base "output/" file "-gray.png")
#define save(image, file) \
    separate                                                                                                           \
    lodepng::encode(std::string(base "output/" file ".png"), image.data, image.width, image.height, LCT_GREY);            \
    out("Saved " base "output/" file ".png")

int main() {
    out("Hello, World! Starting OpenCL Application")
    separate
    out(">>> ls")
    system("ls");
    separate
    inspect_context();
    Image image_a, image_b;
    load(image_a, "im0")
    load(image_b, "im1")
    //doWork();
    save(image_a, "im0-final")
    save(image_b, "im1-final")
    separate
    out("Done")
    return 0;
}
