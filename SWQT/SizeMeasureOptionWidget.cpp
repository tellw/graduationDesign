#include "SizeMeasureOptionWidget.h"
#include <QGridLayout>
#include <QButtonGroup>

SizeMeasureOptionWidget::SizeMeasureOptionWidget(QWidget* parent) : QWidget(parent) {
	QGridLayout* sizeMeasureOptionWidgetLayout = new QGridLayout();
	this->cameraToGroundHeightCheckBox = new QCheckBox("Camera to ground height:");
	this->cameraToGroundHeightLE = new QLineEdit();
	QLabel* cmLabel = new QLabel("cm");
	sizeMeasureOptionWidgetLayout->addWidget(this->cameraToGroundHeightCheckBox, 0, 0, 1, 2);
	sizeMeasureOptionWidgetLayout->addWidget(this->cameraToGroundHeightLE, 0, 2);
	sizeMeasureOptionWidgetLayout->addWidget(cmLabel, 0, 3);
	this->cMyIni->ReadINI("SWQT.ini");
	std::stringstream ss;
	int value;
	this->bVisualizedLabel = new QLabel("bVisualized");
	this->bVisualizedSlider = new QSlider(Qt::Horizontal);
	this->bVisualizedSlider->setMinimum(0);
	this->bVisualizedSlider->setMaximum(1);
	ss << this->cMyIni->GetValue("binocular_cameras", "bVisualized");
	ss >> value;
	ss.clear();
	this->bVisualizedSlider->setValue(value);
	this->bVisualizedSlider->setSingleStep(1);
	this->bVisualizedSlider->setPageStep(1);
	connect(this->bVisualizedSlider, SIGNAL(valueChanged(int)), this, SLOT(bVisualizedSliderValueChanged(int)));
	this->bCollectedLabel = new QLabel("bCollected");
	this->bCollectedSlider = new QSlider(Qt::Horizontal);
	this->bCollectedSlider->setMinimum(0);
	this->bCollectedSlider->setMaximum(1);
	ss << this->cMyIni->GetValue("binocular_cameras", "bCollected");
	ss >> value;
	ss.clear();
	this->bCollectedSlider->setValue(value);
	this->bCollectedSlider->setSingleStep(1);
	this->bCollectedSlider->setPageStep(1);
	connect(this->bCollectedSlider, SIGNAL(valueChanged(int)), this, SLOT(bCollectedSliderValueChanged(int)));
	this->bTimeRecordedLabel = new QLabel("bTimeRecorded");
	this->bTimeRecordedSlider = new QSlider(Qt::Horizontal);
	this->bTimeRecordedSlider->setMinimum(0);
	this->bTimeRecordedSlider->setMaximum(1);
	ss << this->cMyIni->GetValue("binocular_cameras", "bTimeRecorded");
	ss >> value;
	ss.clear();
	this->bTimeRecordedSlider->setValue(value);
	this->bTimeRecordedSlider->setSingleStep(1);
	this->bTimeRecordedSlider->setPageStep(1);
	connect(this->bTimeRecordedSlider, SIGNAL(valueChanged(int)), this, SLOT(bTimeRecordedSliderValueChanged(int)));
	this->bmmLabel = new QLabel("bmm");
	this->bmmSlider = new QSlider(Qt::Horizontal);
	this->bmmSlider->setMinimum(0);
	this->bmmSlider->setMaximum(1);
	ss << this->cMyIni->GetValue("binocular_cameras", "bmm");
	ss >> value;
	ss.clear();
	this->bmmSlider->setValue(value);
	this->bmmSlider->setSingleStep(1);
	this->bmmSlider->setPageStep(1);
	connect(this->bmmSlider, SIGNAL(valueChanged(int)), this, SLOT(bmmSliderValueChanged(int)));
	this->bAutoLabel = new QLabel("bAuto");
	this->bAutoSlider = new QSlider(Qt::Horizontal);
	this->bAutoSlider->setMinimum(0);
	this->bAutoSlider->setMaximum(1);
	ss << this->cMyIni->GetValue("binocular_cameras", "bAuto");
	ss >> value;
	ss.clear();
	ss.str("");
	this->bAutoSlider->setValue(value);
	this->bAutoSlider->setSingleStep(1);
	this->bAutoSlider->setPageStep(1);
	connect(this->bAutoSlider, SIGNAL(valueChanged(int)), this, SLOT(bAutoSliderValueChanged(int)));
	/*this->bPreciseLabel = new QLabel("bPrecise");
	this->bPreciseSlider = new QSlider(Qt::Horizontal);
	this->bPreciseSlider->setMinimum(0);
	this->bPreciseSlider->setMaximum(1);
	ss << this->cMyIni->GetValue("binocular_cameras", "bPrecise");
	ss >> value;
	ss.clear();
	this->bVisualizedSlider->setValue(value);
	this->bVisualizedSlider->setSingleStep(1);
	this->bVisualizedSlider->setPageStep(1);
	connect(this->bPreciseSlider, SIGNAL(valueChanged(int)), this, SLOT(bPreciseSliderValueChanged(int)));*/
	sizeMeasureOptionWidgetLayout->addWidget(this->bVisualizedLabel, 1, 0);
	sizeMeasureOptionWidgetLayout->addWidget(this->bVisualizedSlider, 1, 1);
	sizeMeasureOptionWidgetLayout->addWidget(this->bCollectedLabel, 1, 2);
	sizeMeasureOptionWidgetLayout->addWidget(this->bCollectedSlider, 1, 3);
	sizeMeasureOptionWidgetLayout->addWidget(this->bTimeRecordedLabel, 2, 0);
	sizeMeasureOptionWidgetLayout->addWidget(this->bTimeRecordedSlider, 2, 1);
	sizeMeasureOptionWidgetLayout->addWidget(this->bmmLabel, 2, 2);
	sizeMeasureOptionWidgetLayout->addWidget(this->bmmSlider, 2, 3);
	sizeMeasureOptionWidgetLayout->addWidget(this->bAutoLabel, 3, 0);
	sizeMeasureOptionWidgetLayout->addWidget(this->bAutoSlider, 3, 1);
	/*sizeMeasureOptionWidgetLayout->addWidget(this->bPreciseLabel, 2, 2);
	sizeMeasureOptionWidgetLayout->addWidget(this->bPreciseSlider, 2, 3);*/
	QButtonGroup* processThingButtonGroup = new QButtonGroup(this);
	QButtonGroup* disparityStyleButtonGroup = new QButtonGroup(this);
	QButtonGroup* constructionStyleButtonGroup = new QButtonGroup(this);
	QButtonGroup* sizeCalculationButtonGroup = new QButtonGroup(this);
	this->processThingLabel = new QLabel("Process thing:");
	this->contourLookingForRectsRadioButton = new QRadioButton("Contour looking for rects");
	this->contourLookingForRectsRadioButton->setChecked(true);
	processThingButtonGroup->addButton(this->contourLookingForRectsRadioButton);
	sizeMeasureOptionWidgetLayout->addWidget(this->processThingLabel, 4, 0, 1, 2);
	sizeMeasureOptionWidgetLayout->addWidget(this->contourLookingForRectsRadioButton, 4, 2, 1, 2);
	this->disparityStyleLabel = new QLabel("Disparity style:");
	this->SGBMDisparityRadioButton = new QRadioButton("SGBMDisparity");
	this->SGBMDisparityRadioButton->setChecked(true);
	disparityStyleButtonGroup->addButton(this->SGBMDisparityRadioButton);
	sizeMeasureOptionWidgetLayout->addWidget(this->disparityStyleLabel, 5, 0, 1, 2);
	sizeMeasureOptionWidgetLayout->addWidget(this->SGBMDisparityRadioButton, 5, 2, 1, 2);
	this->constructionStyleLabel = new QLabel("Construction style:");
	this->easyStyleRadioButton = new QRadioButton("Easy style");
	this->easyStyleRadioButton->setChecked(true);
	constructionStyleButtonGroup->addButton(this->easyStyleRadioButton);
	sizeMeasureOptionWidgetLayout->addWidget(this->constructionStyleLabel, 6, 0, 1, 2);
	sizeMeasureOptionWidgetLayout->addWidget(this->easyStyleRadioButton, 6, 2, 1, 2);
	this->sizeCalculationLabel = new QLabel("Size calculation:");
	this->rectCalculationRadioButton = new QRadioButton("Rect calculation");
	this->rectCalculationRadioButton->setChecked(true);
	sizeCalculationButtonGroup->addButton(this->rectCalculationRadioButton);
	sizeMeasureOptionWidgetLayout->addWidget(this->sizeCalculationLabel, 7, 0, 1, 2);
	sizeMeasureOptionWidgetLayout->addWidget(this->rectCalculationRadioButton, 7, 2, 1, 2);
	this->setLayout(sizeMeasureOptionWidgetLayout);
}

void SizeMeasureOptionWidget::bVisualizedSliderValueChanged(int value) {
	/*if (value == 0)this->sizeMeasureClass->setBVisualized(false);
	else this->sizeMeasureClass->setBVisualized(true);*/
	std::stringstream ss;
	std::string visualized;
	ss << value;
	ss >> visualized;
	ss.clear();
	ss.str("");
	this->cMyIni->SetValue("binocular_cameras", "bVisualized", visualized);
	this->cMyIni->WriteINI("SWQT.ini");
}

void SizeMeasureOptionWidget::bCollectedSliderValueChanged(int value) {
	/*if (value == 0)this->sizeMeasureClass->setBCollected(false);
	else this->sizeMeasureClass->setBCollected(true);*/
	std::stringstream ss;
	std::string collected;
	ss << value;
	ss >> collected;
	ss.clear();
	ss.str("");
	this->cMyIni->SetValue("binocular_cameras", "bCollected", collected);
	this->cMyIni->WriteINI("SWQT.ini");
}

void SizeMeasureOptionWidget::bTimeRecordedSliderValueChanged(int value) {
	/*if (value == 0)this->sizeMeasureClass->setBTimeRecorded(false);
	else this->sizeMeasureClass->setBTimeRecorded(true);*/
	std::stringstream ss;
	std::string timeRecorded;
	ss << value;
	ss >> timeRecorded;
	ss.clear();
	ss.str("");
	this->cMyIni->SetValue("binocular_cameras", "bTimeRecorded", timeRecorded);
	this->cMyIni->WriteINI("SWQT.ini");
}

void SizeMeasureOptionWidget::bmmSliderValueChanged(int value) {
	/*if (value == 0)this->sizeMeasureClass->setBmm(false);
	else this->sizeMeasureClass->setBmm(true);*/
	std::stringstream ss;
	std::string mm;
	ss << value;
	ss >> mm;
	ss.clear();
	ss.str("");
	this->cMyIni->SetValue("binocular_cameras", "bmm", mm);
	this->cMyIni->WriteINI("SWQT.ini");
}

void SizeMeasureOptionWidget::bAutoSliderValueChanged(int value) {
	/*if (value == 0)this->sizeMeasureClass->setBAuto(false);
	else this->sizeMeasureClass->setBAuto(true);*/
	std::stringstream ss;
	std::string autostr;
	ss << value;
	ss >> autostr;
	ss.clear();
	ss.str("");
	this->cMyIni->SetValue("binocular_cameras", "bAuto", autostr);
	this->cMyIni->WriteINI("SWQT.ini");
}

void SizeMeasureOptionWidget::setCMyIni(CMyINI* cMyIni) {
	this->cMyIni = cMyIni;
}

//void SizeMeasureOptionWidget::bPreciseSliderValueChanged(int value) {
//	/*if (value == 0)this->sizeMeasureClass->setBAuto(false);
//	else this->sizeMeasureClass->setBAuto(true);*/
//	std::stringstream ss;
//	std::string precise;
//	ss << value;
//	ss >> precise;
//	ss.clear();
//	ss.str("");
//	this->cMyIni->SetValue("binocular_cameras", "bPrecise", precise);
//	this->cMyIni->WriteINI("SWQT.ini");
//}