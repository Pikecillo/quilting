#pragma once

#include <cv.h>

cv::Mat luminance_image(cv::Mat image);
cv::Mat blurred_luminance_image(cv::Mat image, int window_size);
void average_filter(cv::Mat image, int window_size);
void median_filter(cv::Mat image, int window_size);
CvScalar window_average(cv::Mat image, int i, int j, int window_size);
CvScalar window_median(cv::Mat image, int i, int j, int window_size);
