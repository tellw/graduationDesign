#include "SettingsDialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QIntValidator>
#include <QDoubleValidator>
#include "calib.h"

SettingsDialog::SettingsDialog(CMyINI* cMyIni, SizeMeasureClass* sizeMeasureClass, QWidget* parent) :QDialog(parent) {
	QLabel* CLAHECoeffLabel = new QLabel("CLAHECoeff");
	QLabel* cannyHighThreshLabel = new QLabel("cannyHighThresh");
	QLabel* cannyLowThreshLabel = new QLabel("cannyLowThresh");
	QLabel* gaussianBlurKSizeLabel = new QLabel("gaussianBlurKSize");
	QLabel* cut_rateLabel = new QLabel("cut_rate");
	QLabel* xmlfileLabel = new QLabel("xmlfile");
	QLabel* colorBlockDisLabel = new QLabel("colorBlockDis");
	this->CLAHECoeffLE = new QLineEdit(QString::fromStdString(cMyIni->GetValue("process", "CLAHECoeff")));
	this->cannyHighThreshLE = new QLineEdit(QString::fromStdString(cMyIni->GetValue("process", "cannyHighThresh")));
	this->cannyLowThreshLE = new QLineEdit(QString::fromStdString(cMyIni->GetValue("process", "cannyLowThresh")));
	this->gaussianBlurKSizeLE = new QLineEdit(QString::fromStdString(cMyIni->GetValue("process", "gaussianBlurKSize")));
	this->cut_rateLE = new QLineEdit(QString::fromStdString(cMyIni->GetValue("binocular_cameras", "cut_rate")));
	this->xmlfileLE = new QLineEdit(QString::fromStdString(cMyIni->GetValue("binocular_cameras", "xmlfile")));
	this->colorBlockDisLE = new QLineEdit(QString::fromStdString(cMyIni->GetValue("process", "colorBlockDis")));
	QDoubleValidator* CLAHECoeffLEValidator = new QDoubleValidator(0, 100, 1, this->CLAHECoeffLE);
	CLAHECoeffLEValidator->setNotation(QDoubleValidator::StandardNotation);
	this->CLAHECoeffLE->setValidator(CLAHECoeffLEValidator);
	this->CLAHECoeffLE->setFixedWidth(27);
	QIntValidator* cannyHighThreshLEValidator = new QIntValidator(0, 255, this->cannyHighThreshLE);
	this->cannyHighThreshLE->setValidator(cannyHighThreshLEValidator);
	this->cannyHighThreshLE->setFixedWidth(27);
	QIntValidator* cannyLowThreshLEValidator = new QIntValidator(0, 255, this->cannyLowThreshLE);
	this->cannyLowThreshLE->setValidator(cannyLowThreshLEValidator);
	this->cannyLowThreshLE->setFixedWidth(27);
	QIntValidator* gaussianBlurKSizeLEValidator = new QIntValidator(1, 10, this->gaussianBlurKSizeLE);
	this->gaussianBlurKSizeLE->setValidator(gaussianBlurKSizeLEValidator);
	this->gaussianBlurKSizeLE->setFixedWidth(27);
	QDoubleValidator* cut_rateLEValidator = new QDoubleValidator(0, 0.5, 2, this->cut_rateLE);
	cut_rateLEValidator->setNotation(QDoubleValidator::StandardNotation);
	this->cut_rateLE->setValidator(cut_rateLEValidator);
	this->cut_rateLE->setFixedWidth(27);
	QIntValidator* colorBlockDisLEValidator = new QIntValidator(1, 765, this->colorBlockDisLE);
	this->colorBlockDisLE->setValidator(colorBlockDisLEValidator);
	this->colorBlockDisLE->setFixedWidth(27);
	this->saveButton = new QPushButton(tr("Save"));
	this->cancelButton = new QPushButton(tr("Cancel"));
	QGridLayout* settingsDialogLayout = new QGridLayout();
	settingsDialogLayout->addWidget(CLAHECoeffLabel, 0, 0);
	settingsDialogLayout->addWidget(this->CLAHECoeffLE, 0, 1);
	settingsDialogLayout->addWidget(cannyHighThreshLabel, 1, 0);
	settingsDialogLayout->addWidget(this->cannyHighThreshLE, 1, 1);
	settingsDialogLayout->addWidget(cannyLowThreshLabel, 2, 0);
	settingsDialogLayout->addWidget(this->cannyLowThreshLE, 2, 1);
	settingsDialogLayout->addWidget(gaussianBlurKSizeLabel, 3, 0);
	settingsDialogLayout->addWidget(this->gaussianBlurKSizeLE, 3, 1);
	settingsDialogLayout->addWidget(cut_rateLabel, 4, 0);
	settingsDialogLayout->addWidget(this->cut_rateLE, 4, 1);
	settingsDialogLayout->addWidget(xmlfileLabel, 5, 0);
	settingsDialogLayout->addWidget(this->xmlfileLE, 5, 1);
	settingsDialogLayout->addWidget(colorBlockDisLabel, 6, 0);
	settingsDialogLayout->addWidget(this->colorBlockDisLE, 6, 1);
	settingsDialogLayout->addWidget(this->saveButton, 7, 0);
	settingsDialogLayout->addWidget(this->cancelButton, 7, 1);
	this->setLayout(settingsDialogLayout);
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveButtonClicked()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

	this->cMyIni = cMyIni;
	this->sizeMeasureClass = sizeMeasureClass;
}

void SettingsDialog::setCMyIni(CMyINI* cMyIni) {
	this->cMyIni = cMyIni;
}

void SettingsDialog::saveButtonClicked() {
	//std::stringstream ss;
	if(this->CLAHECoeffLE->text().toStdString() != this->cMyIni->GetValue("process", "CLAHECoeff"))
		this->cMyIni->SetValue("process", "CLAHECoeff", this->CLAHECoeffLE->text().toStdString());
	if (this->cannyHighThreshLE->text().toStdString() != this->cMyIni->GetValue("process", "cannyHighThresh"))
		this->cMyIni->SetValue("process", "cannyHighThresh", this->cannyHighThreshLE->text().toStdString());
	if (this->cannyLowThreshLE->text().toStdString() != this->cMyIni->GetValue("process", "cannyLowThresh"))
	this->cMyIni->SetValue("process", "cannyLowThresh", this->cannyLowThreshLE->text().toStdString());
	if (this->gaussianBlurKSizeLE->text().toStdString() != this->cMyIni->GetValue("process", "gaussianBlurKSize"))
	this->cMyIni->SetValue("process", "gaussianBlurKSize", this->gaussianBlurKSizeLE->text().toStdString());
	if (this->cut_rateLE->text().toStdString() != this->cMyIni->GetValue("binocular_cameras", "cut_rate")) {
		this->cMyIni->SetValue("binocular_cameras", "cut_rate", this->cut_rateLE->text().toStdString());
		this->sizeMeasureClass->cut_rate = this->cut_rateLE->text().toDouble();
	}
	if (this->xmlfileLE->text().toStdString() != this->cMyIni->GetValue("binocular_cameras", "xmlfile")) {
		this->cMyIni->SetValue("binocular_cameras", "xmlfile", this->xmlfileLE->text().toStdString());
		delete this->sizeMeasureClass->calibrator;
		this->sizeMeasureClass->calibrator = NULL;
		this->sizeMeasureClass->calibrator = new stereo_calibrator(this->xmlfileLE->text().toStdString(), stereo_calibrator::init_mode::STATIC_PARAM);
	}
	if (this->colorBlockDisLE->text().toStdString() != this->cMyIni->GetValue("process", "colorBlockDis"))
		this->cMyIni->SetValue("process", "colorBlockDis", this->colorBlockDisLE->text().toStdString());
	this->cMyIni->WriteINI("SWQT.ini");
	this->close();
}

//void SettingsDialog::cancelButtonClicked() {
//
//}
