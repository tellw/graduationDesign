#include "SWQTCanvas.h"
#include <QPainter>
#include <QDebug>
#include <QtWidgets/QApplication>
#include <QMouseEvent>

std::vector<QPoint> rectFourPoints(QPoint& left_top, QPoint& right_bottom) {
	std::vector<QPoint> res;
	res.push_back(left_top);
	QPoint right_top(right_bottom.x(), left_top.y());
	res.push_back(right_top);
	res.push_back(right_bottom);
	QPoint left_bottom(left_top.x(), right_bottom.y());
	res.push_back(left_bottom);
	return res;
}

std::vector<int> boundingRect(std::vector<QPoint>& points) {
	int xmin = points[0].x(), ymin = points[0].y(), xmax = points[0].x(), ymax = points[0].y();
	for (int i = 1; i < points.size(); i++) {
		if (points[i].x() < xmin)xmin = points[i].x();
		else if (points[i].x() > xmax)xmax = points[i].x();
		if (points[i].y() < ymin) ymin = points[i].y();
		else if (points[i].y() > ymax)ymax = points[i].y();
	}
	std::vector<int> res;
	res.push_back(xmin);
	res.push_back(ymin);
	res.push_back(xmax);
	res.push_back(ymax);
	return res;
}

//qreal distance(QPoint p1, QPoint p2) {
//	return sqrt((p2.x() - p1.x())*(p2.x() - p1.x()) + (p2.y() - p1.y())*(p2.y() - p1.y()));
//}

SWQTCanvas::SWQTCanvas(QWidget* parent) :QWidget(parent) {
	this->pix = NULL;
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//this->setFocusPolicy(Qt::WheelFocus);
	this->setMouseTracking(true);
}

void SWQTCanvas::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	//QDebug(QtDebugMsg) << "SWQTCanvas's gemetry: " << this->geometry().width() << " " << this->geometry().height() << "\n";
	if (this->pix != NULL) {
		painter.scale(this->picScale, this->picScale);
		//painter.translate(this->offsetToCenter());
		//QDebug(QtDebugMsg) << "offsetToCenter()'s result is " << this->offsetToCenter() << "\n";
		painter.drawPixmap(0, 0, *this->pix);
	}
	if (this->bDraw) {
		painter.setPen(QColor(0, 0, 0));
		painter.drawLine(this->prev_point.x(), 0, this->prev_point.x(), this->height());
		painter.drawLine(0, this->prev_point.y(), this->width(), this->prev_point.y());
		this->setCursor(Qt::CrossCursor);
	}
	else {
		this->setCursor(Qt::ArrowCursor);
	}
	painter.setPen(this->drawColor);
	if (this->shapes.size() > 0) {
		for (auto shape : shapes) {
			switch (shape.st) {
			case shape_type::rect: {
				std::vector<int> theRect = boundingRect(shape.points);
				painter.drawRect(theRect[0], theRect[1], abs(shape.points[0].x()-shape.points[1].x()), abs(shape.points[0].y()-shape.points[1].y()));
				break;
			}
			case shape_type::circle: {
				std::vector<int> theRect = boundingRect(shape.points);
				painter.drawEllipse(theRect[0], theRect[1], abs(shape.points[0].x() - shape.points[1].x()), abs(shape.points[0].y() - shape.points[1].y()));
				break;
			}
			case shape_type::poly: {
				for (int i = 0; i < shape.points.size()-1; i++)painter.drawLine(shape.points[i].x(), shape.points[i].y(), shape.points[i + 1].x(), shape.points[i + 1].y());
				painter.drawLine(shape.points.back().x(), shape.points.back().y(), shape.points[0].x(), shape.points[0].y());
				break;
			}
			}
		}
	}
	if (this->current_shape.points.size() != 0) {
		switch (current_shape.st) {
		case shape_type::rect: {
			std::vector<int> theRect = boundingRect(current_shape.points);
			painter.drawRect(theRect[0], theRect[1], abs(current_shape.points[0].x() - prev_point.x()), abs(current_shape.points[0].y() -prev_point.y()));
			break;
		}
		case shape_type::circle: {
			std::vector<int> theRect = boundingRect(current_shape.points);
			painter.drawEllipse(theRect[0], theRect[1], abs(current_shape.points[0].x() - prev_point.x()), abs(current_shape.points[0].y() - prev_point.y()));
			break;
		}
		case shape_type::poly: {
			for (int i = 0; i < current_shape.points.size() - 1; i++)painter.drawLine(current_shape.points[i].x(), current_shape.points[i].y(), current_shape.points[i + 1].x(), current_shape.points[i + 1].y());
			painter.drawLine(current_shape.points.back().x(), current_shape.points.back().y(), prev_point.x(), prev_point.y());
			painter.drawLine(prev_point.x(), prev_point.y(), current_shape.points[0].x(), current_shape.points[0].y());
			break;
		}
		}
	}
}

void SWQTCanvas::loadImage(QImage* image) {
	this->pix = new QPixmap(QPixmap::fromImage(*image));
	this->resize(this->picScale*this->pix->size());
	this->repaint();
}

void SWQTCanvas::loadImage(QString& image_name) {
	this->pix = new QPixmap(image_name);
	this->resize(this->picScale*this->pix->size());
	this->repaint();
}

void SWQTCanvas::setPicScale(float picScale) {
	this->picScale = picScale;
}

//QPointF SWQTCanvas::offsetToCenter() {
//	QSize area = this->size();
//	int w = this->pix->width()*this->picScale;
//	int h = this->pix->height()*this->picScale;
//	int aw = area.width();
//	int ah = area.height();
//	QDebug(QtDebugMsg) << "values: " << w << " " << h << " " << aw << " " << ah << " " << this->picScale << "\n";
//	float x = 0, y = 0;
//	if (aw > w*this->picScale)x = (aw - w*this->picScale) / 2;
//	if (ah > h*this->picScale)y = (ah - h*this->picScale) / 2;
//	return QPointF(x, y);
//}

void SWQTCanvas::resetState() {
	this->pix = NULL;
	this->clearAllShapes();
	this->update();
}

QSize SWQTCanvas::sizeHint() const{
	if (this->pix != NULL) {
		//QDebug(QtDebugMsg) << "size: " << this->picScale*this->pix->size() << "\n"; 
		return this->picScale*this->pix->size();
	}
	else return this->size();
}

void SWQTCanvas::enterEvent(QEvent* ev) {
	this->overrideCursor(this->cursor);
}

void SWQTCanvas::leaveEvent(QEvent* ev) {
	this->setCursor(QCursor(Qt::ArrowCursor));
}

void SWQTCanvas::overrideCursor(QCursor* cursor) {
	this->cursor = cursor;
	this->setCursor(*this->cursor);
}

void SWQTCanvas::mouseMoveEvent(QMouseEvent* ev) {
	//QDebug(QtDebugMsg) << ev->pos() << "\n";
	this->prev_point = this->pointInPic(ev);
	this->update();
}

void SWQTCanvas::mousePressEvent(QMouseEvent* ev) {
	if (this->bScale) {
		if (ev->button() == Qt::LeftButton) {
			this->setCursor(QCursor(QPixmap("imgs/zoomIn.png"), -1, -1));
		}
		else if (ev->button() == Qt::RightButton) {
			this->setCursor(QCursor(QPixmap("imgs/zoomOut.png"), -1, -1));
		}
	}
	if (this->bDraw) {
		if (ev->button() == Qt::LeftButton) {
			if (this->drawRadiosWidget->rectRadioButton->isChecked()) {
				this->prev_point = this->pointInPic(ev);
				//QDebug(QtDebugMsg) << this->prev_point << "\n";
				this->current_shape.st = shape_type::rect;
				this->current_shape.points.push_back(this->prev_point);
				//this->current_shape.points.push_back(this->prev_point);
			}
			else if (this->drawRadiosWidget->circleRadioButton->isChecked()) {
				this->prev_point = this->pointInPic(ev);
				//QDebug(QtDebugMsg) << this->prev_point << "\n";
				this->current_shape.st = shape_type::circle;
				this->current_shape.points.push_back(this->prev_point);
				//this->current_shape.points.push_back(this->prev_point);
			}
			else if (this->drawRadiosWidget->polyRadioButton->isChecked()) {
				this->prev_point = this->pointInPic(ev);
				//QDebug(QtDebugMsg) << this->prev_point << "\n";
				this->current_shape.st = shape_type::poly;
				this->current_shape.points.push_back(this->prev_point);
				//this->current_shape.points.push_back(this->prev_point);
			}
			else if (this->drawRadiosWidget->eraserRadioButton->isChecked()) {
				this->prev_point = this->pointInPic(ev);
				for (int i = 0; i < this->shapes.size(); i++) {
					std::vector<int> theRect = boundingRect(this->shapes[i].points);
					if (this->prev_point.x() >= theRect[0] && this->prev_point.y() >= theRect[1] && this->prev_point.x() <= theRect[2] && this->prev_point.y() <= theRect[3]) {
						this->shapes.erase(this->shapes.begin() + i);
						break;
					}
				}
			}
		}
		else if (ev->button() == Qt::RightButton) {
			if (this->drawRadiosWidget->polyRadioButton) {
				this->addCurrentShapeToShapes();
			}
		}
		
	}
	this->update();
}

void SWQTCanvas::mouseReleaseEvent(QMouseEvent* ev) {
	if (this->bScale) {
		if (ev->button() == Qt::LeftButton) {
			this->picScale += 0.1;
			this->picScaleWidget->picScaleSlider->setValue(this->picScale*10);
			this->loadPixmap();
			this->setCursor(QCursor(Qt::ArrowCursor));
		}
		else if (ev->button() == Qt::RightButton) {
			this->picScale -= 0.1;
			this->picScaleWidget->picScaleSlider->setValue(this->picScale*10);
			this->loadPixmap();
			this->setCursor(QCursor(Qt::ArrowCursor));
		}
	}
	if (bDraw) {
		if (ev->button() == Qt::LeftButton) {
			if (this->drawRadiosWidget->rectRadioButton->isChecked()) {
				this->current_shape.points.push_back(pointInPic(ev));
				this->addCurrentShapeToShapes();
			}
			else if (this->drawRadiosWidget->circleRadioButton->isChecked()) {
				this->current_shape.points.push_back(pointInPic(ev));
				this->addCurrentShapeToShapes();
			}
		}
	}
	this->update();
}

void SWQTCanvas::loadPixmap() {
	this->resize(this->picScale*this->pix->size());
	this->repaint();
}

void SWQTCanvas::setPicScaleWidget(PicScaleWidget* picScaleWidget) {
	this->picScaleWidget = picScaleWidget;
}

void SWQTCanvas::setDrawRadiosWidget(DrawRadiosWidget* drawRadiosWidget) {
	this->drawRadiosWidget = drawRadiosWidget;
}

QPoint SWQTCanvas::pointInPic(QMouseEvent* ev) {
	return QPoint((int)(ev->pos().x() / this->picScale), (int)(ev->pos().y() / this->picScale));
}

void SWQTCanvas::setDrawColor(QColor& drawColor) {
	this->drawColor = drawColor;
}

void SWQTCanvas::addCurrentShapeToShapes() {
	if (this->current_shape.points.size() > 1) {
		this->shapes.push_back(this->current_shape);
		this->current_shape.points.clear();
		std::vector<QPoint>().swap(this->current_shape.points);
	}
}

void SWQTCanvas::clearAllShapes() {
	this->shapes.clear();
	std::vector<Shape>().swap(this->shapes);
	this->current_shape.points.clear();
	std::vector<QPoint>().swap(this->current_shape.points);
}