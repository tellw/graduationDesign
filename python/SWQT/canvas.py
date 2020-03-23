from PyQt5.QtWidgets import QWidget
from PyQt5.QtGui import QPainter, QColor, QCursor, QPixmap
from PyQt5.QtCore import Qt, QPoint
from utils import bounding_rect
import copy

class Canvas(QWidget):
    def __init__(self, docker, parent=None):
        super(Canvas, self).__init__(parent)
        self.pix = None
        self.setMouseTracking(True)
        self.bDraw = False
        self.prev_point = None
        self.drawColor = None
        self.shapes = []
        self.current_shape = ['', [[] for x in range(2)]]
        self.picScale = 1.0
        self.cursor = QCursor(Qt.ArrowCursor)
        self.bScale = False
        self.docker = docker

    def paintEvent(self, e):
        painter = QPainter(self)
        if self.pix != None:
            painter.scale(self.picScale, self.picScale)
            painter.drawPixmap(0, 0, self.pix)
            if self.bDraw:
                painter.setPen(QColor(0, 0, 0))
                painter.drawLine(self.prev_point.x(), 0, self.prev_point.x(), self.height())
                painter.drawLine(0, self.prev_point.y(), self.width(), self.prev_point.y())
                self.setCursor(QCursor(Qt.CrossCursor))
            else:
                self.setCursor(QCursor(Qt.ArrowCursor))
            painter.setPen(self.drawColor)
            if len(self.shapes) > 0:
                for shape in self.shapes:
                    if shape[0] == 'rect':
                        the_rect = bounding_rect(shape[1])
                        painter.drawRect(the_rect[0], the_rect[1], the_rect[2]-the_rect[0], the_rect[3]-the_rect[1])
                    elif shape[0] == 'circle':
                        the_rect = bounding_rect(shape[1])
                        painter.drawEllipse(the_rect[0], the_rect[1], the_rect[2]-the_rect[0], the_rect[3]-the_rect[1])
                    elif shape[0] == 'poly':
                        for i in range(-1, len(shape[1][0])-1):
                            painter.drawLine(shape[1][0][i], shape[1][1][i], shape[1][0][i+1], shape[1][1][i+1])
            if len(self.current_shape[1][0]) > 0:
                if self.current_shape[0] == 'rect':
                    the_rect = bounding_rect(self.current_shape[1])
                    painter.drawRect(the_rect[0], the_rect[1], the_rect[2] - the_rect[0], the_rect[3] - the_rect[1])
                elif self.current_shape[0] == 'circle':
                    the_rect = bounding_rect(self.current_shape[1])
                    painter.drawEllipse(the_rect[0], the_rect[1], the_rect[2] - the_rect[0], the_rect[3] - the_rect[1])
                elif self.current_shape[0] == 'poly':
                    if len(self.current_shape[1][0]) > 2:
                        for i in range(0, len(self.current_shape[1][0])-1):
                            painter.drawLine(self.current_shape[1][0][i], self.current_shape[1][1][i], self.current_shape[1][0][i + 1], self.current_shape[1][1][i + 1])
                    painter.drawLine(self.current_shape[1][0][0], self.current_shape[1][1][0], self.prev_point.x(), self.prev_point.y())
                    painter.drawLine(self.current_shape[1][0][-1], self.current_shape[1][1][-1], self.prev_point.x(), self.prev_point.y())

    def load_image(self, image):
        self.pix = QPixmap(QPixmap.fromImage(image))
        self.resize(self.picScale*self.pix.size())
        self.repaint()

    def set_picScale(self, picScale):
        self.picScale = picScale

    def resetState(self):
        self.pix = None
        self.clearAllShapes()
        self.update()

    def sizeHint(self):
        if self.pix != None:
            return self.picScale*self.pix.size()
        else:
            return self.size()

    def enterEvent(self, e):
        self.overrideCursor(self.cursor)

    def leaveEvent(self, e):
        self.setCursor(QCursor(Qt.ArrowCursor))

    def overrideCursor(self, cursor):
        self.cursor = cursor
        self.setCursor(self.cursor)

    def mouseMoveEvent(self, e):
        if self.bDraw:
            self.prev_point = self.pointInPic(e)
            if self.current_shape[0] == 'rect' or self.current_shape[0] == 'circle':
                self.current_shape[1][0][1] = self.prev_point.x()
                self.current_shape[1][1][1] = self.prev_point.y()
            self.update()

    def mousePressEvent(self, e):
        if self.bScale:
            if e.button() == Qt.LeftButton:
                self.setCursor(QCursor(QPixmap('imgs/zoomIn.png')))
            elif e.button() == Qt.RightButton:
                self.setCursor(QCursor(QPixmap('imgs/zoomOut.png')))
        elif self.bDraw:
            if e.button() == Qt.LeftButton:
                self.prev_point = self.pointInPic(e)
                if self.docker.rectRadioButton.isChecked():
                    self.current_shape[0] = 'rect'
                    self.current_shape[1][0].append(self.prev_point.x())
                    self.current_shape[1][0].append(self.prev_point.x())
                    self.current_shape[1][1].append(self.prev_point.y())
                    self.current_shape[1][1].append(self.prev_point.y())
                elif self.docker.circleRadioButton.isChecked():
                    self.current_shape[0] = 'circle'
                    self.current_shape[1][0].append(self.prev_point.x())
                    self.current_shape[1][0].append(self.prev_point.x())
                    self.current_shape[1][1].append(self.prev_point.y())
                    self.current_shape[1][1].append(self.prev_point.y())
                elif self.docker.polyRadioButton.isChecked():
                    self.current_shape[0] = 'poly'
                    self.current_shape[1][0].append(self.prev_point.x())
                    self.current_shape[1][1].append(self.prev_point.y())
                elif self.docker.eraserRadioButton.isChecked():
                    for shape in self.shapes:
                        the_rect = bounding_rect(shape[1])
                        if self.prev_point.x() >= the_rect[0] and self.prev_point.y() >= the_rect[1] and self.prev_point.x() <= the_rect[2] and self.prev_point.y() <= the_rect[3]:
                            self.shapes.remove(shape)
                            break
            elif e.button() == Qt.RightButton:
                if self.docker.polyRadioButton.isChecked():
                    self.addCurrentShapeToShapes()
        self.update()

    def mouseReleaseEvent(self, e):
        if self.bScale:
            if e.button() == Qt.LeftButton:
                self.picScale += 0.1
                self.docker.picScaleSlider.setValue(int(self.picScale*10))
                self.loadPixmap()
                self.setCursor(QCursor(Qt.ArrowCursor))
            elif e.button() == Qt.RightButton:
                self.picScale -= 0.1
                self.docker.picScaleSlider.setValue(int(self.picScale*10))
                self.loadPixmap()
                self.setCursor(QCursor(Qt.ArrowCursor))
        elif self.bDraw:
            if e.button() == Qt.LeftButton:
                if self.docker.rectRadioButton.isChecked() or self.docker.circleRadioButton.isChecked():
                    the_point = self.pointInPic(e)
                    self.current_shape[1][0][1] = the_point.x()
                    self.current_shape[1][1][1] = the_point.y()
                    self.addCurrentShapeToShapes()
        self.update()

    def loadPixmap(self):
        self.resize(self.picScale*self.pix.size())
        self.repaint()

    def pointInPic(self, e):
        return QPoint(int(e.pos().x()/self.picScale), int(e.pos().y()/self.picScale))

    def setDrawColor(self, drawColor):
        self.drawColor = drawColor

    def addCurrentShapeToShapes(self):
        if len(self.current_shape[1][0]) > 0:
            self.shapes.append(copy.deepcopy(self.current_shape))
            self.current_shape[0] = ''
            self.current_shape[1][0].clear()
            self.current_shape[1][1].clear()

    def clearAllShapes(self):
        self.shapes.clear()
        if len(self.current_shape[1][0]) > 0:
            self.current_shape[0] = ''
            self.current_shape[1][0].clear()
            self.current_shape[1][1].clear()