
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

Mat generateSinusoidalPattern(int width, int height, double phaseShift) {
    Mat pattern(height, width, CV_64F);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pattern.at<double>(y, x) = 0.5 * (1 + sin(2 * CV_PI * x / 10.0 + phaseShift));
        }
    }
    return pattern;
}

Mat calculatePhaseMap(const vector<Mat>& patterns) {
    Mat phaseMap(patterns[0].size(), CV_64F);
    for (int y = 0; y < patterns[0].rows; ++y) {
        for (int x = 0; x < patterns[0].cols; ++x) {
            double I1 = patterns[0].at<double>(y, x);
            double I2 = patterns[1].at<double>(y, x);
            double I3 = patterns[2].at<double>(y, x);
            phaseMap.at<double>(y, x) = atan2(sqrt(3) * (I1 - I3), 2 * I2 - I1 - I3);
        }
    }
    return phaseMap;
}

Mat averagePhaseMaps(const Mat& phaseMap1, const Mat& phaseMap2) {
    Mat averagedPhaseMap = (phaseMap1 + phaseMap2) / 2.0;
    for (int y = 0; y < averagedPhaseMap.rows; ++y) {
        for (int x = 0; x < averagedPhaseMap.cols; ++x) {
            if (abs(phaseMap1.at<double>(y, x) - phaseMap2.at<double>(y, x)) > CV_PI) {
                averagedPhaseMap.at<double>(y, x) += CV_PI;
            }
        }
    }
    return averagedPhaseMap;
}

Mat decodeGrayCodePatterns(const vector<Mat>& grayCodePatterns) {
    Mat fringeOrder(grayCodePatterns[0].size(), CV_64F);
    for (int y = 0; y < grayCodePatterns[0].rows; ++y) {
        for (int x = 0; x < grayCodePatterns[0].cols; ++x) {
            int order = 0;
            for (size_t i = 0; i < grayCodePatterns.size(); ++i) {
                order = (order << 1) | (grayCodePatterns[i].at<uchar>(y, x) > 127 ? 1 : 0);
            }
            fringeOrder.at<double>(y, x) = order;
        }
    }
    return fringeOrder;
}

Mat unwrapPhaseMap(const Mat& wrappedPhaseMap, const Mat& fringeOrder) {
    Mat unwrappedPhaseMap(wrappedPhaseMap.size(), CV_64F);
    for (int y = 0; y < wrappedPhaseMap.rows; ++y) {
        for (int x = 0; x < wrappedPhaseMap.cols; ++x) {
            unwrappedPhaseMap.at<double>(y, x) = wrappedPhaseMap.at<double>(y, x) + 2 * CV_PI * fringeOrder.at<double>(y, x);
        }
    }
    return unwrappedPhaseMap;
}

void displayImage(const Mat& img, const string& windowName) {
    Mat display;
    normalize(img, display, 0, 1, NORM_MINMAX);
    imshow(windowName, display);
    waitKey(0);
}

int main() {
    int width = 1280;
    int height = 720;

    vector<Mat> patterns1 = {
        generateSinusoidalPattern(width, height, 0),
        generateSinusoidalPattern(width, height, 2 * CV_PI / 3),
        generateSinusoidalPattern(width, height, 4 * CV_PI / 3)
    };

    vector<Mat> patterns2 = {
        generateSinusoidalPattern(width, height, CV_PI / 3),
        generateSinusoidalPattern(width, height, CV_PI),
        generateSinusoidalPattern(width, height, 5 * CV_PI / 3)
    };

    Mat phaseMap1 = calculatePhaseMap(patterns1);
    Mat phaseMap2 = calculatePhaseMap(patterns2);

    Mat averagedPhaseMap = averagePhaseMaps(phaseMap1, phaseMap2);
    displayImage(averagedPhaseMap, "Averaged Phase Map");

    vector<Mat> grayCodePatterns(7);
    for (int i = 0; i < 7; ++i) {
        grayCodePatterns[i] = imread("gray_pattern_" + to_string(i) + ".png", IMREAD_GRAYSCALE);
    }

    Mat fringeOrder = decodeGrayCodePatterns(grayCodePatterns);
    Mat unwrappedPhaseMap = unwrapPhaseMap(averagedPhaseMap, fringeOrder);
    displayImage(unwrappedPhaseMap, "Unwrapped Phase Map");

    // Plot a single row
    int rowToPlot = height / 2;
    Mat rowPlot(1, width, CV_64F);
    for (int x = 0; x < width; ++x) {
        rowPlot.at<double>(0, x) = unwrappedPhaseMap.at<double>(rowToPlot, x);
    }
    displayImage(rowPlot, "Single Row Plot");

    return 0;
}
