#pragma once

#include <opencv2/core.hpp>

/**
 * @brief Performs contrast limited adaptive histogram equalization.
 *
 * Applies histogram equalization to a grayscale image.
 * Depending on the radius, the operation can be global or local.
 *
 * @param in Input grayscale image.
 * @param s Contrast limitation factor. Values below 1 disable the limitation.
 * @param radius Radius of the local cells. A value of 0 enables global equalization.
 *
 * @return Equalized image with the same dimensions and type as the input.
 */
cv::Mat fsiv_clahe(const cv::Mat &in, float s, int radius);