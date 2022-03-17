#pragma once

#include <iostream>
#include <cmath>
#include "lodepng.h"

#define pixels std::vector<unsigned char>

class Image {
public:
    unsigned width, height;
    std::string str;
    pixels data;

    explicit Image() {
        this->width = 0;
        this->height = 0;
        this->data = pixels();
        this->str = "empty image";
    }

    void load(const char* name) {
        unsigned error = lodepng::decode(data, width, height, name, LCT_RGBA);
        if (error) {
            throw std::runtime_error(lodepng_error_text(error));
        }
        makeStr();
    }

    void makeStr() {
        this->str = std::string("Image: (")
                    + std::to_string(width)
                    + std::string(",")
                    + std::to_string(height)
                    + std::string(")");
    }

    void setData(pixels* newData) {
        data = *newData;
        makeStr();
    }

};

void toGrayScale(Image* image) {
    /*
     * Make image grayscale by averaging color pixels
     */
    pixels data = pixels(image->width * image->height, 0);
    int idx = 0;
    for (int i = 0; i < image->width * image->height * 4; i += 4) {
        double avg = 0;
        for (int k = 0; k < 3; k++) {
            avg += image->data[i + k] / 3.0;
        }
        data[idx] = (unsigned char) std::round(avg);
        idx++;
    }
    image->setData(&data);
}

void downSample(Image* image, int factor) {
    /*
     * Reduce image size by factor by averaging a grayscale image
     */
    pixels data = pixels(image->width * image->height / (factor * factor), 0);
    int idx = 0;
    double area = factor * factor;
    for (int y = 0; y < image->height; y += factor) {
        for (int x = 0; x < image->width; x += factor) {
            double avg = 0;
            for (int i = 0; i < factor; i++) {
                for (int j = 0; j < factor; j++) {
                    avg += image->data[((y + i) * image->width) + x + j] / area;
                }
            }
            data[idx] = (unsigned char) std::round(avg);
            idx++;
        }
    }
    image->width = image->width / factor;
    image->height = image->height / factor;
    image->setData(&data);
}
