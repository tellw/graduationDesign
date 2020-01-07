#ifndef COMMON_HPP_INCLUDED
#define COMMON_HPP_INCLUDED

#include <opencv2/opencv.hpp>

inline void slice_horizontal(const cv::Mat& stereo_view, cv::Mat& left, cv::Mat& right){
    const int cols = stereo_view.cols;
    stereo_view(cv::Range::all(), cv::Range(0, cols/2)).copyTo(left);
    stereo_view(cv::Range::all(), cv::Range(cols/2, cols)).copyTo(right);
}

inline double area(const cv::Rect2i& rect){
    return rect.height * rect.width;
}

inline cv::Mat crop(const cv::Mat& source, const cv::Rect2i& crop_rect){
    return source(cv::Range(crop_rect.y, crop_rect.y+crop_rect.height), cv::Range(crop_rect.x, crop_rect.x+crop_rect.width));
}

#endif // COMMON_HPP_INCLUDED
