#include <iostream>
#include "cl_support.cpp"


int main() {
    Image image = Image("../im0.png");
    std::cout << "Hello, World! Starting OpenCL Application" << std::endl;
    separate();
    std::cout << "Loaded an Image" << std::endl;
    std::cout << " - Image:        " << image.str << std::endl;
    std::cout << " - Values/Pixel: " << image.data.size() / image.width / image.height << std::endl;
    separate();
    std::cout << "Making the image gray..." << std::endl;
    Image gray = toGrayScale(&image);
    std::cout << "Done" << std::endl;
    std::cout << " - Image:        " << gray.str << std::endl;
    std::cout << " - Values/Pixel: " << gray.data.size() / image.width / image.height << std::endl;
    lodepng::encode(std::string("../im0-gray.png"), gray.data, gray.width, gray.height, LCT_GREY);
    std::cout << "Saved" << std::endl;
    create();
    separate();
    std::cout << "Filtering" << std::endl;
    auto filtered = fiveByFive(&gray);
    lodepng::encode(std::string("../im0-filtered.png"), filtered.data, filtered.width, filtered.height, LCT_GREY);
    std::cout << "Done" << std::endl;
    return 0;
}
