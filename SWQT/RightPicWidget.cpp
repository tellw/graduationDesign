#include "RightPicWidget.h"
#include <QHBoxLayout>

RightPicWidget::RightPicWidget(QImage* image, QWidget* parent) :QWidget(parent) {
	this->image = image;
	this->setWindowTitle(tr("Right Pic"));
	this->setMinimumSize(500, 400);
	QHBoxLayout* rightPicWidgetLayout = new QHBoxLayout();
	QScrollArea* scroll = new QScrollArea(this);
	rightPicWidgetLayout->addWidget(scroll);
	this->setLayout(rightPicWidgetLayout);
	this->swqtCanvas = new SWQTCanvas();
	scroll->setWidget(this->swqtCanvas);
	scroll->setWidgetResizable(false);
	scroll->setAlignment(Qt::AlignCenter);
	this->swqtCanvas->loadImage(image);
}

void RightPicWidget::closeEvent(QCloseEvent* e) {
	delete this->image;
	this->image = NULL;
}