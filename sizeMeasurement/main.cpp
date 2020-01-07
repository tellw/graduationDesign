#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>

#include "calib.hpp"
#include "disparity.hpp"
#include "reconstruction.hpp"
#include "app.hpp"

#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/features/normal_3d.h>
#include <pcl/surface/gp3.h>

#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/cloud_viewer.h>

const float camera_height = 466.1;
const std::string xmlfile = "parametersOfCamera/selfMakeParam1910262022.xml"

double _distance(const pcl::PointXYZ& p1, const pcl::PointXYZ& p2){
	return sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)+(p1.z-p2.z)*(p1.z-p2.z));
}

cv::Mat convertTo3Channels(const cv::Mat& binImg){
	cv::Mat three_channel = cv::Mat::zeros(binImg.rows, binImg.cols, CV_8UC3);
	std::vector<cv::Mat> channels;
	for(int i = 0; i < 3; i++)channels.push_back(binImg);
	cv::merge(channels, three_channel);
	return three_channels;
}

void detect(stereo_calibrator& calibrator, cv::Mat& left, cv::Mat& right, bool bVisualized=false, bool bCollected=false, bool bTimeRecorded=false){
	auto rectified_left = calibrator.rectify_left(left);
}