#pragma once

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <QImageReader>
//#include "ui_SWQT.h"
#include "SWQTCanvas.h"
#include <QSlider>
#include <QLineEdit>
#include "SWQTDock.h"
#include "CMyINI.h"
#include <QCameraInfo>
#include <QCameraViewfinder>
//#include "CameraShowDialog.h"
#include "RightPicWidget.h"
#include "SizeMeasureClass.h"
#include "CamerasShowWidget.h"

class SWQT : public QMainWindow
{
	Q_OBJECT

public:
	SWQT(QWidget *parent = Q_NULLPTR);
	QStatusBar* status;
	//QImage* target;
	QImage* src = NULL;
	QImage* right_src = NULL;
	QImage* processed = NULL;
	QImage* right_processed = NULL;
	//QImageReader* reader;
	SWQTCanvas* canvas;
	QDockWidget* dock;
	float picScale = 1;
	void setPicScale(float picScale);
	void resetState();
	void paintCanvas();
	void loadImage(QImage* image);
	SWQTDock* dockWidget;
	CMyINI* cMyIni = new CMyINI();
	void setBScale(bool bScale);
	void setBDraw(bool bDraw);
	QList<QCameraInfo> m_camera;
	QList<QCamera*> m_cameraList;
	QCamera* leftCamera;
	QCamera* rightCamera;
	QDialog* camerasSelectDialog;
	/*CameraShowDialog* leftCameraShowDialog;
	CameraShowDialog* rightCameraShowDialog;*/
	CamerasShowWidget* camerasShowWidget;
	RightPicWidget* rightPicWidget = NULL;
	SizeMeasureClass* sizeMeasureClass;
	cv::Mat someProcessDetailed(QImage& image);
private:
	//Ui::SWQTClass ui;
	QAction *openAPicAction;
	QAction *quitAction;
	QAction *drawAction;
	QAction *refreshImageAction;
	QAction *scaleAction;
	QAction *someProcessAction;
	QAction *settingsAction;
	QAction *selectDrawColorAction;
	QAction *clearAllShapesAction;
	QAction *openCamerasAction;
	QAction* openRightPicAction;
	QAction* measureAction;
	void openAPic();
	void refreshImage();
	void scale();
	void someProcess();
	void settings();
	void draw();
	void selectDrawColor();
	void clearAllShapes();
	void openCameras();
	void openRightPic();
	void measure();
	void captureToShow(int id, QImage image);
	void rightCaptureToShow(int id, QImage image);
};