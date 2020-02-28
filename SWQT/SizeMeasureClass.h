#pragma once
#include <string>
#include <opencv2/core.hpp>
#include <pcl/point_types.h>
#include "calib.h"
#include "CMyINI.h"
#include "SizeMeasureOptionWidget.h"
#include <pcl/common/projection_matrix.h>

typedef pcl::PointCloud<pcl::PointXYZ> cloud_type;
typedef cloud_type::Ptr cloud_ptr;

class SizeMeasureClass {
public:
	const float camera_height = 466.1;
	std::string xmlfile;
	double cut_rate = 0.05;
	bool bVisualized = true;
	bool bCollected = true;
	bool bTimeRecorded = false;
	bool bmm = true;
	bool bAuto = true;
	bool bPrecise = false;
	stereo_calibrator* calibrator;
	CMyINI* cMyIni;
	int detectCount;
	SizeMeasureOptionWidget* sizeMeasureOptionWidget;
	double _distance(const pcl::PointXYZ& p1, const pcl::PointXYZ& p2) {
		return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y) + (p1.z - p2.z)*(p1.z - p2.z));
	}

	cv::Mat convertTo3Channels(const cv::Mat& binImg) {
		cv::Mat three_channel = cv::Mat::zeros(binImg.rows, binImg.cols, CV_8UC3);
		std::vector<cv::Mat> channels;
		for (int i = 0; i < 3; i++)channels.push_back(binImg);
		cv::merge(channels, three_channel);
		return three_channel;
	}
	SizeMeasureClass(CMyINI* cMyIni, SizeMeasureOptionWidget* sizeMeasureOptionWidget);
	void detect(cv::Mat& left, cv::Mat& right, cv::Mat& left_src, cv::Mat& right_src);
	/*void setCMyIni(CMyINI* cMyIni);*/
	void setBVisualized(bool bVisualized);
	void setBCollected(bool bVisualized);
	void setBTimeRecorded(bool bTimeRecorded);
	void setBmm(bool bmm);
	void setBAuto(bool bAuto);
	/*void setBPrecise(bool bPrecise);*/
};