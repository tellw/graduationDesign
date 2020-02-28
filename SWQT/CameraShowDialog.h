#pragma once
#include <QWidget>
#include <QCamera>
#include <QDialog>

class CameraShowDialog :public QDialog {
	Q_OBJECT
public:
	CameraShowDialog(QCamera* camera, QString& title, QWidget* parent = Q_NULLPTR);
	QCamera* camera;
	void closeEvent(QCloseEvent* e);
};