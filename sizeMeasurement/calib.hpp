#ifndef CALIB_HPP_INCLUDED
#define CALIB_HPP_INCLUDED

#include "common.hpp"

struct stereo_calibrator {
    enum init_mode {STATIC_PARAM, CALIBRATION, MATLAB_CALIB};
    cv::Mat left_intrinsic, right_intrinsic, left_coeffs, right_coeffs, transform, rotation, r1, r2, p1, p2, q;
    stereo_calibrator() = default;
    stereo_calibrator(const std::string& conf_file, init_mode mode = STATIC_PARAM);
    bool chessboard_calibrate(const std::string& conf_file);
    bool matlab_calibrate(const std::string& conf_file);
    cv::Mat rectify_left(const cv::Mat& left_img)const{
        return rectify_left(left_img, _size(left_img));
    }
    cv::Mat rectify_left(const cv::Mat& left_img, cv::Size2i new_size) const;
    cv::Mat rectify_right(const cv::Mat& right_img)const{
	    return rectify_right(right_img, _size(right_img));
    }
    cv::Mat rectify_right(const cv::Mat& right_img, cv::Size2i new_size) const;
    bool save_params(const std::string& path) const;
    bool load_params(const std::string& path);
    bool load_params_matlab(const std::string& path);

	private:
    static const uint32_t min_good_images = 1;
    static const cv::Size2i subpix_window_size;
    bool _calibrate(const std::vector<cv::Mat>& lefts, const std::vector<cv::Mat>& rights, cv::Size2i image_size, cv::Size2i pattern, cv::Size2f grid_size, cv::Size2i win_size);
    static cv::Size2i _size(const cv::Mat& img){
	    return cv::Size2i(img.cols, img.rows);
    }
    static cv::Rect2i _whole_img(const cv::Mat& img){
	    return cv::Rect2i(cv::Point2i(0, 0), _size(img));
    }
}

#endif // CALIB_HPP_INCLUDED
