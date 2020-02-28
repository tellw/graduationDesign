#include "CameraShowDialog.h"
#include <QCameraViewfinder>
#include <QCameraViewfinderSettings>
#include <QDebug>

CameraShowDialog::CameraShowDialog(QCamera* camera, QString& title, QWidget* parent) :QDialog(parent) {
	/*this->setModal(false);
	this->setWindowModality(Qt::NonModal);*/
	QCameraViewfinder* viewfinder = new QCameraViewfinder(this);
	this->camera = camera;
	this->camera->setViewfinder(viewfinder);
	this->camera->start();
	QList<QCameraViewfinderSettings> ViewSets = this->camera->supportedViewfinderSettings();
	//QDebug(QtDebugMsg) << ViewSets.length() << "\n";
	this->camera->setViewfinderSettings(ViewSets[0]);
	viewfinder->setFixedSize(ViewSets[0].resolution());
	this->setWindowTitle(title);
	//viewfinder->setFixedSize(this->camera->supportedViewfinderResolutions());
}

void CameraShowDialog::closeEvent(QCloseEvent* e) {
	delete this->camera;
	this->camera = NULL;
}