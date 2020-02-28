#pragma once

#include <QWidget>
#include <QSlider>
#include <QLineEdit>

class PicScaleWidget :public QWidget {
	Q_OBJECT

public:
	PicScaleWidget(QWidget* parent=Q_NULLPTR);
	float picScale = 1.0;
	QSlider* picScaleSlider;
	QLineEdit* picScaleLE;
	
private slots:
	void picScaleSliderValueChanged(int value);
	void picScaleLETextChanged(QString text);
};