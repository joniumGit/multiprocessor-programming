#include "zncc_common.cpp"

void initialize() {}

std::pair<Image, Image> zncc(Image* left, Image* right, int window, int max_disparity) {
    int width = (int) left->width;
    int height = (int) left->height;
    auto disparityMapLTR = std::vector<double>(width * height, 0);
    auto disparityMapRTL = std::vector<double>(width * height, 0);
    int idx = 0;
    std::cout << "ZNCC Starting\n" << " - window: " << window << "\n - max d: " << max_disparity << std::endl;
    double lineCount = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            auto value = znccBothWays(x, y, &left->data, &right->data, window, width, max_disparity);
            disparityMapLTR[idx] = value.first;
            disparityMapRTL[idx] = value.second;
            idx++;
        }
        lineCount++;
        std::cout << "Done: " << (int) std::round(lineCount / height * 100) << "%\r";
    }
    std::cout << "Done: 100%" << std::endl;
    auto data1 = handleDisparity(disparityMapLTR, width, height);
    auto data2 = handleDisparity(disparityMapRTL, width, height);
    return {data1, data2};
}