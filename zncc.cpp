#pragma once

#include "image.cpp"

#define WINDOW(loop_var) for(int loop_var = -window / 2; loop_var < window / 2; loop_var++)
#define LEFT(loop_var) left->data[(y * width) + x + loop_var]
#define RIGHT(loop_var, offset) right->data[(y * width) + x + loop_var offset d]
#define CHECKS(offset) if (x offset d + window / 2 >= width or x offset d - window / 2 < 0) continue;

inline int for_x_y_minus(
        unsigned x,
        unsigned y,
        Image* left,
        Image* right,
        int window,
        int width,
        int disparity_max
) {
    double topZncc = 0;
    int bestDisparity = 0;
    int d = 0;
    for (; d < disparity_max; d++) {
        CHECKS(-)

        // Avg
        double avgL = 0;
        double avgR = 0;
        WINDOW(j) {
            avgL += LEFT(j) / window;
            avgR += RIGHT(j, -) / window;
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
        unsigned x,
        unsigned y,
        Image* left,
        Image* right,
        int window,
        int width,
        int disparity_max
) {
    double topZncc = 0;
    int bestDisparity = 0;
    int d = 0;
    for (; d < disparity_max; d++) {
        CHECKS(+)

        // Avg
        double avgL = 0;
        double avgR = 0;
        WINDOW(j) {
            avgL += LEFT(j) / window;
            avgR += RIGHT(j, +) / window;
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

