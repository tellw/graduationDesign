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
const std::string xmlfile = "parametersOfCamera/selfMakeParam1910262022.xml";
double cut_rate = 0.05;
bool bVisualized = true;
bool bCollected = true;
bool bTimeRecorded = false;
bool bmm = true;

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

void detect(stereo_calibrator& calibrator, cv::Mat& left, cv::Mat& right, int detectCount, bool bVisualized = false, bool bCollected = false, bool bTimeRecorded = false, bool bmm = true) {
	time_t startTime;
	if (bTimeRecorded)startTime = time(NULL);
	auto rectified_left = calibrator.rectify_left(left);
	auto rectified_right = calibrator.rectify_right(right);
	if (bVisualized) {
		cv::imshow("rectified_l", rectified_left);
		cv::imshow("rectified_r", rectified_right);
		cv::waitKey();
	}
	auto left_roi = rectified_left(cv::Rect2i(cut_rate*left.cols, cut_rate*left.rows, (1.0 - 2 * cut_rate)*left.cols, (1.0 - 2 * cut_rate)*left.rows));
	auto right_roi = rectified_right(cv::Rect2i(cut_rate*right.cols, cut_rate*right.rows, (1.0 - 2 * cut_rate)*right.cols, (1.0 - 2 * cut_rate)*right.rows));
	if (bVisualized) {
		cv::imshow("rectified_l", left_roi);
		cv::imshow("rectified_r", right_roi);
		cv::waitKey();
	}
	std::vector<cv::Point2i> pts;
	std::vector<std::vector<cv::Point2i>> polys;
	_find_polys(left_roi, polys, bVisualized);
	auto disp = compute_disparity(left_roi, right_roi);
	cv::imshow("disp", visualize_disparity(disp));
	auto cloud = reproject(disp, calibrator.q, bmm);
	cv::Mat resShow = convertTo3Channels(rectified_left);
	for (auto& poly : polys) {
		auto& pt0 = cloud->at(poly[0].x, poly[0].y);
		auto& pt1 = cloud->at(poly[1].x, poly[1].y);
		auto& pt2 = cloud->at(poly[2].x, poly[2].y);
		auto& pt3 = cloud->at(poly[3].x, poly[3].y);
		if (!bTimeRecorded) {
			std::cout << poly[0] << "\'s world coordinate is " << pt0 << std::endl;
			std::cout << poly[1] << "\'s world coordinate is " << pt1 << std::endl;
			std::cout << poly[2] << "\'s world coordinate is " << pt2 << std::endl;
			std::cout << poly[3] << "\'s world coordinate is " << pt3 << std::endl;
		}
		double a = (_distance(pt0, pt1) + _distance(pt2, pt3)) / 2.0;
		double b = (_distance(pt1, pt2) + _distance(pt0, pt3)) / 2.0;
		std::vector<float> zs({ pt0.z, pt1.z,, pt2.z, pt3.z });
		float zmin = camera_height;
		for (auto z : zs) {
			if (z > 0) {
				if (z < zmin)zmin = z;
			}
		}
		float c = camera_height - zmin;
		if (!bmm) {
			a *= 1000;
			b *= 1000;
			c *= 1000;
		}
		std::ostringstream astr, bstr, cstr, Vstr, timestr;
		astr << std::setiosflags(std::ios::fixed) << std::setprecision(3) << a / 10;
		bstr << b / 10;
		cstr << c / 10;
		Vstr << a * b*c / 1000000 << "dm^3";
		if (bTimeRecorded)timestr << "costs" << time(NULL) - startTime << "s, fps: " << ((float)1) / (time(NULL) - startTime);
		else {
			std::cout << "a: " << a / 10 << std::endl;
			std::cout << "b: " << b / 10 << std::endl;
			std::cout << "c: " << c / 10 << std::endl;
		}
		if (a != 0) {
			cv::line(resShow, cv::Point2i(poly[0].x + (int)left.cols*cut_rate, poly[0].y + (int)left.rows*cut_rate), cv::Point2i(poly[1].x + (int)left.cols*cut_rate, poly[1].y + (int)left.rows*cut_rate), cv::Scalar(0, 0, 255));
			cv::putText(resShow, astr.str(), cv::Point2i((poly[0].x + poly[1].x) / 2 + (int)left.cols*cut_rate, (poly[0].y + poly[1].y) / 2 + (int)left.rows*cut_rate), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 0));
		}
		if (b != 0) {
			cv::line(resShow, cv::Point2i(poly[1].x + (int)left.cols*cut_rate, poly[1].y + (int)left.rows*cut_rate), cv::Point2i(poly[2].x + (int)left.cols*cut_rate, poly[2].y + (int)left.rows*cut_rate), cv::Scalar(0, 0, 255));
			cv::putText(resShow, bstr.str(), cv::Point2i((poly[2].x + poly[1].x) / 2 + (int)left.cols*cut_rate, (poly[2].y + poly[1].y) / 2 + (int)left.rows*cut_rate), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 0));
		}
		if (c != camera_height) {
			cv::putText(resShow, cstr.str(), cv::Point2i((poly[0].x + poly[2].x) / 2 + (int)left.cols*cut_rate, (poly[0].y + poly[2].y) / 2 + (int)left.rows*cut_rate), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0));
		}
		cv::line(resShow, cv::Point2i(poly[0].x + (int)left.cols*cut_rate, poly[0].y + (int)left.rows*cut_rate), cv::Point2i(poly[1].x + (int)left.cols*cut_rate, poly[1].y + (int)left.rows*cut_rate), cv::Scalar(0, 0, 255));
		cv::line(resShow, cv::Point2i(poly[1].x + (int)left.cols*cut_rate, poly[1].y + (int)left.rows*cut_rate), cv::Point2i(poly[2].x + (int)left.cols*cut_rate, poly[2].y + (int)left.rows*cut_rate), cv::Scalar(0, 0, 255));
		int VStrPosX = poly[1].x + (int)left.cols*cut_rate > left.cols - 40 ? left.cols - 40 : poly[1].x + (int)left.cols*cut_rate;
		cv::putText(resShow, Vstr.str(), cv::Point2i(VStrPosX, poly[1].y + (int)left.rows*cut_rate), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 255));
		auto rect = cv::boundingRect(poly);
		cv::rectangle(resShow, cv::Point2i(rect.tl().x + (int)left.cols*cut_rate, rect.tl().y + (int)left.rows*cut_rate), cv::Point2i(rect.br().x + (int)left.cols*cut_rate, rect.br().y + (int)left.rows*cut_rate), cv::Scalar(255, 0, 0));
	}
	cv::imshow("measure", resShow);
	if (bCollected) {
		std::ostringstream detectedImgPath;
		detectedImgPath << "detectresult/" << detectCount << ".jpg";
		cv::imwrite(detectedImgPath.str(), resShow);
	}
	cv::waitKey();
}

int main(int argc, char* argv[]) {
	stereo_calibrator calibrator(xmlfile, stereo_calibrator::init_mode::STATIC_PARAM);
	cv::Mat left, right;
	cv::VideoCapture cameral(0);
	cv::VideoCapture camerar(2);
	int detectCount = 0;
	ifstream fr("detectcount.txt");
	if (!fr) {
		std::cout << "can't open detectcount.txt..." << std::endl;
		system("pause");
		exit(1);
	}
	fr >> detectCount;
	fr.close();
	while (true) {
		if (!cameral.isOpened())return 1;
		if (!camerar.isOpened())return 1;
		cv::Mat framel, left;
		cameral >> framel;
		if (bVisualized)
			std::cout << framel.dims << endl;
		cv::Mat framer, right;
		camerar >> framer;
		if (bCollected) {
			cv::Mat collectedImg;
			cv::hconcat(framel, framer, collectedImg);
			std::ostringstream collectedImgPath;
			collectedImgPath << "collect/" << detectCount << ".jpg";
			cv::imwrite(collectedImgPath.str(), collectedImg);
		}
		cv::cvtColor(framel, left, cv::COLOR_RGB2GRAY);
		cv::cvtColor(framer, right, cv::COLOR_RGB2GRAY);
		detect(calibrator, left, right, detectCount, bVisualized, bCollected, bTimeRecorded, bmm);
		detectCount++;
		ofstream fw("detectcount.txt");
		fw << detectCount;
		fw.close();
		int c = cv::waitKey(40);
		if (27 == char(c))break;
	}
	return 0;
}