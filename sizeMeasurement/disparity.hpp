#ifndef DISPARITY_HPP_INCLUDED
#define DISPARITY_HPP_INCLUDED

#include "common.hpp"

cv::Mat compute_disparity(const cv::Mat& left, const cv::Mat& right, float CLAHECoeff = 2.0);
cv::Mat visualize_disparity(const cv::Mat& disp);

#endif // DISPARITY_HPP_INCLUDED
