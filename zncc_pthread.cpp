#include "zncc_common.cpp"

typedef std::vector<double> disparity;

void initialize() {}

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


std::pair<Image, Image> zncc(Image* left, Image* right, int window, int max_disparity) {
    int n = 4;
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