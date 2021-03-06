#pragma once

#include "image.cpp"

void crossCheck(Image* a, Image* b, unsigned threshold) {
#ifdef ZNCC_OPENMP
#pragma omp parallel for shared(a, b) firstprivate(threshold) default(none)
#endif
    for (int i = 0; i < a->data.size(); i++) {
        if (abs(a->data[i] - b->data[i]) > threshold) {
            a->data[i] = 0;
            b->data[i] = 0;
        }
    }
}

void oculus(Image* a, Image* b) {
    int height = (int) a->height;
    int width = (int) a->width;
#ifdef ZNCC_OPENMP
#pragma omp parallel for shared(b) firstprivate(a, width, height) default(none)
#endif
    for (int idx = 0; idx < width * height; idx++) {
        int y = idx / width;
        int x = idx % width;
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

inline Image handleDisparity(std::vector<double> disparityMap, int width, int height) {
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
#ifdef ZNCC_OPENMP
#pragma omp parallel for shared(dataMap) firstprivate(disparityMap, max, min) default(none)
#endif
    for (int i = 0; i < disparityMap.size(); i++) {
        dataMap[i] = (unsigned char) std::round((disparityMap[i] - min) * (255 - 0) / (max - min) + 0);
    }

    Image image = Image();
    image.width = width;
    image.height = height;
    image.setData(&dataMap);
    return image;
}