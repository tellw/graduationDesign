#include "SWQTDock.h"
#include <QVBoxLayout>
#include "PicScaleWidget.h"

SWQTDock::SWQTDock(QWidget* parent):QWidget(parent) {
	this->picScaleWidget = new PicScaleWidget(this);
	this->someProcessWidget = new SomeProcessWidget(this);
	this->drawRadiosWidget = new DrawRadiosWidget(this);
	this->selectionStatusWidget = new SelectionStatusWidget(this);
	this->getRadiosWidget = new GetRadiosWidget(this);
	this->sizeMeasureOptionWidget = new SizeMeasureOptionWidget();
	QVBoxLayout* dockLayout = new QVBoxLayout();
	dockLayout->setAlignment(Qt::AlignTop);
	dockLayout->addWidget(this->picScaleWidget);
	dockLayout->addWidget(this->someProcessWidget);
	dockLayout->addWidget(this->drawRadiosWidget);
	dockLayout->addWidget(this->selectionStatusWidget);
	dockLayout->addWidget(this->getRadiosWidget);
	dockLayout->addWidget(this->sizeMeasureOptionWidget);
	this->setLayout(dockLayout);
}

QSize SWQTDock::sizeHint() const {
	return QSize(240, 600);
}