#pragma once

#include <QDebug>
#include <QImage>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"

QImage cvMat2QImage(const cv::Mat& mat, bool clone = true, bool rb_swap = true);
cv::Mat QImage2cvMat(QImage& image, bool clone = true, bool rb_swap = true);