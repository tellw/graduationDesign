#include "CamerasShowWidget.h"
#include <QCameraViewfinder>
#include <QHBoxLayout>

CamerasShowWidget::CamerasShowWidget(QCamera* leftCamera, QCamera* rightCamera, QWidget* parent) :QWidget(parent) {
	this->setWindowTitle(tr("Cameas"));
	QHBoxLayout* camerasShowWidgetLayout = new QHBoxLayout();
	QWidget* leftWidget = new QWidget(this);
	QWidget* rightWidget = new QWidget(this);
	this->leftCamera = leftCamera;
	this->rightCamera = rightCamera;
	QCameraViewfinder* leftViewfinder = new QCameraViewfinder(leftWidget);
	QCameraViewfinder* rightViewfinder = new QCameraViewfinder(rightWidget);
	this->leftCamera->setViewfinder(leftViewfinder);
	this->rightCamera->setViewfinder(rightViewfinder);
	this->leftCamera->start();
	this->rightCamera->start();
	QList<QCameraViewfinderSettings> leftViewSets = this->leftCamera->supportedViewfinderSettings();
	QList<QCameraViewfinderSettings> rightViewSets = this->rightCamera->supportedViewfinderSettings();
	this->leftCamera->setViewfinderSettings(leftViewSets[0]);
	this->rightCamera->setViewfinderSettings(rightViewSets[0]);
	/*leftViewfinder->setFixedSize(leftViewSets[0].resolution());
	rightViewfinder->setFixedSize(rightViewSets[0].resolution());*/
	leftWidget->setFixedSize(leftViewSets[0].resolution());
	rightWidget->setFixedSize(rightViewSets[0].resolution());
	/*QDebug(QtDebugMsg) << leftViewSets[0].resolution().width() << " " << leftViewSets[0].resolution().height() << "\n";
	QDebug(QtDebugMsg) << leftViewSets[1].resolution().width() << " " << leftViewSets[1].resolution().height() << "\n";*/
	camerasShowWidgetLayout->addWidget(leftWidget);
	camerasShowWidgetLayout->addWidget(rightWidget);
	this->setLayout(camerasShowWidgetLayout);
}

void CamerasShowWidget::closeEvent(QCloseEvent* e) {
	delete this->leftCamera;
	delete this->rightCamera;
	this->leftCamera = NULL;
	this->rightCamera = NULL;
}