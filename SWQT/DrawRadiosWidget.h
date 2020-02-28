#pragma once
#include <QWidget>
#include <QRadioButton>

class DrawRadiosWidget : public QWidget {
	Q_OBJECT

public:
	DrawRadiosWidget(QWidget* parent = Q_NULLPTR);
	QRadioButton* rectRadioButton;
	QRadioButton* circleRadioButton;
	//QRadioButton* linesRadioButton;
	QRadioButton* polyRadioButton;
	QRadioButton* eraserRadioButton;
};