#pragma once
#include <QWidget>
#include "SWQTCanvas.h"

class RightPicWidget : public QWidget {
	Q_OBJECT
public:
	RightPicWidget(QImage* image, QWidget* parent = Q_NULLPTR);
	QImage* image = NULL;
	void closeEvent(QCloseEvent* e);
	SWQTCanvas* swqtCanvas;
	//SWQT* swqt;
	//void setSWQT(SWQT* swqt);
};