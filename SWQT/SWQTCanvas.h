#pragma once

#include <QtWidgets/QWidget>
#include <QScrollArea>
#include "PicScaleWidget.h"
#include "DrawRadiosWidget.h"
#include "Shape.h"

class SWQTCanvas :public QWidget {
	Q_OBJECT
public:
	SWQTCanvas(QWidget *parent = Q_NULLPTR);
	//QPainter painter;
	QPixmap* pix;
	void loadImage(QImage* image);
	void loadImage(QString& image_name);
	float picScale = 1.0;
	void setPicScale(float picScale);
	//QPointF offsetToCenter();
	void resetState();
	QSize sizeHint() const;
	bool bScale = false;
	bool bDraw = false;
	QCursor* cursor = new QCursor(Qt::ArrowCursor);
	void enterEvent(QEvent* ev);
	void leaveEvent(QEvent* ev);
	void overrideCursor(QCursor* cursor);
	void mouseMoveEvent(QMouseEvent* ev);
	void mousePressEvent(QMouseEvent* ev);
	void mouseReleaseEvent(QMouseEvent* ev);
	void loadPixmap();
	PicScaleWidget* picScaleWidget = NULL;
	void setPicScaleWidget(PicScaleWidget* picScaleWidget);
	DrawRadiosWidget* drawRadiosWidget;
	void setDrawRadiosWidget(DrawRadiosWidget* drawRadiosWidget);
	std::vector<Shape> shapes;
	Shape current_shape;
	QPoint prev_point;
	QPoint pointInPic(QMouseEvent* ev);
	QColor drawColor;
	void setDrawColor(QColor& drawColor);
	void addCurrentShapeToShapes();
	void clearAllShapes();
protected:
	void paintEvent(QPaintEvent *event);
};