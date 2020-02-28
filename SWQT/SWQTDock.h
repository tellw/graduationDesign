#pragma once

#include <QWidget>
#include "PicScaleWidget.h"
#include "SomeProcessWidget.h"
#include "DrawRadiosWidget.h"
#include "SelectionStatusWidget.h"
#include "GetRadiosWidget.h"
#include "SizeMeasureOptionWidget.h"

class SWQTDock : public QWidget {
	Q_OBJECT
public:
	SWQTDock(QWidget* parent = Q_NULLPTR);
	QSize sizeHint() const;
	PicScaleWidget* picScaleWidget;
	SomeProcessWidget* someProcessWidget;
	DrawRadiosWidget* drawRadiosWidget;
	SelectionStatusWidget* selectionStatusWidget;
	GetRadiosWidget* getRadiosWidget;
	SizeMeasureOptionWidget* sizeMeasureOptionWidget;
};