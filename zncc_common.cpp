#pragma once

#include "common.cpp"

inline std::pair<int, int> znccBothWays(
        int x,
        int y,
        const pixels* left,
        const pixels* right,
        int window,
        int width,
        int disparity_max
) {
    int index = (y * width) + x;

    int plusBestDisparity = 0;
    int minusBestDisparity = 0;

    double plusTopZncc = 0;
    double minusTopZncc = 0;

    for (int d = 0; d < disparity_max; d++) {

        bool doPlus = true;
        bool doMinus = true;

        if (x + d + window >= width) doPlus = false;
        if (x + d - window < 0) doPlus = false;
        if (x - d + window >= width) doMinus = false;
        if (x - d - window < 0) doMinus = false;

        if (doPlus && doMinus) {

            double plusLeftAverage = 0;
            double plusRightAverage = 0;

            double minusLeftAverage = 0;
            double minusRightAverage = 0;

            for (int j = -window; j < window + 1; j++) {

                plusLeftAverage += (*left)[index + j] / (double) window;
                plusRightAverage += (*right)[index + j + d] / (double) window;

                minusLeftAverage += (*right)[index + j] / (double) window;
                minusRightAverage += (*left)[index + j - d] / (double) window;

            }

            double plusTopSum = 0;
            double plusBottomSumLeft = 0;
            double plusBottomSumRight = 0;

            double minusTopSum = 0;
            double minusBottomSumLeft = 0;
            double minusBottomSumRight = 0;

            for (int j = -window; j < window + 1; j++) {

                double plusValueLeft = (*left)[index + j] - plusLeftAverage;
                double plusValueRight = (*right)[index + j + d] - plusRightAverage;

                plusTopSum += plusValueLeft * plusValueRight;
                plusBottomSumLeft += plusValueLeft * plusValueLeft;
                plusBottomSumRight += plusValueRight * plusValueRight;

                double minusValueLeft = (*right)[index + j] - minusLeftAverage;
                double minusValueRight = (*left)[index + j - d] - minusRightAverage;

                minusTopSum += minusValueLeft * minusValueRight;
                minusBottomSumLeft += minusValueLeft * minusValueLeft;
                minusBottomSumRight += minusValueRight * minusValueRight;

            }

            plusBottomSumLeft = sqrt((double) plusBottomSumLeft);
            plusBottomSumRight = sqrt((double) plusBottomSumRight);

            minusBottomSumLeft = sqrt((double) minusBottomSumLeft);
            minusBottomSumRight = sqrt((double) minusBottomSumRight);

            double plusZncc = plusTopSum / (plusBottomSumLeft * plusBottomSumRight);
            if (plusZncc > plusTopZncc) {

                plusTopZncc = plusZncc;
                plusBestDisparity = d;

            }

            double minusZncc = minusTopSum / (minusBottomSumLeft * minusBottomSumRight);
            if (minusZncc > minusTopZncc) {

                minusTopZncc = minusZncc;
                minusBestDisparity = d;

            }

        } else if (doPlus) {

            double plusLeftAverage = 0;
            double plusRightAverage = 0;

            for (int j = -window; j < window + 1; j++) {

                plusLeftAverage += (*left)[index + j] / (double) window;
                plusRightAverage += (*right)[index + j + d] / (double) window;

            }

            double plusTopSum = 0;
            double plusBottomSumLeft = 0;
            double plusBottomSumRight = 0;

            for (int j = -window; j < window + 1; j++) {

                double plusValueLeft = (*left)[index + j] - plusLeftAverage;
                double plusValueRight = (*right)[index + j + d] - plusRightAverage;

                plusTopSum += plusValueLeft * plusValueRight;
                plusBottomSumLeft += plusValueLeft * plusValueLeft;
                plusBottomSumRight += plusValueRight * plusValueRight;
            }

            plusBottomSumLeft = sqrt((double) plusBottomSumLeft);
            plusBottomSumRight = sqrt((double) plusBottomSumRight);

            double plusZncc = plusTopSum / (plusBottomSumLeft * plusBottomSumRight);
            if (plusZncc > plusTopZncc) {

                plusTopZncc = plusZncc;
                plusBestDisparity = d;

            }

        } else if (doMinus) {

            double minusLeftAverage = 0;
            double minusRightAverage = 0;

            for (int j = -window; j < window + 1; j++) {

                minusLeftAverage += (*right)[index + j] / (double) window;
                minusRightAverage += (*left)[index + j - d] / (double) window;

            }

            double minusTopSum = 0;
            double minusBottomSumLeft = 0;
            double minusBottomSumRight = 0;

            for (int j = -window; j < window + 1; j++) {

                double minusValueLeft = (*right)[index + j] - minusLeftAverage;
                double minusValueRight = (*left)[index + j - d] - minusRightAverage;

                minusTopSum += minusValueLeft * minusValueRight;
                minusBottomSumLeft += minusValueLeft * minusValueLeft;
                minusBottomSumRight += minusValueRight * minusValueRight;

            }

            minusBottomSumLeft = sqrt((double) minusBottomSumLeft);
            minusBottomSumRight = sqrt((double) minusBottomSumRight);

            double minusZncc = minusTopSum / (minusBottomSumLeft * minusBottomSumRight);
            if (minusZncc > minusTopZncc) {

                minusTopZncc = minusZncc;
                minusBestDisparity = d;

            }

        } else {
            continue;
        }

    }

    return std::make_pair(plusBestDisparity, minusBestDisparity);
}