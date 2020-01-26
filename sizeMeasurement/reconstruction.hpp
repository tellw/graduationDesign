#ifndef RECONSTRUCTION_HPP_INCLUDED
#define RECONSTRUCTION_HPP_INCLUDED

#include "common.hpp"
#include <pcl/common/common_headers.h>

typedef pcl::PointCloud<pcl::PointXYZ> cloud_type;
typedef cloud_type::Ptr cloud_ptr;

cv::Vec3f reproject(const cv::Vec2i& pos, int16_t disparity, const cv::Mat& q);
cloud_ptr reproject(const cv::Mat& disparity, const cv::Mat& q, bool bmm = true, cv::Vec2f zclamp = cv::Vec2f(100., 1000.));

#endif // RECONSTRUCTION_HPP_INCLUDED
