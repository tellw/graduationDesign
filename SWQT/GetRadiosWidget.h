#pragma once
#include <QWidget>
#include <QRadioButton>

class GetRadiosWidget : public QWidget {
	Q_OBJECT

public:
	GetRadiosWidget(QWidget* parent = Q_NULLPTR);
	QRadioButton* picRadioButton;
	QRadioButton* captureSelectedPicRadioButton;
	QRadioButton* captureRadioButton;
};