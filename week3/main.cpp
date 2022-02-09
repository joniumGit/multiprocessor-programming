#include <iostream>
#include "lodepng.h"

#define pixels std::vector<unsigned char>

class Image {
public:
    unsigned width, height;
    std::string str;
    pixels data;

    explicit Image(Image* image, pixels* new_data) {
        this->width = image->width;
        this->height = image->height;
        this->data = *new_data;
        this->str = image->str;
    }

    explicit Image(const char* name) {
        this->width = 0;
        this->height = 0;
        this->data = pixels();
        unsigned error = lodepng::decode(data, width, height, name);
        if (error) {
            throw std::runtime_error(lodepng_error_text(error));
        }
        this->str = std::string("Image: (")
                    + std::to_string(width)
                    + std::string(",")
                    + std::to_string(this->height)
                    + std::string(")");
    }
};

Image toGrayScale(Image* image) {
    pixels data = pixels(image->width * image->height, 0);
    for (unsigned i = 0; i < image->width * image->height; i++) {
        for (unsigned k = 0; k < 3; k++) {
            data[i] += image->data[i * 4 + k] / 3;
        }
    }
    return Image(image, &data);
}

int main() {
    Image image = Image("../im0.png");
    std::cout << "Hello, World!" << std::endl;
    std::cout << image.str << std::endl;
    std::cout << image.data.size() / image.width / image.height << std::endl;
    std::cout << "Making the image gray" << std::endl;
    Image gray = toGrayScale(&image);
    std::cout << gray.str << std::endl;
    std::cout << gray.data.size() / image.width / image.height << std::endl;
    std::cout << "Saving" << std::endl;
    lodepng::encode(std::string("../im0-gray.png"), gray.data, gray.width, gray.height, LCT_GREY);
    return 0;
}
