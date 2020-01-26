#include "disparity.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>

cv::Mat compute_disparity(const cv::Mat& left, const cv::Mat& right, float CLAHECoeff) {
	cv::Mat clahed_left, clahed_right;
	auto clahe = cv::createCLAHE(CLAHECoeff);
	clahe->apply(left, clahed_left);
	clahe->apply(right, clahed_right);
	cv::Mat filtered_left, filtered_right;
	float laplacian[] = { 0.f, -1.f, 0.f, -1.f, 5.f, -1.f, 0.f, -1.f, 0.f };
	auto kernel = cv::Mat(3, 3, CV_32F, laplacian);
	cv::filter2D(clahed_left, filtered_left, -1, kernel);
	cv::filter2D(clahed_right, filtered_right, -1, kernel);
	const int win_size = 3;
	auto matcher_left = cv::StereoSGBM::create(
		0, 16 * 7, 2, 8 * 2 * win_size*win_size, 32 * 10 * win_size*win_size,
		8, 31, 1, 8, 8, cv::StereoSGBM::MODE_SGBM_3WAY
	);
	cv::Mat displ, true_displ;
	matcher_left->compute(filtered_left, filtered_right, displ);
	displ.convertTo(true_displ, CV_16S, 1.0 / 16);
	auto matcher_right = cv::ximgproc::createRightMatcher(matcher_left);
	cv::Mat dispr, true_dispr;
	matcher_right->compute(filtered_left, filtered_right, dispr);
	dispr.convertTo(true_dispr, CV_16S, 1.0 / 16);
	const int lambda = 3000;
	const double sigma = 10;
	auto filter = cv::ximgproc::createDisparityWLSFilter(matcher_left);
	filter->setLambda(lambda);
	filter->setSigmaColor(sigma);
	cv::Mat filtered_displ;
	filter->filter(true_displ, filtered_left, filtered_displ, true_dispr);
	return filtered_displ;
}

cv::Mat visualize_disparity(const cv::Mat& disp) {
	double min, max;
	cv::minMaxIdx(disp, &min, &max);
	float scale = 256.f / (max - min);
	cv::Mat scaled_map;
	disp.convertTo(scaled_map, CV_8UC1, scale, -min * scale);
	return scaled_map;
}