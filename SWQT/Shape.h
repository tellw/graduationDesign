#pragma once
#include <vector>
#include <QPoint>
enum shape_type {rect, circle, poly};

class Shape{
public:
	shape_type st;
	std::vector<QPoint> points;
};