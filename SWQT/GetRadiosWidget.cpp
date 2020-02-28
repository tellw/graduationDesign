#include "GetRadiosWidget.h"
#include <QGridLayout>

GetRadiosWidget::GetRadiosWidget(QWidget* parent) :QWidget(parent) {
	QGridLayout* getRadiosWidgetLayout = new QGridLayout();
	this->picRadioButton = new QRadioButton("Pic");
	this->picRadioButton->setChecked(true);
	this->captureSelectedPicRadioButton = new QRadioButton("CaputreSelectedPic");
	this->captureRadioButton = new QRadioButton("Capture");
	getRadiosWidgetLayout->addWidget(this->picRadioButton, 0, 0);
	getRadiosWidgetLayout->addWidget(this->captureSelectedPicRadioButton, 0, 1);
	getRadiosWidgetLayout->addWidget(this->captureRadioButton, 1, 0);
	this->setLayout(getRadiosWidgetLayout);
}