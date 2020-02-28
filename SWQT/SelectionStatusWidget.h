#pragma once
#include <QWidget>
#include <QSlider>
#include "SWQTCanvas.h"

class SelectionStatusWidget :public QWidget {
	Q_OBJECT

public:
	SelectionStatusWidget(QWidget* parent = Q_NULLPTR);
	QSlider* bScaleSlider;
	QSlider* bDrawSlider;
	SWQTCanvas* canvas;
	void setCanvas(SWQTCanvas* canvas);
private slots:
	void bScaleSliderValueChanged(int value);
	void bDrawSliderValueChanged(int value);
};