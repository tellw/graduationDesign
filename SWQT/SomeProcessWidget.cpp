#include "SomeProcessWidget.h"
#include <QGridLayout>

SomeProcessWidget::SomeProcessWidget(QWidget* parent) :QWidget(parent) {
	this->gaussianBlurCheckBox = new QCheckBox("Gaussian Blur");
	this->cannyCheckBox = new QCheckBox("Canny");
	this->CLAHECheckBox = new QCheckBox("CLAHE");
	this->sharpenCheckBox = new QCheckBox("Sharpen");
	this->laplacianCheckBox = new QCheckBox("Laplacian");
	this->grayCheckBox = new QCheckBox("Gray");
	this->OTSUCheckBox = new QCheckBox("OTSU");
	this->colorBlockCheckBox = new QCheckBox("Color Block");
	QGridLayout* someProcessWidgetLayout = new QGridLayout();
	someProcessWidgetLayout->addWidget(this->grayCheckBox, 0, 0);
	someProcessWidgetLayout->addWidget(this->gaussianBlurCheckBox, 0, 1);
	someProcessWidgetLayout->addWidget(this->cannyCheckBox, 3, 0);
	someProcessWidgetLayout->addWidget(this->CLAHECheckBox, 1, 0);
	someProcessWidgetLayout->addWidget(this->sharpenCheckBox, 1, 1);
	someProcessWidgetLayout->addWidget(this->laplacianCheckBox, 2, 0);
	someProcessWidgetLayout->addWidget(this->OTSUCheckBox, 2, 1);
	someProcessWidgetLayout->addWidget(this->colorBlockCheckBox, 3, 1);
	this->setLayout(someProcessWidgetLayout);
}