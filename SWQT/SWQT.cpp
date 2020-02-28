#include "SWQT.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "SWQTDock.h"
#include <QDoubleValidator>
#include <QScrollArea>
#include <string>
#include "QtOpencvImageConvert.h"
#include <opencv2/core/core.hpp>
#include <vector>
#include "SettingsDialog.h"
#include <QColorDialog>
#include <QCamera>
#include <QAction>
#include <QStatusBar>
#include <QToolBar>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QCameraImageCapture>

std::string dirname(std::string& filename) {
	int i = filename.size() - 1;
	for (; filename[i] != '/'; i--);
	return filename.substr(0, i);
}

int otsu(cv::Mat &img) {
	float histogram[256] = { 0 };
	for (int i = 0; i < img.rows; i++) {
		const unsigned char* p = (const unsigned char*)img.ptr(i);
		for (int j = 0; j < img.cols; j++)histogram[p[j]]++;
	}
	float avgValue = 0;
	int numPixel = img.cols*img.rows;
	for (int i = 0; i < 256; i++) {
		histogram[i] = histogram[i] / numPixel;
		avgValue += i * histogram[i];
	}
	int threshold = 0;
	float gmax = 0;
	float wk = 0, uk = 0;
	for (int i = 0; i < 256; i++) {
		wk += histogram[i];
		uk += i * histogram[i];
		float ut = avgValue * wk - uk;
		float g = ut * ut / (wk*(1 - wk));
		if (g > gmax) {
			gmax = g;
			threshold = i;
		}
	}
	return threshold;
}

cv::Mat colorBlock(cv::Mat &img, int dis) {
	int img_width = img.cols;
	int img_height = img.rows;
	int prePoints[4][2] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1} };
	//std::map<int, std::vector<int>> colorRegion;
	int regionCount = 0;
	std::vector<cv::Vec3b> regionColors;
	std::vector<uchar>grayRegionColors;
	std::vector<std::vector<int>> pixRegionNo(img_height, vector<int>(img_width, 0));
	if (img.type() == CV_8UC3) {
		for (int i = 0; i < img_height; i++) {
			for (int j = 0; j < img_width; j++) {
				bool bAdded = false;
				for (int k = 0; k < 4; k++) {
					if (i + prePoints[k][0] < 0 || j + prePoints[k][1] < 0 || j+prePoints[k][1] >= img_width)continue;
					int real_dis = 0;
					for (int l = 0; l < 3; l++) {
						real_dis += int(img.at<cv::Vec3b>(i, j)[l]) - int(img.at<cv::Vec3b>(i + prePoints[k][0], j + prePoints[k][1])[l]);
					}
					if (real_dis <= dis && real_dis >= -dis) {
						pixRegionNo[i][j] = pixRegionNo[i + prePoints[k][0]][j + prePoints[k][1]];
						bAdded = true;
						break;
					}
				}
				if (!bAdded) {
					pixRegionNo[i][j] = regionCount;
					regionColors.push_back(img.at<cv::Vec3b>(i, j));
					regionCount++;
				}
			}
		}
		for (int i = 0; i < img_height; i++) {
			for (int j = 0; j < img_width; j++) {
				img.at<cv::Vec3b>(i, j) = regionColors[pixRegionNo[i][j]];
			}
		}
	}
	else {
		for (int i = 0; i < img_height; i++) {
			for (int j = 0; j < img_width; j++) {
				bool bAdded = false;
				for (int k = 0; k < 4; k++) {
					if (i + prePoints[k][0] < 0 || j + prePoints[k][1] < 0 || j + prePoints[k][1] >= img_width)continue;
					int real_dis = int(img.at<uchar>(i, j)) - int(img.at<uchar>(i + prePoints[k][0], j + prePoints[k][1]));
					if (real_dis <= dis / 3 && real_dis >= -dis / 3) {
						pixRegionNo[i][j] = pixRegionNo[i + prePoints[k][0]][j + prePoints[k][1]];
						bAdded = true;
						break;
					}
				}
				if (!bAdded) {
					pixRegionNo[i][j] = regionCount;
					grayRegionColors.push_back(img.at<uchar>(i, j));
					regionCount++;
				}
			}
		}
		for (int i = 0; i < img_height; i++) {
			for (int j = 0; j < img_width; j++) {
				img.at<uchar>(i, j) = grayRegionColors[pixRegionNo[i][j]];
			}
		}
	}
	return img;
}

SWQT::SWQT(QWidget *parent)
	: QMainWindow(parent)
{
	this->setWindowTitle("SWQT");
	//this->resize(800, 500);
	this->setMinimumWidth(700);
	this->setMinimumHeight(500);
	this->showMaximized();
	this->status = this->statusBar();
	this->status->showMessage(tr("Starting successfully"), 5000);
	this->setWindowIcon(QIcon("imgs/SWQT.png"));

	this->cMyIni->ReadINI("SWQT.ini");

	QScrollArea* scroll = new QScrollArea(this);
	this->canvas = new SWQTCanvas();
	scroll->setWidget(this->canvas);
	scroll->setWidgetResizable(false);
	this->setCentralWidget(scroll);
	scroll->setAlignment(Qt::AlignCenter);
	
	this->dock = new QDockWidget(this);
	this->addDockWidget(Qt::RightDockWidgetArea, this->dock);
	//this->dock->setGeometry(0, 0, 200, this->dock->geometry().height());

	QScrollArea* dockScroll = new QScrollArea(this->dock);
	this->dockWidget = new SWQTDock(this->dock);
	dockScroll->setWidget(this->dockWidget);
	dockScroll->setWidgetResizable(false);
	this->dock->setWidget(dockScroll);
	this->canvas->setPicScaleWidget(this->dockWidget->picScaleWidget);
	this->canvas->setDrawRadiosWidget(this->dockWidget->drawRadiosWidget);
	this->dockWidget->selectionStatusWidget->setCanvas(this->canvas);

	this->sizeMeasureClass = new SizeMeasureClass(this->cMyIni, this->dockWidget->sizeMeasureOptionWidget);

	QMenu* file = this->menuBar()->addMenu(tr("&File"));
	QMenu* process = this->menuBar()->addMenu(tr("&Process"));
	QMenu* settings = this->menuBar()->addMenu(tr("&Settings"));
	QMenu* binocularCameras = this->menuBar()->addMenu(tr("&Binocular Cameras"));

	openAPicAction = new QAction(QIcon("imgs/openAPic.png"), tr("&Open a picture"), this);
	openAPicAction->setShortcut(tr("Ctrl+O"));
	openAPicAction->setStatusTip(tr("open a picture"));
	openAPicAction->setToolTip(tr("open a picture"));
	connect(openAPicAction, &QAction::triggered, this, &SWQT::openAPic);
	file->addAction(openAPicAction);

	quitAction = new QAction(QIcon("imgs/quit.jpg"), tr("&Quit"), this);
	quitAction->setShortcut(tr("Ctrl+Q"));
	quitAction->setStatusTip(tr("quit"));
	quitAction->setToolTip(tr("quit"));
	connect(quitAction, &QAction::triggered, this, &QMainWindow::close);
	file->addAction(quitAction);

	drawAction = new QAction(QIcon("imgs/draw.png"), tr("&Draw"), this);
	drawAction->setShortcut(tr("Ctrl+D"));
	drawAction->setStatusTip(tr("draw"));
	drawAction->setToolTip("draw");
	connect(drawAction, &QAction::triggered, this, &SWQT::draw);
	file->addAction(drawAction);

	openRightPicAction = new QAction(QIcon("imgs/openRightPic.jpg"), tr("&Open right picture"), this);
	openRightPicAction->setShortcut(tr("Ctrl+2"));
	openRightPicAction->setStatusTip(tr("open right picture"));
	openRightPicAction->setToolTip(tr("open right picture"));
	connect(openRightPicAction, &QAction::triggered, this, &SWQT::openRightPic);
	file->addAction(openRightPicAction);

	refreshImageAction = new QAction(QIcon("imgs/refreshImage.jpg"), tr("&Refresh image"), this);
	refreshImageAction->setShortcut(tr("Ctrl+R"));
	refreshImageAction->setStatusTip(tr("refresh image"));
	refreshImageAction->setToolTip(tr("refresh image"));
	connect(refreshImageAction, &QAction::triggered, this, &SWQT::refreshImage);
	process->addAction(refreshImageAction);

	scaleAction = new QAction(QIcon("imgs/scale.png"), tr("&Scale image"), this);
	scaleAction->setShortcut(tr("Ctrl+S"));
	scaleAction->setStatusTip(tr("scale image"));
	scaleAction->setToolTip(tr("scale image"));
	connect(scaleAction, &QAction::triggered, this, &SWQT::scale);
	process->addAction(scaleAction);

	someProcessAction = new QAction(QIcon("imgs/someProcess.jpg"), tr("&Show some process"), this);
	someProcessAction->setShortcut(tr("Ctrl+P"));
	someProcessAction->setStatusTip(tr("show some process"));
	someProcessAction->setToolTip(tr("show some process"));
	connect(someProcessAction, &QAction::triggered, this, &SWQT::someProcess);
	process->addAction(someProcessAction);

	settingsAction = new QAction(QIcon("imgs/settings.jpg"), tr("&Settings"), this);
	settingsAction->setShortcut(tr("Ctrl+E"));
	settingsAction->setStatusTip(tr("settings"));
	settingsAction->setToolTip(tr("settings"));
	connect(settingsAction, &QAction::triggered, this, &SWQT::settings);
	settings->addAction(settingsAction);

	selectDrawColorAction = new QAction(QIcon("imgs/selectDrawColor.png"), tr("&Select draw color"), this);
	selectDrawColorAction->setShortcut(tr("Ctrl+L"));
	selectDrawColorAction->setStatusTip(tr("select draw color"));
	selectDrawColorAction->setToolTip(tr("select draw color"));
	connect(selectDrawColorAction, &QAction::triggered, this, &SWQT::selectDrawColor);
	file->addAction(selectDrawColorAction);

	clearAllShapesAction = new QAction(QIcon("imgs/clearAllShapes.png"), tr("&Clear all shapes"), this);
	clearAllShapesAction->setShortcut(tr("Ctrl+1"));
	clearAllShapesAction->setStatusTip(tr("clear all shapes"));
	clearAllShapesAction->setToolTip(tr("clear all shapes"));
	connect(clearAllShapesAction, &QAction::triggered, this, &SWQT::clearAllShapes);
	file->addAction(clearAllShapesAction);

	openCamerasAction = new QAction(QIcon("imgs/openBinocularCameras.jpg"), tr("&Open binocular cameras"), this);
	openCamerasAction->setShortcut(tr("Ctrl+B"));
	openCamerasAction->setStatusTip(tr("open binocular cameras"));
	openCamerasAction->setToolTip(tr("open binocular cameras"));
	connect(openCamerasAction, &QAction::triggered, this, &SWQT::openCameras);
	binocularCameras->addAction(openCamerasAction);

	measureAction = new QAction(QIcon("imgs/measure.png"), tr("&Measure"), this);
	measureAction->setShortcut(tr("Ctrl+M"));
	measureAction->setStatusTip(tr("measure"));
	measureAction->setToolTip(tr("measure"));
	connect(measureAction, &QAction::triggered, this, &SWQT::measure);
	binocularCameras->addAction(measureAction);

	QToolBar *toolBarFile = addToolBar(tr("File"));
	toolBarFile->addAction(openAPicAction);
	toolBarFile->addAction(drawAction);
	toolBarFile->addAction(selectDrawColorAction);
	toolBarFile->addAction(clearAllShapesAction);
	toolBarFile->addAction(openRightPicAction);
	this->addToolBar(Qt::LeftToolBarArea, toolBarFile);
	QToolBar *toolBarProcess = addToolBar(tr("Process"));
	toolBarProcess->addAction(refreshImageAction);
	toolBarProcess->addAction(scaleAction);
	toolBarProcess->addAction(someProcessAction);
	this->addToolBar(Qt::LeftToolBarArea, toolBarProcess);
	QToolBar *toolBarSettings = addToolBar(tr("Settings"));
	toolBarSettings->addAction(settingsAction);
	this->addToolBar(Qt::LeftToolBarArea, toolBarSettings);
	QToolBar *toolBarBinocularCameras = addToolBar(tr("Binocular Cameras"));
	toolBarBinocularCameras->addAction(openCamerasAction);
	toolBarBinocularCameras->addAction(measureAction);
	this->addToolBar(Qt::LeftToolBarArea, toolBarBinocularCameras);

	int red, green, blue;
	std::stringstream ss;
	ss << this->cMyIni->GetValue("file", "drawColorRed");
	ss >> red;
	ss.clear();
	ss << this->cMyIni->GetValue("file", "drawColorGreen");
	ss >> green;
	ss.clear();
	ss << this->cMyIni->GetValue("file", "drawColorBlue");
	ss >> blue;
	ss.clear();
	ss.str("");
	this->canvas->setDrawColor(QColor(red, green, blue));
}

void SWQT::openAPic() {
	this->setBScale(false);
	this->setBDraw(false);
	QString picFile = QFileDialog::getOpenFileName(this, tr("open a picture"), QString::fromStdString(this->cMyIni->GetValue("file", "lastOpenDir")), "All files(*.*);;Image files(*.bmp *.jpg *.pbm *.png *.ppm *.xbm *.xpm)");
	//QDebug(QtDebugMsg) << "filename:" << picFile << "\n";
	if (picFile != "") {

		/*this->reader = new QImageReader(picFile);
		if (this->reader->canRead()) {
			this->target = &this->reader->read();
			this->center->loadImage(this->target);
		}
		else {
			QByteArray ba = this->reader->errorString().toLatin1();
			char *es = ba.data();
			QMessageBox::warning(this, tr("warning about reading the picture"), tr(es));
		}*/
		this->cMyIni->SetValue("file", "lastOpenDir", dirname(picFile.toStdString()));
		this->cMyIni->WriteINI("SWQT.ini");
		this->resetState();
		//this->canvas->setEnabled(false);
		//this->target = new QImage(picFile);
		this->src = new QImage(picFile);
		this->loadImage(this->src);
	}
}

void SWQT::setPicScale(float picScale) {
	this->picScale = picScale;
}

void SWQT::resetState() {
	this->src = NULL;
	this->canvas->resetState();
}

void SWQT::paintCanvas() {
	this->canvas->adjustSize();
	this->canvas->update();
}

void SWQT::refreshImage() {
	this->setBScale(false);
	this->setBDraw(false);
	if (this->src != NULL) {
		this->canvas->setPicScale(this->dockWidget->picScaleWidget->picScale);
		this->canvas->loadImage(this->src);
	}
}

void SWQT::loadImage(QImage* image) {
	this->status->showMessage(tr("loading picture"), 5000);
	this->canvas->loadImage(image);
	//this->canvas->setEnabled(true);
	this->paintCanvas();
	//this->canvas->setFocus();
	//this->center->loadImage(picFile);
}

void SWQT::scale() {
	this->setBScale(true);
	this->setBDraw(false);
}

void SWQT::someProcess() {
	this->setBScale(false);
	this->setBDraw(false);
	if (this->src != NULL) {
		cv::Mat toProcess = someProcessDetailed(*this->src);
		this->canvas->setPicScale(this->dockWidget->picScaleWidget->picScale);
		this->canvas->loadImage(&cvMat2QImage(toProcess));
	}
}

void SWQT::settings() {
	this->setBScale(false);
	this->setBDraw(false);
	SettingsDialog* settingsDialog = new SettingsDialog(this->cMyIni, this->sizeMeasureClass, this);
	settingsDialog->exec();
}

void SWQT::draw() {
	this->setBScale(false);
	this->setBDraw(true);
}

void SWQT::setBScale(bool bScale) {
	this->canvas->bScale = bScale;
	if (bScale == true)this->dockWidget->selectionStatusWidget->bScaleSlider->setValue(1);
	else this->dockWidget->selectionStatusWidget->bScaleSlider->setValue(0);
}

void SWQT::setBDraw(bool bDraw) {
	this->canvas->bDraw = bDraw;
	if (bDraw == true)this->dockWidget->selectionStatusWidget->bDrawSlider->setValue(1);
	else this->dockWidget->selectionStatusWidget->bDrawSlider->setValue(0);
}

void SWQT::selectDrawColor() {
	this->setBScale(false);
	//this->setBDraw(false);
	QColorDialog colorDialog(this);
	colorDialog.setWindowTitle("Select Draw Color: ");
	colorDialog.setCurrentColor(Qt::red);
	if (colorDialog.exec() == QColorDialog::Accepted) {
		QColor drawColor = colorDialog.selectedColor();
		this->canvas->setDrawColor(drawColor);
		int red = drawColor.red();
		int green = drawColor.green();
		int blue = drawColor.blue();
		std::string drawColorRed, drawColorGreen, drawColorBlue;
		std::stringstream ss;
		ss << red;
		ss >> drawColorRed;
		ss.clear();
		ss << green;
		ss >> drawColorGreen;
		ss.clear();
		ss << blue;
		ss >> drawColorBlue;
		ss.clear();
		ss.str("");
		this->cMyIni->SetValue("file", "drawColorRed", drawColorRed);
		this->cMyIni->SetValue("file", "drawColorGreen", drawColorGreen);
		this->cMyIni->SetValue("file", "drawColorBlue", drawColorBlue);
		this->cMyIni->WriteINI("SWQT.ini");
	}
}

void SWQT::clearAllShapes() {
	this->setBScale(false);
	//this->setBDraw(false);
	this->canvas->clearAllShapes();
	this->paintCanvas();
}

void SWQT::openCameras() {
	this->setBDraw(false);
	this->setBScale(false);
	this->m_camera.clear();
	this->m_cameraList.clear();
	qDeleteAll(this->m_cameraList);
	foreach(QCameraInfo info, QCameraInfo::availableCameras()) {
		this->m_camera.append(info);
	}
	this->camerasSelectDialog = new QDialog(this);
	this->camerasSelectDialog->setMinimumWidth(950);
	this->camerasSelectDialog->setWindowTitle(tr("Select your disired cameras"));
	QGridLayout* camerasSelectDialogLayout = new QGridLayout();
	QLabel* leftLabel = new QLabel(tr("Left"));
	QLabel* rightLabel = new QLabel(tr("Right"));
	QButtonGroup* leftCameraButtonGroup = new QButtonGroup(this->camerasSelectDialog);
	QButtonGroup* rightCameraButtonGroup = new QButtonGroup(this->camerasSelectDialog);
	camerasSelectDialogLayout->addWidget(leftLabel, 1, 0);
	camerasSelectDialogLayout->addWidget(rightLabel, 2, 0);
	//std::vector<QCheckBox*> optionalCameraCheckBoxes;
	for (int i = 0; i < this->m_camera.count(); i++) {
		QWidget* wid = new QWidget(this->camerasSelectDialog);
		wid->setFixedSize(400, 300);
		QCamera* camera = new QCamera(m_camera.at(i));
		QCameraViewfinder* viewfinder = new QCameraViewfinder(wid);
		viewfinder->setFixedSize(wid->size());
		camera->setViewfinder(viewfinder);
		this->m_cameraList << camera;
		this->m_cameraList.at(i)->start();
		camerasSelectDialogLayout->addWidget(wid, 0, i + 1);
		QRadioButton* optionalLeftCameraRadioButton = new QRadioButton(this->m_camera.at(i).description());
		leftCameraButtonGroup->addButton(optionalLeftCameraRadioButton, i);
		QRadioButton* optionalRightCameraRadioButton = new QRadioButton(this->m_camera.at(i).description());
		rightCameraButtonGroup->addButton(optionalRightCameraRadioButton, i);
		if (i == 0) {
			optionalLeftCameraRadioButton->setChecked(true);
			optionalRightCameraRadioButton->setChecked(true);
		}
		camerasSelectDialogLayout->addWidget(optionalLeftCameraRadioButton, 1, i+1);
		camerasSelectDialogLayout->addWidget(optionalRightCameraRadioButton, 2, i+1);
	}
	QDialogButtonBox* button = new QDialogButtonBox(this->camerasSelectDialog);
	button->addButton(tr("OK"), QDialogButtonBox::YesRole);
	button->addButton(tr("Cancel"), QDialogButtonBox::NoRole);
	/*connect(button, &QDialogButtonBox::accepted, this, &SWQT::openBinocularCameras);
	connect(button, &QDialogButtonBox::rejected, this->camerasSelectDialog, &QDialog::close);*/
	connect(button, SIGNAL(accepted()), this->camerasSelectDialog, SLOT(accept()));
	connect(button, SIGNAL(rejected()), this->camerasSelectDialog, SLOT(reject()));
	camerasSelectDialogLayout->addWidget(button, 3, 0, 1, -1);
	this->camerasSelectDialog->setLayout(camerasSelectDialogLayout);
	auto res = this->camerasSelectDialog->exec();
	for (int i = 0; i < this->m_cameraList.count(); i++)this->m_cameraList.at(i)->stop();
	if (res == QDialog::Accepted) {
		int leftCameraId = leftCameraButtonGroup->checkedId();
		int rightCameraId = rightCameraButtonGroup->checkedId();
		/*for (int i = 0; i < this->m_camera.count(); i++) {
			if(i == leftCameraId)this->leftCamera = this->m_cameraList.at(leftCameraId);
			else if(i == rightCameraId)this->rightCamera = this->m_cameraList.at(rightCameraId);
			else delete this->m_cameraList.at(i);
		}*/
		cv::VideoCapture left_cap(leftCameraId);
		cv::VideoCapture right_cap(rightCameraId);
		cv::Mat left_frame;
		cv::Mat right_frame;
		left_cap >> left_frame;
		right_cap >> right_frame;
		cv::imshow("left_frame", left_frame);
		cv::imshow("right_frame", right_frame);
		this->m_cameraList.clear();
		qDeleteAll(this->m_cameraList);
		this->leftCamera = new QCamera(m_camera.at(leftCameraId));
		this->rightCamera = new QCamera(m_camera.at(rightCameraId));
		this->camerasShowWidget = new CamerasShowWidget(this->leftCamera, this->rightCamera);
		/*this->rightCameraShowDialog = new CamerasShowDialog(this->rightCamera, tr("Right Camera"));*/
		this->camerasShowWidget->show();
		m_camera.clear();
	}
	else {
		this->m_camera.clear();
		this->m_cameraList.clear();
		qDeleteAll(this->m_cameraList);
	}
	/*for (int i = 0; i < this->m_camera.count(); i++) {
		QDialog* d = new QDialog(this);
		d->setFixedWidth(500);
		d->setFixedHeight(500);
		QCamera* camera = new QCamera(m_camera.at(i));
		QCameraViewfinder* viewfinder = new QCameraViewfinder(d);
		viewfinder->setFixedSize(d->size());
		camera->setViewfinder(viewfinder);
		d->setWindowTitle(m_camera.at(i).description());
		m_cameraList << camera;
		m_cameraList.at(i)->start();
		d->show();
	}*/
}

void SWQT::openRightPic() {
	this->setBScale(false);
	this->setBDraw(false);
	QString picFile = QFileDialog::getOpenFileName(this, tr("open a right picture"), QString::fromStdString(this->cMyIni->GetValue("file", "lastOpenRightDir")), "All files(*.*);;Image files(*.bmp *.jpg *.pbm *.png *.ppm *.xbm *.xpm)");
	if (picFile != "") {
		this->cMyIni->SetValue("file", "lastOpenRightDir", dirname(picFile.toStdString()));
		this->cMyIni->WriteINI("SWQT.ini");
		this->right_src = new QImage(picFile);
		if (this->rightPicWidget != NULL) {
			if (this->rightPicWidget->image != NULL)this->rightPicWidget->close();
			delete this->rightPicWidget;
			this->rightPicWidget = NULL;
		}
		this->rightPicWidget = new RightPicWidget(this->right_src);
		//this->rightPicWidget->loadImage(this->right_src);
		this->rightPicWidget->show();
	}
}

void SWQT::measure() {
	if (this->dockWidget->getRadiosWidget->picRadioButton->isChecked()) {
		if (this->rightPicWidget == NULL || this->rightPicWidget->image == NULL || this->src == NULL) {
			QMessageBox message(QMessageBox::Information, tr("Information"), tr("Please get two pictures"), QMessageBox::Yes, this);
			message.exec();
		}
		else {
			cv::Mat left = someProcessDetailed(*this->src);
			cv::Mat right = someProcessDetailed(*this->right_src);
			this->canvas->loadImage(&cvMat2QImage(left));
			//this->rightPicWidget->swqtCanvas->loadImage(&cvMat2QImage(right));
			this->sizeMeasureClass->detect(left, right, QImage2cvMat(*this->src), QImage2cvMat(*this->right_src));
		}
	}
	else if (this->camerasShowWidget->leftCamera == NULL || this->camerasShowWidget->rightCamera == NULL) {
		QMessageBox message(QMessageBox::Information, tr("Information"), tr("Please start your binocular cameras"), QMessageBox::Yes, this);
		message.exec();
	}
	else {
		QCameraImageCapture* leftCameraCapture = new QCameraImageCapture(this->leftCamera);
		QCameraImageCapture* rightCameraCapture = new QCameraImageCapture(this->rightCamera);
		connect(leftCameraCapture, &QCameraImageCapture::imageCaptured, this, &SWQT::captureToShow);
		connect(rightCameraCapture, &QCameraImageCapture::imageCaptured, this, &SWQT::rightCaptureToShow);
		leftCameraCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
		rightCameraCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
		this->leftCamera->setCaptureMode(QCamera::CaptureStillImage);
		this->rightCamera->setCaptureMode(QCamera::CaptureStillImage);
		if (this->dockWidget->getRadiosWidget->captureSelectedPicRadioButton->isChecked()) {

		}
		else {
			//time_t old_time = time(NULL);
			leftCameraCapture->capture();
			rightCameraCapture->capture();
			//while (true) {
			//	
			//	//old_time = time(NULL);
			//	cv::Mat left = someProcessDetailed(*this->src);
			//	cv::Mat right = someProcessDetailed(*this->right_src);
			//	this->canvas->loadImage(&cvMat2QImage(left));
			//	this->sizeMeasureClass->detect(left, right, QImage2cvMat(*this->src), QImage2cvMat(*this->right_src));
			//}
		}
	}
}

cv::Mat SWQT::someProcessDetailed(QImage& image) {
	cv::Mat toProcess = QImage2cvMat(image);
	std::stringstream ss;
	if (toProcess.type() == CV_8UC4)
		cv::cvtColor(toProcess, toProcess, cv::COLOR_BGRA2BGR);
	if (this->dockWidget->someProcessWidget->grayCheckBox->checkState() == Qt::Checked) {
		cv::cvtColor(toProcess, toProcess, cv::COLOR_BGR2GRAY);
	}
	if (this->dockWidget->someProcessWidget->gaussianBlurCheckBox->checkState() == Qt::Checked) {
		int gaussianBlurKSize;
		ss << this->cMyIni->GetValue("process", "gaussianBlurKSize");
		ss >> gaussianBlurKSize;
		cv::GaussianBlur(toProcess, toProcess, cv::Size(gaussianBlurKSize, gaussianBlurKSize), 1, 1);
	}
	if (this->dockWidget->someProcessWidget->CLAHECheckBox->checkState() == Qt::Checked) {
		ss.clear();
		ss.str("");
		float CLAHECoeff;
		ss << this->cMyIni->GetValue("process", "CLAHECoeff");
		ss >> CLAHECoeff;
		auto clahe = cv::createCLAHE(CLAHECoeff);
		if (toProcess.type() == CV_8UC1) {
			clahe->apply(toProcess, toProcess);
		}
		else if (toProcess.type() == CV_8UC3) {
			cv::Mat clahe_img;
			cv::cvtColor(toProcess, clahe_img, cv::COLOR_BGR2Lab);
			/*std::vector<cv::Mat> channels(3);
			cv::split(toProcess, channels);
			for (int i = 0; i < 3; i++) {
				cv::Mat dst;
				clahe->apply(channels[i], dst);
				dst.copyTo(channels[i]);
			}
			cv::merge(channels, toProcess);*/
			std::vector<cv::Mat> channels(3);
			cv::split(clahe_img, channels);
			cv::Mat dst;
			clahe->apply(channels[0], dst);
			dst.copyTo(channels[0]);
			cv::merge(channels, clahe_img);
			cv::cvtColor(clahe_img, toProcess, cv::COLOR_Lab2BGR);
		}
	}
	if (this->dockWidget->someProcessWidget->colorBlockCheckBox->checkState() == Qt::Checked) {
		int colorBlockDis;
		ss << this->cMyIni->GetValue("process", "colorBlockDis");
		ss >> colorBlockDis;
		toProcess = colorBlock(toProcess, colorBlockDis);
	}
	if (this->dockWidget->someProcessWidget->sharpenCheckBox->checkState() == Qt::Checked) {
		float laplacian[] = { 0.f, -1.f, 0.f, -1.f, 5.f, -1.f, 0.f, -1.f, 0.f };
		auto kernel = cv::Mat(3, 3, CV_32F, laplacian);
		cv::filter2D(toProcess, toProcess, -1, kernel);
	}
	if (this->dockWidget->someProcessWidget->laplacianCheckBox->checkState() == Qt::Checked) {
		float laplacian[] = { 0.f, -1.f, 0.f, -1.f, 4.f, -1.f, 0.f, -1.f, 0.f };
		auto kernel = cv::Mat(3, 3, CV_32F, laplacian);
		cv::filter2D(toProcess, toProcess, -1, kernel);
	}
	if (this->dockWidget->someProcessWidget->OTSUCheckBox->checkState() == Qt::Checked) {
		if (toProcess.type() == CV_8UC1) {
			int thresh = otsu(toProcess);
			cv::threshold(toProcess, toProcess, thresh, 255, cv::THRESH_BINARY);
		}
		else if (toProcess.type() == CV_8UC3) {
			std::vector<cv::Mat> channels(3);
			cv::split(toProcess, channels);
			for (int i = 0; i < 3; i++) {
				cv::Mat dst;
				int thresh = otsu(channels[i]);
				cv::threshold(channels[i], dst, thresh, 255, cv::THRESH_BINARY);
				dst.copyTo(channels[i]);
			}
			cv::merge(channels, toProcess);
		}
	}
	if (this->dockWidget->someProcessWidget->cannyCheckBox->checkState() == Qt::Checked) {
		ss.clear();
		ss.str("");
		int cannyLowThresh, cannyHighThresh;
		ss << this->cMyIni->GetValue("process", "cannyLowThresh");
		ss >> cannyLowThresh;
		ss.clear();
		ss << this->cMyIni->GetValue("process", "cannyHighThresh");
		ss >> cannyHighThresh;
		if (toProcess.type() == CV_8UC1)
			cv::Canny(toProcess, toProcess, cannyLowThresh, cannyHighThresh);
		else if (toProcess.type() == CV_8UC3) {
			std::vector<cv::Mat> channels(3);
			cv::split(toProcess, channels);
			for (int i = 0; i < 3; i++) {
				cv::Mat dst;
				cv::Canny(channels[i], dst, cannyLowThresh, cannyHighThresh);
				dst.copyTo(channels[i]);
			}
			cv::merge(channels, toProcess);
		}
	}
	return toProcess;
}

void SWQT::captureToShow(int id, QImage image) {
	this->src = new QImage(image);
	this->canvas->loadImage(this->src);
}

void SWQT::rightCaptureToShow(int id, QImage image) {
	this->right_src = new QImage(image);
	if (this->rightPicWidget != NULL) {
		if (this->rightPicWidget->image != NULL)this->rightPicWidget->close();
		delete this->rightPicWidget;
		this->rightPicWidget = NULL;
	}
	this->rightPicWidget = new RightPicWidget(this->right_src);
	//this->rightPicWidget->loadImage(this->right_src);
	this->rightPicWidget->show();
}