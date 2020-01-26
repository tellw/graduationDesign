#include "app.hpp"
#include "reconstruction.hpp"
#include <vector>
#include <sstream>

void _find_polys(const cv::Mat& image, std::vector<std::vector<cv::Point2i>>& polys, bool bVisualized, int gaussianBlurKSize=3, int cannyLowThresh=55, int cannyHighThresh=110, double minAreaCoeff=0.02, double approxPolyDPCoeff=0.05) {
	cv::Mat blurred;
	cv::GaussianBlur(image, blurred, cv::Size(gaussianBlurKSize, gaussianBlurKSize), 1, 1);
	cv::Mat edges;
	cv::Canny(blurred, edges, cannyLowThresh, cannyHighThresh);
	if (bVisualized) {
		cv::imshow("edges", edges);
		cv::waitKey();
	}
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(edges, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
	if (bVisualized) {
		cv::Mat contoursShow(edges.rows, edges.cols, CV_8UC3, cv::Scalar(0));
		cv::drawContours(contoursShow, contours, -1, cv::Scalar(0, 255, 0));
		std::ostringstream oss;
		oss << "found " << contours.size() << " contours";
		std::string contourNumStr = oss.str();
		cv::putText(contoursShow, contourNumStr, cv::Size(0, 40), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 255), 1);
		cv::imshow("found contours", contoursShow);
		std::cout << "the size of contours found: " << contours.size() << std::endl;
		cv::waitKey();
	}
	const double min_area = minAreaCoeff * image.rows*image.cols;
	if (bVisualized)std::cout << "min_area=" << min_area << std::endl;
	int count = 1;
	for (auto& contour : contours) {
		double convex_esti_area = convex_area(contour);
		cv::Mat contourShow(edges.rows, edges.cols, CV_8UC3, cv::Scalar(0));
		if (bVisualized) {
			cv::drawContours(contourShow, std::vector<std::vector<cv::Point2i>>{contour}, -1, cv::Scalar(255, 255, 0));
			std::cout << count << "th seeking: " << convex_esti_area;
		}
		count++;
		if (convex_esti_area < min_area) {
			if(bVisualized)std::cout << " the area of the contour is invalid" << std::endl;
			continue;
		}
		std::vector<cv::Point2i> approx;
		double len = cv::arcLength(contour, true);
		cv::approxPolyDP(contour, approx, len*approxPolyDPCoeff, false);
		if (bVisualized) {
			cv::drawContours(contourShow, std::vector<std::vector<cv::Point2i>>{approx}, -1, cv::Scalar(0, 255, 255));
			cv::imshow("contourAndApproxShow", contourShow);
			std::cout << "approx has " << approx.size() << " points, ";
			for (int i = 0; i < approx.size(); i++)std::cout << approx[i] << ", ";
			std::cout << std::endl;
			cv::waitKey();
		}
		cv::Mat c(image.rows, image.cols, CV_8UC3, cv::Scalar(0));
		std::vector<cv::Point2i> reg;
		cv::Size s(image.col, image.rows);
		regularize_output_approx(approx, reg, s, bVisualized);
		if (reg.size() <= 2)continue;
		if (bVisualized) {
			std::cout << "reg has " << reg.size() << " points, ";
			for (int i = 0; i < reg.size(); i++)std::cout << reg[i] << ", ";
			std::cout << std::endl;
		}
		if (reg.size() == 4)polys.push_back(reg);
		auto rect = cv::boundingRect(reg);
		if (bVisualized) {
			for (auto& pt : contour)c.at<cv::Vec3b>(pt.y, pt.x) = (4 == reg.size() ? cv::Vec3b(255, 0, 0) : cv::Vec3b(255, 255, 255));
			for (auto& pt : reg)cv::circle(c, pt, 5, cv::Vec3b(255, 255, 0));
			cv::rectangle(c, cv::Point2i(rect.x, rect.y), cv::Point2i(rect.x + rect.width, rect.y + rect.height), cv::Scalar(0, 0, 255));
			cv::fillPoly(c, std::vector<std::vector<cv::Point2i>>{reg}, cv::Scalar(255));
			cv::imshow("patternDetect", c);
			cv::waitKey();
		}
	}
	
}
double convex_area(cv::InputArray contour) {
	std::vector<cv::Point2i> hull;
	cv::convexHull(contour, hull);
	return cv::contourArea(hull);
}

void regularize_output_approx(std::vector<cv::Point2i>& approx, std::vector<cv::Point2i>& res, cv::Size s, bool bVisualized, float toleratedMinDistCoeff=0.1) {
	std::vector<cv::Point2i> red;
	std::vector<double> firstPointDist;
	for (int i = 1; i < approx.size(); i++) {
		firstPointDist.push_back(_distance_2D(approx[0], approx[i]));
		if (bVisualized)std::cout << "distance between 0 and " << i << " point is " << _distance_2D(approx[0], approx[i]) << std::endl;
	}
	std::sort(firstPointDist.begin(), firstPointDist.end());
	double toleratedMinDist = 0;
	std::set<int> thrownPointIndexes;
	if (approx.size() >= 4) {
		toleratedMinDist = toleratedMinDistCoeff * firstPointDist[approx.size() - 4];
		if (bVisualized)std::cout << "toleratedMinDist is " << toleratedMinDist;
		for (int i = 0; i < approx.size() - 1; i++) {
			if (thrownPointIndexes.find(i) != thrownPointIndexes.end())continue;
			else {
				for (int j = i + 1; j < approx.size(); j++) {
					if (thrownPointIndexes.find(j) != thrownPointIndexes.end())continue;
					else {
						if (_distance_2D(approx[i], approx[j]) < toleratedMinDist)thrownPointIndexes.insert(j);
					}
				}
			}
		}
	}
	for (int i = 0; i < approx.size(); i++) {
		if (thrownPointIndexes.find(i) == thrownPointIndexes.end())red.push_back(approx[i]);
	}
	if (bVisualized) {
		std::cout << "red has " << red.size() << " points:";
		for (int i = 0; i < red.size(); i++)std::cout << red[i] << ", ";
		std::cout << std::endl;
	}
	if (red.size() == 4) {
		res.push_back(red[0]);
		double redFirstPointDist[3];
		for (int i = 0; i < 3; i++)redFirstPointDist[i] = _distance_2D(red[0], red[i + 1]);
		int maxIndex = 0, minIndex, middleIndex;
		for (int i = 1; i < 3; i++) {
			if (redFirstPointDist[i] > redFirstPointDist[maxIndex]) maxIndex = i;
		}
		if (redFirstPointDist[(maxIndex + 1) % 3] > redFirstPointDist[(maxIndex + 2) % 3]) {
			minIndex = (maxIndex + 2) % 3;
			middleIndex = (maxIndex + 1) % 3;
		}
		else {
			minIndex = (maxIndex + 1) % 3;
			middleIndex = (maxIndex + 2) % 3;
		}
		res.push_back(red[minIndex + 1]);
		res.push_back(red[maxIndex + 1]);
		res.push_back(red[middleIndex + 1]);
	}
	else if (red.size() == 3) {
		double triAngleCos[3];
		for (int i = 0; i < 3; i++)triAngleCos[i] = ((red[i].x - red[(i + 2) % 3].x)*(red[i].x - red[(i + 1) % 3].x) + (red[i].y - red[(i + 2) % 3].y)*(red[i].y - red[(i + 1) % 3].y)) / (_distance_2D(red[i], red[(i + 2) % 3])*_distance_2D(red[i], red[(i + 1) % 3]));
		double cosMax = 1.0;
		int cosMinIndex = 0;
		for (int i = 0; i < 3; i++) {
			if (triAngleCos[i] < cosMax) {
				cosMinIndex = i;
				cosMax = triAngleCos[i];
			}
		}
		res.push_back(red[cosMinIndex]);
		res.push_back(red[(cosMinIndex + 1) % 3]);
		int caledForthPointX = red[(cosMinIndex + 2) % 3].x + red[(cosMinIndex + 1) % 3].x - red[cosMinIndex].x;
		if (caledForthPointX < 0)caledForthPointX = 0;
		else if (caledForthPointX > s.width - 1)caledForthPointX = s.width - 1;
		int caledForthPointY = red[(cosMinIndex + 2) % 3].y + red[(cosMinIndex + 1) % 3].y - red[cosMinIndex].y;
		if (caledForthPointY < 0)caledForthPointY = 0;
		else if (caledForthPointY > s.height - 1)caledForthPointY = s.height - 1;
		res.push_back(cv::Point2i({ caledForthPointX, caledForthPointY }));
		res.push_back(red[(cosMinIndex + 2) % 3]);
	}
	else if (red.size() > 4) {
		std::vector<float> disTo0, k;
		for (int i = 0; i < red.size(); i++) {
			disTo0.push_back(sqrt(red[i].x*red[i].x + red[i].y*red[i].y));
			k.push_back(((float)red[i].y) / red[i].x);
		}
		int distMinIndex = std::min_element(disTo0.begin(), disTo0.end()) - disTo0.begin();
		res.push_back(red[distMinIndex]);
		disTo0.erase(std::begin(disTo0) + distMinIndex);
		k.erase(std::begin(k) + distMinIndex);
		red.erase(std::begin(red) + distMinIndex);
		int kMaxIndex = std::max_element(k.begin(), k.end()) - k.begin();
		res.push_back(red[kMaxIndex]);
		disTo0.erase(std::begin(disTo0) + kMaxIndex);
		k.erase(std::begin(k) + kMaxIndex);
		red.erase(std::begin(red) + kMaxIndex);
		int distMaxIndex = std::max_element(disTo0.begin(), disTo0.end()) - disTo0.begin();
		res.push_back(red[distMaxIndex]);
		disTo0.erase(std::begin(disTo0) + distMaxIndex);
		k.erase(std::begin(k) + distMaxIndex);
		red.erase(std::begin(red) + distMaxIndex);
		int kMinIndex = std::min_element(k.begin(), k.end()) - k.begin();
		res.push_back(red[kMinIndex]);
	}
}

double _distance_2D(cv::Point2i A, cv::Point2i B) {
	return sqrt((A.x - B.x)*(A.x - B.x) + (A.y - B.y)*(A.y - B.y));
}