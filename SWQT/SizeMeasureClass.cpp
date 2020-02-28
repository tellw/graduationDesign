#include "SizeMeasureClass.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <set>
#include <opencv2/calib3d.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>
#include <iomanip>
#include <QDebug>

double convex_area(cv::InputArray contour) {
	std::vector<cv::Point2i> hull;
	cv::convexHull(contour, hull);
	return cv::contourArea(hull);
}

double _distance_2D(cv::Point2i A, cv::Point2i B) {
	return sqrt((A.x - B.x)*(A.x - B.x) + (A.y - B.y)*(A.y - B.y));
}

void regularize_output_approx(std::vector<cv::Point2i>& approx, std::vector<cv::Point2i>& res, cv::Size s, bool bVisualized, float toleratedMinDistCoeff = 0.1) {
	std::vector<cv::Point2i> red;
	std::vector<double> firstPointDist;
	for (int i = 1; i < approx.size(); i++) {
		firstPointDist.push_back(_distance_2D(approx[0], approx[i]));
		if (bVisualized)QDebug(QtDebugMsg) << "distance between 0 and " << i << " point is " << _distance_2D(approx[0], approx[i]) << "\n";
	}
	std::sort(firstPointDist.begin(), firstPointDist.end());
	double toleratedMinDist = 0;
	std::set<int> thrownPointIndexes;
	if (approx.size() >= 4) {
		toleratedMinDist = toleratedMinDistCoeff * firstPointDist[approx.size() - 4];
		if (bVisualized)QDebug(QtDebugMsg) << "toleratedMinDist is " << toleratedMinDist;
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
		QDebug(QtDebugMsg) << "red has " << red.size() << " points:";
		for (int i = 0; i < red.size(); i++)QDebug(QtDebugMsg) << "(" << red[i].x  << "," << red[i].y << ")" << ", ";
		QDebug(QtDebugMsg) << "\n";
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

//void _find_polys(const cv::Mat& image, std::vector<std::vector<cv::Point2i>>& polys, bool bVisualized, int gaussianBlurKSize = 3, int cannyLowThresh = 55, int cannyHighThresh = 110, double minAreaCoeff = 0.02, double approxPolyDPCoeff = 0.05) {
void _find_polys(const cv::Mat& image, std::vector<std::vector<cv::Point2i>>& polys, bool bVisualized, double minAreaCoeff = 0.02, double approxPolyDPCoeff = 0.05) {
	/*cv::Mat blurred;
	cv::GaussianBlur(image, blurred, cv::Size(gaussianBlurKSize, gaussianBlurKSize), 1, 1);
	cv::Mat edges;
	cv::Canny(blurred, edges, cannyLowThresh, cannyHighThresh);
	if (bVisualized) {
		cv::imshow("edges", edges);
		cv::waitKey();
	}*/
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	/*cv::findContours(edges, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);*/
	cv::findContours(image, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
	if (bVisualized) {
		cv::Mat contoursShow(image.rows, image.cols, CV_8UC3, cv::Scalar(0));
		cv::drawContours(contoursShow, contours, -1, cv::Scalar(0, 255, 0));
		std::ostringstream oss;
		oss << "found " << contours.size() << " contours";
		std::string contourNumStr = oss.str();
		cv::putText(contoursShow, contourNumStr, cv::Size(0, 40), cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 255), 1);
		cv::imshow("found contours", contoursShow);
		QDebug(QtDebugMsg) << "the size of contours found: " << contours.size() << "\n";
		cv::waitKey();
	}
	double min_area = minAreaCoeff * image.rows*image.cols;
	if (bVisualized)QDebug(QtDebugMsg) << "min_area=" << min_area << "\n";
	int count = 1;
	for (auto& contour : contours) {
		double convex_esti_area = convex_area(contour);
		cv::Mat contourShow(image.rows, image.cols, CV_8UC3, cv::Scalar(0));
		if (bVisualized) {
			cv::drawContours(contourShow, std::vector<std::vector<cv::Point2i>>{contour}, -1, cv::Scalar(255, 255, 0));
			QDebug(QtDebugMsg) << count << "th seeking: " << convex_esti_area;
		}
		count++;
		if (convex_esti_area < min_area) {
			if (bVisualized)QDebug(QtDebugMsg) << " the area of the contour is invalid" << "\n";
			continue;
		}
		std::vector<cv::Point2i> approx;
		double len = cv::arcLength(contour, true);
		cv::approxPolyDP(contour, approx, len*approxPolyDPCoeff, false);
		if (bVisualized) {
			cv::drawContours(contourShow, std::vector<std::vector<cv::Point2i>>{approx}, -1, cv::Scalar(0, 255, 255));
			cv::imshow("contourAndApproxShow", contourShow);
			QDebug(QtDebugMsg) << "approx has " << approx.size() << " points, ";
			for (int i = 0; i < approx.size(); i++)QDebug(QtDebugMsg) << "(" << approx[i].x << "," << approx[i].y << ")" << ", ";
			QDebug(QtDebugMsg) << "\n";
			cv::waitKey();
		}
		cv::Mat c(image.rows, image.cols, CV_8UC3, cv::Scalar(0));
		std::vector<cv::Point2i> reg;
		cv::Size s(image.cols, image.rows);
		regularize_output_approx(approx, reg, s, bVisualized);
		if (reg.size() <= 2)continue;
		if (bVisualized) {
			QDebug(QtDebugMsg) << "reg has " << reg.size() << " points, ";
			for (int i = 0; i < reg.size(); i++)QDebug(QtDebugMsg) <<"(" << reg[i].x << "." << reg[i].y << ")" << ", ";
			QDebug(QtDebugMsg) << "\n";
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

//cv::Mat compute_disparity(const cv::Mat& left, const cv::Mat& right, float CLAHECoeff) {
cv::Mat compute_disparity(const cv::Mat& left, const cv::Mat& right) {
	/*cv::Mat clahed_left, clahed_right;
	auto clahe = cv::createCLAHE(CLAHECoeff);
	clahe->apply(left, clahed_left);
	clahe->apply(right, clahed_right);
	cv::Mat filtered_left, filtered_right;
	float laplacian[] = { 0.f, -1.f, 0.f, -1.f, 5.f, -1.f, 0.f, -1.f, 0.f };
	auto kernel = cv::Mat(3, 3, CV_32F, laplacian);
	cv::filter2D(clahed_left, filtered_left, -1, kernel);
	cv::filter2D(clahed_right, filtered_right, -1, kernel);*/
	const int win_size = 3;
	auto matcher_left = cv::StereoSGBM::create(
		0, 16 * 7, 2, 8 * 2 * win_size*win_size, 32 * 10 * win_size*win_size,
		8, 31, 1, 8, 8, cv::StereoSGBM::MODE_SGBM_3WAY
	);
	cv::Mat displ, true_displ;
	/*matcher_left->compute(filtered_left, filtered_right, displ);*/
	matcher_left->compute(left, right, displ);
	displ.convertTo(true_displ, CV_16S, 1.0 / 16);
	auto matcher_right = cv::ximgproc::createRightMatcher(matcher_left);
	cv::Mat dispr, true_dispr;
	matcher_right->compute(left, right, dispr);
	dispr.convertTo(true_dispr, CV_16S, 1.0 / 16);
	const int lambda = 3000;
	const double sigma = 10;
	auto filter = cv::ximgproc::createDisparityWLSFilter(matcher_left);
	filter->setLambda(lambda);
	filter->setSigmaColor(sigma);
	cv::Mat filtered_displ;
	filter->filter(true_displ, left, filtered_displ, true_dispr);
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

cloud_ptr reproject(const cv::Mat& disparity, const cv::Mat& q, bool bmm, cv::Vec2f zclamp = cv::Vec2f(100., 1000.)) {
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

SizeMeasureClass::SizeMeasureClass(CMyINI* cMyIni, SizeMeasureOptionWidget* sizeMeasureOptionWidget) {
	this->cMyIni = cMyIni;
	this->sizeMeasureOptionWidget = sizeMeasureOptionWidget;
	std::stringstream ss;
	int visualized, collected, timeRecorded, mm;
	this->xmlfile = this->cMyIni->GetValue("binocular_cameras", "xmlfile");
	ss << this->cMyIni->GetValue("binocular_cameras", "cut_rate");
	ss >> this->cut_rate;
	ss.clear();
	ss << this->cMyIni->GetValue("binocular_cameras", "bVisualized");
	ss >> visualized;
	ss.clear();
	if (visualized == 0)this->bVisualized = false;
	else this->bVisualized = true;
	ss << this->cMyIni->GetValue("binocular_cameras", "bCollected");
	ss >> collected;
	ss.clear();
	if (collected == 0)this->bCollected = false;
	else this->bCollected = true;
	ss << this->cMyIni->GetValue("binocular_cameras", "bTimeRecorded");
	ss >> timeRecorded;
	ss.clear();
	if (timeRecorded == 0)this->bTimeRecorded = false;
	else this->bTimeRecorded = true;
	ss << this->cMyIni->GetValue("binocular_cameras", "bmm");
	ss >> mm;
	ss.clear();
	if (mm == 0)this->bmm = false;
	else this->bmm = true;
	ss << this->cMyIni->GetValue("binocular_cameras", "detectCount");
	ss >> this->detectCount;
	ss.clear();
	ss.str("");
	this->calibrator = new stereo_calibrator(this->xmlfile, stereo_calibrator::init_mode::STATIC_PARAM);
}

void SizeMeasureClass::detect(cv::Mat& left, cv::Mat& right, cv::Mat& left_src, cv::Mat& right_src) {
	if (this->sizeMeasureOptionWidget->bVisualizedSlider->value() == 0)this->bVisualized = false;
	else this->bVisualized = true;
	if (this->sizeMeasureOptionWidget->bCollectedSlider->value() == 0)this->bCollected = false;
	else this->bCollected = true;
	if (this->sizeMeasureOptionWidget->bTimeRecordedSlider->value() == 0)this->bTimeRecorded = false;
	else this->bTimeRecorded = true;
	if (this->sizeMeasureOptionWidget->bmmSlider->value() == 0)this->bmm = false;
	else this->bmm = true;
	if (this->sizeMeasureOptionWidget->bAutoSlider->value() == 0)this->bAuto = false;
	else this->bAuto = true;
	/*if (this->sizeMeasureOptionWidget->bPreciseSlider->value() == 0)this->bPrecise = false;
	else this->bPrecise = false;*/
	if (this->bCollected) {
		std::ostringstream collectedLeftImgPath, collectedRightImgPath;
		collectedLeftImgPath << "collect/left/" << this->detectCount << ".jpg";
		collectedRightImgPath << "collect/right/" << this->detectCount << ".jpg";
		cv::imwrite(collectedLeftImgPath.str(), left);
		cv::imwrite(collectedRightImgPath.str(), right);
	}
	time_t startTime;
	if (this->bTimeRecorded)startTime = time(NULL);
	auto rectified_left = this->calibrator->rectify_left(left);
	auto rectified_right = this->calibrator->rectify_right(right);
	if (this->bVisualized) {
		cv::imshow("rectified_l", rectified_left);
		cv::imshow("rectified_r", rectified_right);
		cv::waitKey();
	}
	auto left_roi = rectified_left(cv::Rect2i(this->cut_rate*left.cols, this->cut_rate*left.rows, (1.0 - 2 * this->cut_rate)*left.cols, (1.0 - 2 * this->cut_rate)*left.rows));
	auto right_roi = rectified_right(cv::Rect2i(this->cut_rate*right.cols, this->cut_rate*right.rows, (1.0 - 2 * this->cut_rate)*right.cols, (1.0 - 2 * this->cut_rate)*right.rows));
	if (this->bVisualized) {
		cv::imshow("rectified_l", left_roi);
		cv::imshow("rectified_r", right_roi);
		cv::waitKey();
	}
	//std::vector<cv::Point2i> pts;
	std::vector<std::vector<cv::Point2i>> polys;
	if (this->sizeMeasureOptionWidget->contourLookingForRectsRadioButton->isChecked())
		_find_polys(left_roi, polys, this->bVisualized);
	cv::Mat disp;
	if (this->sizeMeasureOptionWidget->SGBMDisparityRadioButton->isChecked()) {
		disp = compute_disparity(left_roi, right_roi);
		cv::imshow("disp", visualize_disparity(disp));
	}
	cloud_ptr cloud;
	if (this->sizeMeasureOptionWidget->easyStyleRadioButton->isChecked())
		cloud = reproject(disp, this->calibrator->q, this->bmm);
	cv::Mat resShow = convertTo3Channels(rectified_left);
	for (auto& poly : polys) {
		auto& pt0 = cloud->at(poly[0].x, poly[0].y);
		auto& pt1 = cloud->at(poly[1].x, poly[1].y);
		auto& pt2 = cloud->at(poly[2].x, poly[2].y);
		auto& pt3 = cloud->at(poly[3].x, poly[3].y);
		if (!bTimeRecorded) {
			QDebug(QtDebugMsg) << "(" << poly[0].x << "," << poly[0].y << ")" << "\'s world coordinate is " << "(" << pt0.x << "," << pt0.y << "," << pt0.z << ")" << "\n";
			QDebug(QtDebugMsg) << "(" << poly[1].x << "," << poly[1].y << ")" << "\'s world coordinate is " << "(" << pt1.x << "," << pt1.y << "," << pt1.z << ")" << "\n";
			QDebug(QtDebugMsg) << "(" << poly[2].x << "," << poly[2].y << ")" << "\'s world coordinate is " << "(" << pt2.x << "," << pt2.y << "," << pt2.z << ")" << "\n";
			QDebug(QtDebugMsg) << "(" << poly[3].x << "," << poly[3].y << ")" << "\'s world coordinate is " << "(" << pt3.x << "," << pt3.y << "," << pt3.z << ")" << "\n";
			double a = (_distance(pt0, pt1) + _distance(pt2, pt3)) / 2.0;
			double b = (_distance(pt1, pt2) + _distance(pt0, pt3)) / 2.0;
			std::vector<float> zs({ pt0.z, pt1.z, pt2.z, pt3.z });
			float zmin = this->camera_height;
			for (auto z : zs) {
				if (z > 0) {
					if (z < zmin)zmin = z;
				}
			}
			float c = this->camera_height - zmin;
			if (!this->bmm) {
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
				QDebug(QtDebugMsg) << "a: " << a / 10 << "\n";
				QDebug(QtDebugMsg) << "b: " << b / 10 << "\n";
				QDebug(QtDebugMsg) << "c: " << c / 10 << "\n";
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
		if (this->bCollected) {
			std::ostringstream detectedImgPath;
			detectedImgPath << "detectresult/" << this->detectCount << ".jpg";
			cv::imwrite(detectedImgPath.str(), resShow);
		}
		cv::waitKey();
	}
}

//void SizeMeasureClass::setCMyIni(CMyINI* cMyIni) {
//	this->cMyIni = cMyIni;
//}

void SizeMeasureClass::setBVisualized(bool bVisualized) {
	this->bVisualized = bVisualized;
}

void SizeMeasureClass::setBCollected(bool bCollected) {
	this->bCollected = bCollected;
}

void SizeMeasureClass::setBTimeRecorded(bool bTimeRecorded) {
	this->bTimeRecorded = bTimeRecorded;
}

void SizeMeasureClass::setBmm(bool bmm) {
	this->bmm = bmm;
}

void SizeMeasureClass::setBAuto(bool bAuto) {
	this->bAuto = bAuto;
}

//void SizeMeasureClass::setBPrecise(bool bPrecise) {
//	this->bPrecise = bPrecise;
//}