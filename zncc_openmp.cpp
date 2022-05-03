#include "zncc_common.cpp"

void initialize() {}

std::pair<Image, Image> zncc(Image* left, Image* right, int window, int max_disparity) {
    int width = (int) left->width;
    int height = (int) left->height;
    auto disparityMapLTR = std::vector<double>(width * height, 0);
    auto disparityMapRTL = std::vector<double>(width * height, 0);
    std::cout << "ZNCC Starting\n" << " - window: " << window << "\n - max d: " << max_disparity << std::endl;
#pragma omp parallel for shared(disparityMapLTR, disparityMapRTL) firstprivate(left, right, window, width, height, max_disparity) default(none)
    for (int idx = 0; idx < width * height; idx++) {
        int x = idx % width;
        int y = idx / width;
        auto value = znccBothWays(x, y, &left->data, &right->data, window, width, max_disparity);
        disparityMapLTR[idx] = value.first;
        disparityMapRTL[idx] = value.second;
    }
    std::cout << "Done: 100%" << std::endl;
    auto data1 = handleDisparity(disparityMapLTR, width, height);
    auto data2 = handleDisparity(disparityMapRTL, width, height);
    return {data1, data2};
}