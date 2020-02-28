#pragma once
#include <QWidget>
#include <QCamera>
#include <QDialog>

class CamerasShowWidget :public QWidget {
	Q_OBJECT
public:
	CamerasShowWidget(QCamera* leftCamera, QCamera* rightCamera, QWidget* parent = Q_NULLPTR);
	QCamera* leftCamera;
	QCamera* rightCamera;
	void closeEvent(QCloseEvent* e);
};