#include "calib.h"
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include "common.h"
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

const cv::Size2i stereo_calibrator::subpix_window_size(4, 4);
static const std::string IMAGE_SIZE = "image_size";
static const std::string PATTERN = "pattern";
static const std::string GRID_SIZE = "grid_size";
static const std::string PATH = "path";
static const std::string LEFT_INTRINSIC = "left_intrinsic";
static const std::string LEFT_COEFFS = "left_coeffs";
static const std::string RIGHT_INTRINSIC = "right_intrinsic";
static const std::string RIGHT_COEFFS = "right_coeffs";
static const std::string ROTATION = "rotation", TRANSFORM = "transform";
static const std::string R1 = "r1", R2 = "r2", P1 = "p1", P2 = "p2", Q = "q";

static std::string trim(const std::string& str) {
	auto low = str.find_first_not_of(' ');
	auto high = str.find_last_not_of(' ');
	if (std::string::npos == low || std::string::npos == high)return std::string();
	return str.substr(low, high - low + 1);
}

static std::vector<std::string> split(const std::string& str, const char delim) {
	std::vector<std::string> res;
	std::stringstream stream(str);
	std::string tmp;
	while (std::getline(stream, tmp, delim)) {
		tmp = trim(tmp);
		if (tmp.empty())continue;
		res.push_back(tmp);
	}
	return res;
}

stereo_calibrator::stereo_calibrator(const std::string& conf_file, init_mode mode) {
	switch (mode) {
	case stereo_calibrator::STATIC_PARAM: {
		if (!load_params(conf_file)) {
			throw std::exception("Failed to open the parameter file.");
		}
		break;
	}
	case stereo_calibrator::CALIBRATION: {
		if (!chessboard_calibrate(conf_file))throw std::exception("Error occurred on calibration.");
		break;
	}
	case stereo_calibrator::MATLAB_CALIB: {
		if (!matlab_calibrate(conf_file))throw std::exception("Error occurred on calibration.");
		break;
	}
	default:
		throw std::exception("Mode not supported.");
		break;
	}
}

bool stereo_calibrator::load_params(const std::string& path) {
	cv::FileStorage storage(path, cv::FileStorage::READ);
	if (!storage.isOpened())return false;
	storage[LEFT_INTRINSIC] >> left_intrinsic;
	storage[LEFT_COEFFS] >> left_coeffs;
	storage[RIGHT_INTRINSIC] >> right_intrinsic;
	storage[RIGHT_COEFFS] >> right_coeffs;
	storage[R1] >> r1;
	storage[P1] >> p1;
	storage[R2] >> r2;
	storage[P2] >> p2;
	storage[Q] >> q;
	storage.release();
	return true;
}

bool stereo_calibrator::chessboard_calibrate(const std::string& conf_file) {
	cv::FileStorage storage(conf_file, cv::FileStorage::READ);
	if (!storage.isOpened())return false;
	cv::Size2i image_size, pattern;
	cv::Size2f grid_size;
	std::string path;
	storage[IMAGE_SIZE] >> image_size;
	storage[PATTERN] >> pattern;
	storage[GRID_SIZE] >> grid_size;
	storage[PATH] >> path;
	storage.release();
	std::vector<cv::Mat> lefts, rights;
	auto files = split(path, ';');
	for (auto& file : files) {
		cv::Mat origin = cv::imread(file, cv::IMREAD_GRAYSCALE);
		assert(origin.rows == image_size.height && origin.cols == image_size.width * 2);
		lefts.emplace_back();
		rights.emplace_back();
		slice_horizontal(origin, lefts.back(), rights.back());
	}
	return _calibrate(lefts, rights, image_size, pattern, grid_size, subpix_window_size);
}

static std::vector<cv::Point3f> _cal_corner_positions(cv::Size2i pattern, cv::Size2f grid_size) {
	std::vector<cv::Point3f> result;
	for (int row = 0; row < pattern.height; ++row) {
		for (int col = 0; col < pattern.width; ++col) {
			result.emplace_back(col*grid_size.width, row*grid_size.height, 0.0f);
		}
	}
	return result;
}

bool stereo_calibrator::_calibrate(const std::vector<cv::Mat>& lefts, const std::vector<cv::Mat>& rights, cv::Size2i image_size, cv::Size2i pattern, cv::Size2f grid_size, cv::Size2i win_size) {
	assert(!lefts.empty());
	assert(lefts.size() == rights.size());
	uint32_t total_frames = lefts.size();
	uint32_t count_good = 0;
	std::vector<std::vector<cv::Point2f>> left_corners, right_corners;
	std::vector<cv::Point2f> left_corners_buff, right_corners_buff;
	for (auto i = 0u; i < total_frames; ++i) {
		if (!cv::findChessboardCorners(lefts[i], pattern, left_corners_buff) || !cv::findChessboardCorners(rights[i], pattern, right_corners_buff)) {
			std::cerr << "Bad frames" << i << std::endl;
			continue;
		}
		cv::find4QuadCornerSubpix(lefts[i], left_corners_buff, win_size);
		cv::find4QuadCornerSubpix(rights[i], right_corners_buff, win_size);
		left_corners.emplace_back(std::move(left_corners_buff));
		right_corners.emplace_back(std::move(right_corners_buff));
		++count_good;
	}
	if (count_good < min_good_images) return false;
	const auto corner_positions = _cal_corner_positions(pattern, grid_size);
	std::vector<std::vector<cv::Point3f>> corner_positions_vec(count_good, corner_positions);
	std::vector<cv::Mat> rvecs, tvecs;
	std::cout << "Left calibration error: " << cv::calibrateCamera(corner_positions_vec, left_corners, image_size, left_intrinsic, left_coeffs, rvecs, tvecs) << std::endl;
	std::cout << "Right calibration error: " << cv::calibrateCamera(corner_positions_vec, right_corners, image_size, right_intrinsic, right_coeffs, rvecs, tvecs) << std::endl;
	cv::Mat essential, fundamental;
	auto rms = cv::stereoCalibrate(corner_positions_vec, left_corners, right_corners, left_intrinsic, left_coeffs, right_intrinsic, right_coeffs, image_size, rotation, transform, essential, fundamental, cv::CALIB_USE_INTRINSIC_GUESS);
	std::cout << "Stereo calibration error: " << rms << std::endl;
	cv::stereoRectify(left_intrinsic, left_coeffs, right_intrinsic, right_coeffs, image_size, rotation, transform, r1, r2, p1, p2, q);
	return true;
}

bool stereo_calibrator::matlab_calibrate(const std::string& conf_file) {
	if (!load_params_matlab(conf_file))return false;
	int img_width, img_height;
	std::cout << "标定时设置的单张图片的宽度：";
	std::cin >> img_width;
	std::cout << std::endl;
	std::cout << "标定时设置的单张图片的高度：";
	std::cin >> img_height;
	std::cout << std::endl;
	cv::Size2i image_size(img_width, img_height);
	cv::stereoRectify(left_intrinsic, left_coeffs, right_intrinsic, right_coeffs, image_size, rotation, transform, r1, r2, p1, p2, q);
	return true;
}

bool stereo_calibrator::load_params_matlab(const std::string& path) {
	cv::FileStorage storage(path, cv::FileStorage::READ);
	if (!storage.isOpened())return false;
	storage[LEFT_INTRINSIC] >> left_intrinsic;
	storage[LEFT_COEFFS] >> left_coeffs;
	storage[RIGHT_INTRINSIC] >> right_intrinsic;
	storage[RIGHT_COEFFS] >> right_coeffs;
	storage[ROTATION] >> rotation;
	storage[TRANSFORM] >> transform;
	storage.release();
	return true;
}

cv::Mat stereo_calibrator::rectify_left(const cv::Mat& left_img, cv::Size2i new_size)const {
	cv::Mat mapx, mapy;
	cv::initUndistortRectifyMap(left_intrinsic, left_coeffs, r1, p1, new_size, CV_8U, mapx, mapy);
	cv::Mat rectified;
	cv::remap(left_img, rectified, mapx, mapy, cv::INTER_LINEAR);
	return rectified;
}

cv::Mat stereo_calibrator::rectify_right(const cv::Mat& right_img, cv::Size2i new_size)const {
	cv::Mat mapx, mapy;
	cv::initUndistortRectifyMap(right_intrinsic, right_coeffs, r2, p2, new_size, CV_8U, mapx, mapy);
	cv::Mat rectified;
	cv::remap(right_img, rectified, mapx, mapy, cv::INTER_LINEAR);
	return rectified;
}