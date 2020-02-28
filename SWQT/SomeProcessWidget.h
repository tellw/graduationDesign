#pragma once

#include <QWidget>
#include <QCheckBox>

class SomeProcessWidget :public QWidget {
	Q_OBJECT

public:
	SomeProcessWidget(QWidget* parent=Q_NULLPTR);
	QCheckBox* gaussianBlurCheckBox;
	QCheckBox* cannyCheckBox;
	QCheckBox* CLAHECheckBox;
	QCheckBox* sharpenCheckBox;
	QCheckBox* laplacianCheckBox;
	QCheckBox* grayCheckBox;
	QCheckBox* OTSUCheckBox;
	QCheckBox* colorBlockCheckBox;
};