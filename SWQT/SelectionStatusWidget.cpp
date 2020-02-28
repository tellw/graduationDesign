#include "SelectionStatusWidget.h"
//#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

SelectionStatusWidget::SelectionStatusWidget(QWidget* parent) :QWidget(parent) {
	//QGridLayout* selectionStatusWidgetLayout = new QGridLayout();
	QHBoxLayout* selectionStatusWidgetLayout = new QHBoxLayout();
	QLabel* bScaleLabel = new QLabel("bScale");
	this->bScaleSlider = new QSlider(Qt::Horizontal);
	this->bScaleSlider->setMinimum(0);
	this->bScaleSlider->setMaximum(1);
	this->bScaleSlider->setValue(0);
	this->bScaleSlider->setSingleStep(1);
	this->bScaleSlider->setPageStep(1);
	connect(this->bScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(bScaleSliderValueChanged(int)));
	QLabel* bDrawLabel = new QLabel("bDraw");
	this->bDrawSlider = new QSlider(Qt::Horizontal);
	this->bDrawSlider->setMinimum(0);
	this->bDrawSlider->setMaximum(1);
	this->bDrawSlider->setValue(0);
	this->bDrawSlider->setSingleStep(1);
	this->bDrawSlider->setPageStep(1);
	connect(this->bDrawSlider, SIGNAL(valueChanged(int)), this, SLOT(bDrawSliderValueChanged(int)));
	/*selectionStatusWidgetLayout->addWidget(bScaleLabel, 0, 0);
	selectionStatusWidgetLayout->addWidget(bScaleSlider, 0, 1);
	selectionStatusWidgetLayout->addWidget(bDrawLabel, 1, 0);
	selectionStatusWidgetLayout->addWidget(bDrawSlider, 1, 1);*/
	selectionStatusWidgetLayout->addWidget(bScaleLabel);
	selectionStatusWidgetLayout->addWidget(bScaleSlider);
	selectionStatusWidgetLayout->addWidget(bDrawLabel);
	selectionStatusWidgetLayout->addWidget(bDrawSlider);
	this->setLayout(selectionStatusWidgetLayout);
}

void SelectionStatusWidget::setCanvas(SWQTCanvas* canvas) {
	this->canvas = canvas;
}

void SelectionStatusWidget::bScaleSliderValueChanged(int value) {
	if (value == 0)this->canvas->bScale = false;
	else {
		if (this->bDrawSlider->value() == 1)this->bDrawSlider->setValue(0);
		this->canvas->bScale = true;
	}
}

void SelectionStatusWidget::bDrawSliderValueChanged(int value) {
	if (value == 0)this->canvas->bDraw = false;
	else {
		if (this->bScaleSlider->value() == 1)this->bScaleSlider->setValue(0);
		this->canvas->bDraw = true;
	}
}