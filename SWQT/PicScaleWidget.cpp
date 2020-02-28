#include "PicScaleWidget.h"
#include <QLabel>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QDebug>

PicScaleWidget::PicScaleWidget(QWidget* parent):QWidget(parent) {
	QLabel* picScaleLabel = new QLabel(tr("Scale: "));
	this->picScaleSlider = new QSlider(Qt::Horizontal);
	this->picScaleSlider->setMinimum(5);
	this->picScaleSlider->setMaximum(30);
	this->picScaleSlider->setValue(10);
	this->picScaleSlider->setSingleStep(1);
	this->picScaleSlider->setPageStep(5);
	connect(this->picScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(picScaleSliderValueChanged(int)));
	this->picScaleLE = new QLineEdit("1");
	QDoubleValidator* picScaleLEValidator = new QDoubleValidator(0.5, 3, 1, this->picScaleLE);
	picScaleLEValidator->setNotation(QDoubleValidator::StandardNotation);
	this->picScaleLE->setValidator(picScaleLEValidator);
	this->picScaleLE->setFixedWidth(27);
	connect(this->picScaleLE, SIGNAL(textChanged(QString)), this, SLOT(picScaleLETextChanged(QString)));
	QHBoxLayout* picScaleLayout = new QHBoxLayout();
	picScaleLayout->addWidget(picScaleLabel);
	picScaleLayout->addWidget(picScaleSlider);
	picScaleLayout->addWidget(picScaleLE);
	this->setLayout(picScaleLayout);
}

void PicScaleWidget::picScaleSliderValueChanged(int value) {
	//QDebug(QtDebugMsg) << "picScaleSlider's value is : " << value << "\n";
	this->picScale = (float)value / 10;
	QString picScaleText = QString("%1").arg(this->picScale);
	this->picScaleLE->setText(picScaleText);
}

void PicScaleWidget::picScaleLETextChanged(QString text) {
	//QDebug(QtDebugMsg) << "picScaleLE's text is : " << text << "\n";
	this->picScale = text.toFloat();
	if (this->picScale > 3)this->picScale = 3;
	else if (this->picScale < 0.5)this->picScale = 0.5;
	//QDebug(QtDebugMsg) << "picScaleLE's float is : " << this->picScale << "\n";
	this->picScaleSlider->setValue((int)(this->picScale * 10));
}