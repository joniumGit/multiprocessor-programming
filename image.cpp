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

Image znccHorizontal(Image* left, Image* right, int window, int max_disparity) {
    int width = (int) left->width;
    int height = (int) left->height;
    auto disparityMap = std::vector<double>(width * height, 0);
    int idx = 0;
    std::cout << "ZNCC Starting\n" << " - window: " << window << "\n - max d: " << max_disparity << std::endl;
    double lineCount = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double topZncc = 0;
            int bestDisparity = 0;
            for (int d = 0; d < max_disparity; d++) {
                if (x - d - window / 2 < 0) continue;
                // Avg
                double avgL = 0;
                double avgR = 0;
                double count = 0;

                for (int j = -window / 2; j < window / 2; j++) {
                    if (x + j >= width or x + j < 0) continue;
                    avgL += left->data[(y * width) + x + j];
                    avgR += right->data[(y * width) + x - d + j];
                    count++;
                }

                avgL /= count;
                avgR /= count;
                // ZNCC
                double top_sum = 0;
                double bot_sum_l = 0;
                double bot_sum_r = 0;
                double zncc;

                for (int j = -window / 2; j < window / 2; j++) {
                    if (x + j >= width or x + j < 0) continue;
                    // Get values
                    double valL = left->data[(y * width) + x + j] - avgL;
                    double valR = right->data[(y * width) + x - d + j] - avgR;
                    // Do sum
                    top_sum += valL * valR;
                    bot_sum_l += valL * valL;
                    bot_sum_r += valR * valR;
                }

                // SQRT
                bot_sum_l = std::sqrt(bot_sum_l);
                bot_sum_r = std::sqrt(bot_sum_r);
                // ZNCC
                zncc = top_sum / (bot_sum_l * bot_sum_r);
                // Compare
                if (zncc > topZncc) {
                    topZncc = zncc;
                    bestDisparity = d;
                }
            }
            disparityMap[idx] = bestDisparity;
            idx++;
        }
        lineCount++;
        std::cout << "Done: " << (int) std::round(lineCount / height * 100) << "%\r";
    }
    std::cout << "Done: 100%" << std::endl;
    // Min-Max
    double min = disparityMap[0];
    double max = disparityMap[0];
    for (auto i: disparityMap) {
        if (i < min) {
            min = i;
        }
        if (i > max) {
            max = i;
        }
    }
    // Normalize
    pixels dataMap = pixels(width * height, 0);
    for (int i = 0; i < disparityMap.size(); i++) {
        dataMap[i] = (unsigned char) std::round((disparityMap[i] - min) * (255 - 0) / (max - min) + 0);
    }
    // New Image
    Image data = Image();
    data.width = width;
    data.height = height;
    data.setData(&dataMap);
    return data;
}

Image zncc(Image* left, Image* right, int window, int max_disparity) {
    int width = (int) left->width;
    int height = (int) left->height;
    auto disparityMap = std::vector<double>(width * height, 0);
    int idx = 0;
    std::cout << "ZNCC Starting\n" << " - window: " << window << "\n - max d: " << max_disparity << std::endl;
    double lineCount = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double topZncc = 0;
            int bestDisparity = 0;
            for (int d = 0; d < max_disparity; d++) {
                if (y - d - window / 2 < 0) continue;
                if (x - d - window / 2 < 0) continue;
                // Avg
                double avgL = 0;
                double avgR = 0;
                double count = 0;
                for (int dx = 0; dx < 2; dx++) {
                    for (int dy = 0; dx < 2; dx++) {
                        if (dx == dy == 0) continue;
                        for (int i = -window / 2; i < window / 2; i++) {
                            if (y + i >= height or y + i < 0) continue;
                            for (int j = -window / 2; j < window / 2; j++) {
                                if (x + j >= width or x + j < 0) continue;
                                avgL += left->data[((y + i) * width) + x - d + j];
                                avgR += right->data[((y + i - d * dy) * width) + x - d * dx + j];
                                count++;
                            }
                        }
                    }
                }
                avgL /= count;
                avgR /= count;
                // ZNCC
                double top_sum = 0;
                double bot_sum_l = 0;
                double bot_sum_r = 0;
                double zncc;
                for (int dx = 0; dx < 2; dx++) {
                    for (int dy = 0; dx < 2; dx++) {
                        if (dx == dy == 0) continue;
                        for (int i = -window / 2; i < window / 2; i++) {
                            if (y + i >= height or y + i < 0) continue;
                            for (int j = -window / 2; j < window / 2; j++) {
                                if (x + j >= width or x + j < 0) continue;
                                // Get values
                                double valL = left->data[((y + i) * width) + x - d + j] - avgL;
                                double valR = right->data[((y + i - d * dy) * width) + x - d * dx + j] - avgR;
                                // Do sum
                                top_sum += valL * valR;
                                bot_sum_l += valL * valL;
                                bot_sum_r += valR * valR;
                            }
                        };
                    }
                }
                // SQRT
                bot_sum_l = std::sqrt(bot_sum_l);
                bot_sum_r = std::sqrt(bot_sum_r);
                // ZNCC
                zncc = top_sum / (bot_sum_l * bot_sum_r);
                // Compare
                if (zncc > topZncc) {
                    topZncc = zncc;
                    bestDisparity = d;
                }
            }
            disparityMap[idx] = bestDisparity;
            idx++;
        }
        lineCount++;
        std::cout << "Done: " << (int) std::round(lineCount / height * 100) << "%\r";
    }
    std::cout << "Done: 100%" << std::endl;
    // Min-Max
    double min = disparityMap[0];
    double max = disparityMap[0];
    for (auto i: disparityMap) {
        if (i < min) {
            min = i;
        }
        if (i > max) {
            max = i;
        }
    }
    // Normalize
    pixels dataMap = pixels(width * height, 0);
    for (int i = 0; i < disparityMap.size(); i++) {
        dataMap[i] = (unsigned char) std::round((disparityMap[i] - min) * (255 - 0) / (max - min) + 0);
    }
    // New Image
    Image data = Image();
    data.width = width;
    data.height = height;
    data.setData(&dataMap);
    return data;
}

void crossCheck(Image* a, Image* b, unsigned threshold) {
    for (int i = 0; i < a->data.size(); i++) {
        if (abs(a->data[i] - b->data[i]) > threshold) {
            a->data[i] = 0;
            b->data[i] = 0;
        }
    }
}

void oculus(Image* a, Image* b) {
    unsigned height = a->height;
    unsigned width = a->width;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (a->data[y * width + x] == 0) {
                double bdata = 0;
                unsigned vcnt = 0;
                label:
                if (vcnt == 0) {
                    for (int cnt = 1; cnt < width * height; cnt++) {
                        for (int xi = -cnt; xi + x >= 0 and xi + x < width; xi += cnt) {
                            for (int yi = -cnt; yi + y >= 0 and yi + y < height; yi += cnt) {
                                if (a->data[(yi + y) * width + (xi + x)] != 0) {
                                    bdata += a->data[(yi + y) * width + (xi + x)] / 8.0;
                                    vcnt++;
                                    if (vcnt == 8) goto label;
                                }
                            }
                        }
                    }
                }
                b->data[y * width + x] = (unsigned char) std::round(bdata);
            }
        }
    }
}