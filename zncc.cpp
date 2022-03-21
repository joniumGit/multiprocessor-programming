#pragma once

#include "image.cpp"
#include "pthread.h"

#define WINDOW(loop_var) for(int loop_var = -window; loop_var < window + 1; loop_var++)
#define LEFT(loop_var) (*left)[(y * width) + x + loop_var]
#define RIGHT(loop_var, offset) (*right)[(y * width) + x + loop_var offset d]
#define CHECKS(offset) if (x offset d + window >= width) continue; if(x offset d - window < 0) continue;
#define disparity std::vector<double>

inline int for_x_y_minus(
        int x,
        int y,
        const pixels* left,
        const pixels* right,
        int window,
        int width,
        int disparity_max
) {
    double topZncc = 0;
    int bestDisparity = 0;
    for (int d = 0; d < disparity_max; d++) {
        CHECKS(-)

        // Avg
        double avgL = 0;
        double avgR = 0;
        WINDOW(j) {
            avgL += LEFT(j) / (double) window;
            avgR += RIGHT(j, -) / (double) window;
        }

        // ZNCC
        double top_sum = 0;
        double bot_sum_l = 0;
        double bot_sum_r = 0;

        WINDOW(j) {
            // Get values
            double valL = LEFT(j) - avgL;
            double valR = RIGHT(j, -) - avgR;
            // Do sum
            top_sum += valL * valR;
            bot_sum_l += valL * valL;
            bot_sum_r += valR * valR;
        }

        // SQRT
        bot_sum_l = std::sqrt(bot_sum_l);
        bot_sum_r = std::sqrt(bot_sum_r);

        // ZNCC
        double zncc = top_sum / (bot_sum_l * bot_sum_r);
        if (zncc > topZncc) {
            topZncc = zncc;
            bestDisparity = d;
        }
    }
    return bestDisparity;
}

inline int for_x_y_plus(
        int x,
        int y,
        const pixels* left,
        const pixels* right,
        int window,
        int width,
        int disparity_max
) {
    double topZncc = 0;
    int bestDisparity = 0;
    for (int d = 0; d < disparity_max; d++) {
        CHECKS(+)

        // Avg
        double avgL = 0;
        double avgR = 0;
        WINDOW(j) {
            avgL += LEFT(j) / (double) window;
            avgR += RIGHT(j, +) / (double) window;
        }

        // ZNCC
        double top_sum = 0;
        double bot_sum_l = 0;
        double bot_sum_r = 0;

        WINDOW(j) {
            // Get values
            double valL = LEFT(j) - avgL;
            double valR = RIGHT(j, +) - avgR;
            // Do sum
            top_sum += valL * valR;
            bot_sum_l += valL * valL;
            bot_sum_r += valR * valR;
        }

        // SQRT
        bot_sum_l = std::sqrt(bot_sum_l);
        bot_sum_r = std::sqrt(bot_sum_r);

        // ZNCC
        double zncc = top_sum / (bot_sum_l * bot_sum_r);
        if (zncc > topZncc) {
            topZncc = zncc;
            bestDisparity = d;
        }
    }
    return bestDisparity;
}


Image znccHorizontal_RMINUS(Image* left, Image* right, int window, int max_disparity) {
    int width = (int) left->width;
    int height = (int) left->height;
    auto disparityMap = std::vector<double>(width * height, 0);
    int idx = 0;
    std::cout << "ZNCC (RM) Starting\n" << " - window: " << window << "\n - max d: " << max_disparity << std::endl;
    double lineCount = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            disparityMap[idx] = for_x_y_minus(x, y, &left->data, &right->data, window, width, max_disparity);
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

Image znccHorizontal_RPLUS(Image* left, Image* right, int window, int max_disparity) {
    int width = (int) left->width;
    int height = (int) left->height;
    auto disparityMap = std::vector<double>(width * height, 0);
    int idx = 0;
    std::cout << "ZNCC (RP) Starting\n" << " - window: " << window << "\n - max d: " << max_disparity << std::endl;
    double lineCount = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            disparityMap[idx] = for_x_y_plus(x, y, &left->data, &right->data, window, width, max_disparity);
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
    for (int i = 0; i < disparityMap.size(); i++) {
        dataMap[i] = (unsigned char) std::round((disparityMap[i] - min) * (255 - 0) / (max - min) + 0);
    }

    Image image = Image();
    image.width = width;
    image.height = height;
    image.setData(&dataMap);
    return image;
}

inline void znccHorizontalThread(
        const pixels* left,
        const pixels* right,
        disparity* outLTR,
        disparity* outRTL,
        int window,
        int max_disparity,
        int width,
        int from,
        int to
) {
    for (int y = from; y < to; y++) {
        for (int x = 0; x < width; x++) {
            (*outLTR)[(y * width) + x] = for_x_y_plus(x, y, left, right, window, width, max_disparity);
            (*outRTL)[(y * width) + x] = for_x_y_minus(x, y, right, left, window, width, max_disparity);
        }
    }
}


struct ZNCC_ARGS {
    const pixels* left;
    const pixels* right;
    disparity* outLTR;
    disparity* outRTL;
    const int* window;
    const int* max_disparity;
    const int* width;
    int from;
    int to;
};

inline void* znccHorizontalThreadResolve(void* o) {
    try {
        auto* args = (struct ZNCC_ARGS*) (o);
        const pixels* left = args->left;
        const pixels* right = args->right;
        disparity* outLTR = args->outLTR;
        disparity* outRTL = args->outRTL;
        int window = *args->window;
        int max_disparity = *args->max_disparity;
        int width = *args->width;
        int from = args->from;
        int to = args->to;
        znccHorizontalThread(left, right, outLTR, outRTL, window, max_disparity, width, from, to);
        free(args);
        std::cout << "Done (" << from << "-" << to << ")" << std::endl;
    } catch (const std::exception &e) {
        std::cout << "Exec: " << e.what() << std::endl;
        throw e;
    }
}


std::tuple<Image, Image> invokeThreaded(unsigned short n, Image* left, Image* right, int window, int max_disparity) {
    int width = (int) left->width;
    int height = (int) left->height;
    unsigned short step = (unsigned short) left->height / n;

    auto disparityMap1 = disparity(width * height, 0);
    auto disparityMap2 = disparity(width * height, 0);

    auto threads = new pthread_t[n]();

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < n - 1; i++) {
        auto* args = new ZNCC_ARGS();
        args->left = &left->data;
        args->right = &right->data;
        args->outLTR = &disparityMap1;
        args->outRTL = &disparityMap2;
        args->window = &window;
        args->max_disparity = &max_disparity;
        args->width = &width;
        args->from = i * step;
        args->to = i * step + step;
        std::cout << "Starting (" << args->from << "-" << args->to << ")" << std::endl;
        pthread_create(&threads[i], &attr, znccHorizontalThreadResolve, (void*) args);
    }

    auto* args = new ZNCC_ARGS();
    args->left = &left->data;
    args->right = &right->data;
    args->outLTR = &disparityMap1;
    args->outRTL = &disparityMap2;
    args->window = &window;
    args->max_disparity = &max_disparity;
    args->width = &width;
    args->from = (n - 1) * step;
    args->to = height;
    std::cout << "Starting Last (" << args->from << "-" << args->to << ")" << std::endl;
    pthread_create(&threads[n - 1], &attr, znccHorizontalThreadResolve, (void*) args);

    pthread_attr_destroy(&attr);

    int code;
    bool fail = false;
    for (int i = 0; i < n; i++) {
        code = pthread_join(threads[i], nullptr);
        if (code) {
            std::cout << "Failed joining thread: " << code << std::endl;
            fail = true;
        }
    }
    free(threads);
    if (fail) exit(-1);


    auto data1 = handleDisparity(disparityMap1, width, height);
    auto data2 = handleDisparity(disparityMap2, width, height);

    return {data1, data2};
}