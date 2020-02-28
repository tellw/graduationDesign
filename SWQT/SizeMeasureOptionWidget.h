#pragma once
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QRadioButton>
#include "CMyINI.h"
//#include "SizeMeasureClass.h"
#include <QCheckBox>
#include <QLineEdit>

class SizeMeasureOptionWidget :public QWidget {
	Q_OBJECT

public:
	/*SizeMeasureOptionWidget(CMyINI* cMyIni, SizeMeasureClass* sizeMeasureClass, QWidget* parent=Q_NULLPTR);*/
	SizeMeasureOptionWidget(QWidget* parent = Q_NULLPTR);
	QLabel* bVisualizedLabel;
	QLabel* bCollectedLabel;
	QLabel* bTimeRecordedLabel;
	QLabel* bmmLabel;
	QLabel* bAutoLabel;
	/*QLabel* bPreciseLabel;*/
	QSlider* bVisualizedSlider;
	QSlider* bCollectedSlider;
	QSlider* bTimeRecordedSlider;
	QSlider* bmmSlider;
	QSlider* bAutoSlider;
	/*QSlider* bPreciseSlider;*/
	QLabel* processThingLabel;
	QRadioButton* contourLookingForRectsRadioButton;
	QLabel* disparityStyleLabel;
	QRadioButton* SGBMDisparityRadioButton;
	QLabel* constructionStyleLabel;
	QRadioButton* easyStyleRadioButton;
	QLabel* sizeCalculationLabel;
	QRadioButton* rectCalculationRadioButton;
	CMyINI* cMyIni = new CMyINI();
	QCheckBox* cameraToGroundHeightCheckBox;
	QLineEdit* cameraToGroundHeightLE;
private slots:
	void bVisualizedSliderValueChanged(int value);
	void bCollectedSliderValueChanged(int value);
	void bTimeRecordedSliderValueChanged(int value);
	void bmmSliderValueChanged(int value);
	void bAutoSliderValueChanged(int value);
	void setCMyIni(CMyINI* cMyIni);
	/*void bPreciseSliderValueChanged(int value);*/
};