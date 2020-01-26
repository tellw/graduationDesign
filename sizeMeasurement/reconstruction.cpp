#include "reconstruction.hpp"
#include <iostream>

cv::Vec3f reproject(const cv::Vec2i& pos, int16_t disparity, const cv::Mat& q) {
	cv::Mat p(4, 1, CV_64F);
	p.at<double>(0) = pos[0];
	p.at<double>(1) = pos[1];
	p.at<double>(2) = 0 == disparity ? 10000.0 : disparity;
	p.at<double>(3) = 1.0;
	cv::Mat res = q * p;
	float w = res.at<double>(3);
	return cv::Vec3f(res.at<double>(0), res.at<double>(1), res.at<double>(2)) / w;
}

cloud_ptr reproject(const cv::Mat& disparity, const cv::Mat& q, bool bmm, cv::Vec2f zclamp) {
	cloud_ptr cloud(new cloud_type(disparity.cols, disparity.rows));
	cv::Mat image3d;
	cv::reprojectImageTo3D(disparity, image3d, q, true, -1);
	if (!bmm) {
		zclamp[0] /= 1000;
		zclamp[1] /= 1000;
	}
	for (auto col = 0; col < disparity.cols; ++col) {
		for (auto row = 0; row < disparity.rows; ++row) {
			const cv::Point3f& point = image3d.at<cv::Point3f>(row, col);
			if (point.z < zclamp[0] || zclamp[1] < point.z)continue;
			pcl::PointXYZ pcl_point(point.x, point.y, point.z);
			cloud->at(col, row) = pcl_point;
		}
	}
	return cloud;
}