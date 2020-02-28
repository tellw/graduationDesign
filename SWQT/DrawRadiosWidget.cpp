#include "DrawRadiosWidget.h"
#include <QGridLayout>

DrawRadiosWidget::DrawRadiosWidget(QWidget* parent) :QWidget(parent) {
	QGridLayout* drawRadiosWidgetLayout = new QGridLayout();
	this->rectRadioButton = new QRadioButton("Rect");
	this->rectRadioButton->setChecked(true);
	this->circleRadioButton = new QRadioButton("Circle");
	//this->linesRadioButton = new QRadioButton("Lines");
	this->eraserRadioButton = new QRadioButton("Eraser");
	this->polyRadioButton = new QRadioButton("Poly");
	drawRadiosWidgetLayout->addWidget(this->rectRadioButton, 0, 0);
	drawRadiosWidgetLayout->addWidget(this->circleRadioButton, 0, 1);
	//drawRadiosWidgetLayout->addWidget(this->linesRadioButton, 1, 0);
	drawRadiosWidgetLayout->addWidget(this->polyRadioButton, 1, 0);
	drawRadiosWidgetLayout->addWidget(this->eraserRadioButton, 1, 1);
	this->setLayout(drawRadiosWidgetLayout);
}