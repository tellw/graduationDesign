#pragma once
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include "CMyINI.h"
#include "SizeMeasureClass.h"

class SettingsDialog :public QDialog {
	Q_OBJECT

public:
	SettingsDialog(CMyINI* cMyIni, SizeMeasureClass* sizeMeasureClass, QWidget* parent = Q_NULLPTR);
	void setCMyIni(CMyINI* cMyIni);
	CMyINI* cMyIni;
	SizeMeasureClass* sizeMeasureClass;
	/*QWidget* parent;*/
protected:
	QPushButton* saveButton;
	QPushButton* cancelButton;
	QLineEdit* CLAHECoeffLE;
	QLineEdit* cannyHighThreshLE;
	QLineEdit* cannyLowThreshLE;
	QLineEdit* gaussianBlurKSizeLE;
	QLineEdit* cut_rateLE;
	QLineEdit* xmlfileLE;
	QLineEdit* colorBlockDisLE;
protected slots:
	void saveButtonClicked();
	//void cancelButtonClicked();
};