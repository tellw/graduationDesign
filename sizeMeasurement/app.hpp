#ifndef APP_HPP_INCLUDED
#define APP_HPP_INCLUDED

#include "common.hpp"
#include "reconstruction.hpp"

void sample_plane_point_cloud(const cv::Mat& disp, const cv::Mat& q, cloud_ptr& cloud, std::vector<cv::Vec3f>& set);
cv::Vec3f estimate_reference_plane(const std::vector<cv::Vec3f>& set);
bool reference_plane_bounding_box(const cv::Mat& source, cv::Rect2i& bbox, std::vector<cv::Point2i>& poly);
void _find_polys(const cv::Mat& image, std::vector<std::vector<cv::Point2i>>& polys);
void find_polys(const cv::Mat& image, std::vector<std::vector<cv::Point2i>>& polys);

#endif // APP_HPP_INCLUDED
