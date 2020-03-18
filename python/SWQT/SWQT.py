from PyQt5.QtWidgets import QApplication, QMainWindow, QScrollArea, QDockWidget, QFileDialog, QMessageBox, QColorDialog, QDialog, QGridLayout, QLabel, QButtonGroup, QRadioButton, QDialogButtonBox
from PyQt5.QtGui import QIcon, QColor, QImage, QPixmap
import sys
from configparser import ConfigParser
from canvas import Canvas
from PyQt5.QtCore import Qt
from otherWidgets import SWQTDocker, SettingsDialog
from utils import create_action, npndarray2qimage, messageBox_info
import numpy as np
import cv2
import os
from volumeMeasure import VolumeMeasure
from PyQt5.QtMultimedia import QCameraInfo

class SWQT(QMainWindow):
    def __init__(self, parent=None):
        super(SWQT, self).__init__(parent)
        self.initUI()
        self.src = None
        self.right_src = None
        self.cfg = ConfigParser()
        self.cfg.read('SWQT.ini')
        self.canvas.setDrawColor(QColor(self.cfg.getint('file', 'drawColorRed'), self.cfg.getint('file', 'drawColorGreen'), self.cfg.getint('file', 'drawColorBlue')))
        self.leftCameraId = self.cfg.getint('binocular_cameras', 'leftCameraId')
        self.rightCameraId = self.cfg.getint('binocular_cameras', 'rightCameraId')
        self.m_camera = []
        self.bContinuouslyCaptured = False
        if self.leftCameraId > -1:
            self.left_cap = cv2.VideoCapture(self.leftCameraId)
        if self.rightCameraId > -1:
            self.right_cap = cv2.VideoCapture(self.rightCameraId)

    def initUI(self):
        self.setWindowTitle('SWQT')
        self.setMinimumSize(700, 500)
        self.showMaximized()
        self.status = self.statusBar()
        self.status.showMessage('Starting successfully', 5000)
        self.setWindowIcon(QIcon('imgs/SWQT.png'))

        self.dock = QDockWidget(self)
        self.addDockWidget(Qt.RightDockWidgetArea, self.dock)
        dockScroll = QScrollArea(self.dock)
        self.volumeMeasure = VolumeMeasure()
        self.dockWidget = SWQTDocker(self.volumeMeasure)
        dockScroll.setWidget(self.dockWidget)
        dockScroll.setWidgetResizable(False)
        self.dock.setWidget(dockScroll)

        scroll = QScrollArea(self)
        self.canvas = Canvas(self.dockWidget)
        scroll.setWidget(self.canvas)
        scroll.setWidgetResizable(False)
        self.setCentralWidget(scroll)
        scroll.setAlignment(Qt.AlignCenter)

        file = self.menuBar().addMenu('&File')
        process = self.menuBar().addMenu('&Process')
        settings = self.menuBar().addMenu('&Settings')
        binocularCameras = self.menuBar().addMenu('&Binocular Cameras')
        toolbarFile = self.addToolBar('File')
        self.addToolBar(Qt.LeftToolBarArea, toolbarFile)
        toolbarProcess = self.addToolBar('Process')
        self.addToolBar(Qt.LeftToolBarArea, toolbarProcess)
        toolbarSettings = self.addToolBar('Settings')
        self.addToolBar(Qt.LeftToolBarArea, toolbarSettings)
        toolbarBinocularCameras = self.addToolBar('Binocular Cameras')
        self.addToolBar(Qt.LeftToolBarArea, toolbarBinocularCameras)
        openAPicAction = create_action('imgs/openAPic.png', '&Open a picture', self, 'Ctrl+O', 'open a picture', file, toolbarFile)
        openAPicAction.triggered.connect(self.openAPic)
        quitAction = create_action('imgs/quit.jpg', '&Quit', self, 'Ctrl+Q', 'quit', file, toolbarFile)
        quitAction.triggered.connect(self.close)
        drawAction = create_action('imgs/draw.png', '&Draw', self, 'Ctrl+D', 'draw', file, toolbarFile)
        drawAction.triggered.connect(self.draw)
        openRightPicAction = create_action('imgs/openRightPic.jpg', '&Open right picture', self, 'Ctrl+2', 'open right picutre', file, toolbarFile)
        openRightPicAction.triggered.connect(self.openRightPic)
        refreshImageAction = create_action('imgs/refreshImage.jpg', '&Refresh image', self, 'Ctrl+R', 'refresh image', process, toolbarProcess)
        refreshImageAction.triggered.connect(self.refreshImage)
        scaleAction = create_action('imgs/scale.png', '&Scale image', self, 'Ctrl+S', 'scale image', process, toolbarProcess)
        scaleAction.triggered.connect(self.scale)
        someProcessAction = create_action('imgs/someProcess.jpg', '&Show some process', self, 'Ctrl+P', 'show some process', process, toolbarProcess)
        someProcessAction.triggered.connect(self.someProcess)
        settingsAction = create_action('imgs/settings.jpg', '&Settings', self, 'Ctrl+E', 'settings', settings, toolbarSettings)
        settingsAction.triggered.connect(self.settings)
        selectDrawColorAction = create_action('imgs/selectDrawColor.png', '&Select draw color', self, 'Ctrl+L', 'select draw color', file, toolbarFile)
        selectDrawColorAction.triggered.connect(self.selectDrawColor)
        clearAllShapesAction = create_action('imgs/clearAllShapes.png', '&Clear all shapes', self, 'Ctrl+1', 'clear all shapes', file, toolbarFile)
        clearAllShapesAction.triggered.connect(self.clearAllShapes)
        openCamerasAction = create_action('imgs/openBinocularCameras.jpg', '&Open binocular cameras', self, 'Ctrl+B', 'open binocular cameras', binocularCameras, toolbarBinocularCameras)
        openCamerasAction.triggered.connect(self.openCameras)
        measureAction = create_action('imgs/measure.png', '&Measure', self, 'Ctrl+M', 'measure', binocularCameras, toolbarBinocularCameras)
        measureAction.triggered.connect(self.measure)
        showCamerasAction = create_action('imgs/showCameras.jpg', '&Show binocular cameras', self, 'Ctrl+3', 'show binocular cameras', binocularCameras, toolbarBinocularCameras)
        showCamerasAction.triggered.connect(self.showCameras)

    def openAPic(self):
        self.setBScale(False)
        self.setBDraw(False)
        pic_file = QFileDialog.getOpenFileName(self, 'open a picture', self.cfg.get('file', 'lastOpenDir'), 'All files(*.*);;Image files(*.bmp *.jpg *.pbm *.png *.ppm *.xbm *.xpm)')
        if pic_file != '':
            self.cfg.set('file', 'lastOpenDir', os.path.dirname(pic_file))
            with open('SWQT.ini', 'w') as configfile:
                self.cfg.write(configfile)
            self.resetState()
            self.src = cv2.imread(pic_file)
            if self.src is None:
                messageBox_info('the path should include Englise symbol only or we could not read a .gif file', self)
            else:
                self.loadImage(npndarray2qimage(self.src))

    def setPicScale(self, picScale):
        pass

    def resetState(self):
        self.src = None
        self.canvas.resetState()

    def paintCanvas(self):
        self.canvas.adjustSize()
        self.canvas.update()

    def refreshImage(self):
        self.setBScale(False)
        self.setBDraw(False)
        if self.src is not None:
            self.canvas.set_picScale(self.dockWidget.picScaleSlider.value()/10)
            self.canvas.load_image(npndarray2qimage(self.src))

    def loadImage(self, image):
        self.status.showMessage('loading picture', 5000)
        self.canvas.load_image(image)
        self.paintCanvas()

    def scale(self):
        self.setBScale(True)
        self.setBDraw(False)

    def someProcess(self):
        self.setBScale(False)
        self.setBDraw(False)
        if self.src != None:
            toProcess = self.someProcessDetailed(self.src)
            self.canvas.set_picScale(self.dockWidget.picScaleSlider.value()/10)
            self.canvas.load_image(npndarray2qimage(toProcess))

    def settings(self):
        self.setBScale(False)
        self.setBDraw(False)
        settingsDialog = SettingsDialog(self.volumeMeasure, self)
        settingsDialog.exec_()

    def draw(self):
        self.setBScale(False)
        self.setBDraw(True)

    def setBScale(self, bScale):
        if bScale != self.dockWidget.bScaleCheckBox.isChecked():
            self.canvas.bScale = bScale
            if bScale == True:
                self.dockWidget.bScaleCheckBox.setChecked(True)
            else:
                self.dockWidget.bScaleCheckBox.setChecked(False)

    def setBDraw(self, bDraw):
        if bDraw != self.dockWidget.bDrawCheckBox.isChecked():
            self.canvas.bDraw = bDraw
            if bDraw == True:
                self.dockWidget.bDrawCheckBox.setChecked(True)
            else:
                self.dockWidget.bDrawCheckBox.setChecked(False)

    def selectDrawColor(self):
        self.setBScale(False)
        colorDialog = QColorDialog(self)
        colorDialog.setWindowTitle('Select Draw Color: ')
        colorDialog.setCurrentColor(Qt.red)
        if colorDialog.exec_() == QColorDialog.Accepted:
            drawColor = colorDialog.selectedColor()
            self.canvas.setDrawColor(drawColor)
            red = drawColor.red()
            green = drawColor.green()
            blue = drawColor.blue()
            self.cfg.set('file', 'drawColorRed', str(red))
            self.cfg.set('file', 'drawColorGreen', str(green))
            self.cfg.set('file', 'drawColorBlue', str(blue))
            self.cfg.write(open('SWQT.ini', 'w'))

    def clearAllShapes(self):
        self.setBScale(False)
        self.canvas.clearAllShapes()
        self.paintCanvas()

    def openCameras(self):
        self.setBScale(False)
        self.setBDraw(False)
        for info in QCameraInfo.availableCameras():
            self.m_camera.append(info)
        if len(self.m_camera) < 2:
            messageBox_info('please get your binocular cameras', self)
        else:
            self.camerasSelectDialog = QDialog(self)
            self.camerasSelectDialog.setMinimumWidth(950)
            self.camerasSelectDialog.setWindowTitle('Select your disired cameras')
            camerasSelectDialogLayout = QGridLayout()
            leftLabel = QLabel('left')
            rightLabel = QLabel('right')
            leftCameraButtonGroup = QButtonGroup(self.camerasSelectDialog)
            rightCameraButtonGroup = QButtonGroup(self.camerasSelectDialog)
            camerasSelectDialogLayout.addWidget(leftLabel, 1, 0)
            camerasSelectDialogLayout.addWidget(rightLabel, 2, 0)
            for i in range(len(self.m_camera)):
                capLabel = QLabel()
                capLabel.setFixedSize(400, 300)
                cap = cv2.VideoCapture(i)
                _, then_cap = cap.read()
                cap.release()
                capLabel.setPixmap(QPixmap.fromImage(npndarray2qimage(then_cap)))
                camerasSelectDialogLayout.addWidget(capLabel, 0, i+1)
                indexStr = 'Index: %d'%i
                optionalLeftCameraRadioButton = QRadioButton(indexStr)
                leftCameraButtonGroup.addButton(optionalLeftCameraRadioButton, i)
                optionalRightCameraRadioButton = QRadioButton(indexStr)
                rightCameraButtonGroup.addButton(optionalRightCameraRadioButton, i)
                if i == 0:
                    optionalLeftCameraRadioButton.setChecked(True)
                    optionalRightCameraRadioButton.setChecked(True)
                camerasSelectDialogLayout.addWidget(optionalLeftCameraRadioButton, 1, i+1)
                camerasSelectDialogLayout.addWidget(optionalRightCameraRadioButton, 2, i+1)
            button = QDialogButtonBox(self.camerasSelectDialog)
            button.addButton('OK', QDialogButtonBox.YesRole)
            button.addButton('Cancel', QDialogButtonBox.NoRole)
            button.accepted.connect(self.camerasSelectDialog.accept)
            button.rejected.connect(self.camerasSelectDialog.reject)
            camerasSelectDialogLayout.addWidget(button, 3, 0, 1, -1)
            self.camerasSelectDialog.setLayout(camerasSelectDialogLayout)
            res = self.camerasSelectDialog.exec_()
            if res == QDialog.Accepted:
                if self.leftCameraId > -1:
                    self.left_cap.release()
                if self.rightCameraId > -1:
                    self.right_cap.release()
                self.leftCameraId = leftCameraButtonGroup.checkedId()
                self.rightCameraId = rightCameraButtonGroup.checkedId()
                self.cfg.set('binocular_cameras', 'leftCameraId', str(self.leftCameraId))
                self.cfg.set('binocular_cameras', 'rightCameraId', str(self.rightCameraId))
                self.cfg.write(open('SWQT.ini', 'w'))
                self.left_cap = cv2.VideoCapture(self.leftCameraId)
                self.right_cap = cv2.VideoCapture(self.rightCameraId)
        self.m_camera.clear()

    def openRightPic(self):
        self.setBScale(False)
        self.setBDraw(False)
        picFile = QFileDialog.getOpenFileName(self, 'open a right picture', self.cfg.get('file', 'lastOpenRightDir'), 'All files(*.*);;Image files(*.bmp *.jpg *.pbm *.png *.ppm *.xbm *.xpm)')
        if picFile != '':
            self.cfg.set('file', 'lastOpenRightDir', os.path.dirname(picFile))
            with open('SWQT.ini', 'w') as configfile:
                self.cfg.write(configfile)
            self.right_src = cv2.imread(picFile)
            if self.right_src is None:
                messageBox_info('the path should include Englise symbol only or we could not read a .gif file', self)
            else:
                cv2.imshow('right pic', self.right_src)

    def measure(self):
        if self.dockWidget.picRadioButton.isChecked():
            if self.right_src == None or self.src == None or self.src.shape != self.right_src.shape:
                messageBox_info('please get two proper pictures', self)
            else:
                left = self.someProcessDetailed(self.src)
                right = self.someProcessDetailed(self.right_src)
                self.canvas.load_image(npndarray2qimage(left))
                self.volumeMeasure.detect(left, right, self.src, self.right_src)
        elif self.leftCameraId <= -1 or self.rightCameraId < 0:
            messageBox_info('Please get your binocular cameras', self)
        else:
            if self.dockWidget.captureSelectedPicRadioButton.isChecked():
                _, left_frame = self.left_cap.read()
                _, right_frame = self.right_cap.read()
                if left_frame.shape == right_frame.shape:
                    self.src = left_frame
                    self.right_src = right_frame
                    left = self.someProcessDetailed(left_frame)
                    right = self.someProcessDetailed(right_frame)
                    self.canvas.load_image(npndarray2qimage(left))
                    cv2.imshow('right pic', right_frame)
                    ret = self.volumeMeasure.detect(left, right, left_frame, right_frame)
                    if ret == 1:
                        messageBox_info('please binarify the image(eg. gray, canny or thresh)', self)
                else:
                    messageBox_info('you possibly get wrong binocular cameras', self)
                    self.openCameras()
            else:
                self.bContinuouslyCaptured = not self.bContinuouslyCaptured
                bChecked = False
                while(self.bContinuouslyCaptured):
                    _, left_frame = self.left_cap.read()
                    _, right_frame = self.right_cap.read()
                    if not bChecked:
                        bChecked = True
                        if left_frame.shape != right_frame.shape:
                            messageBox_info('you possibly get wrong binocular cameras', self)
                            self.openCameras()
                    self.src = left_frame
                    self.right_src = right_frame
                    left = self.someProcessDetailed(left_frame)
                    right = self.someProcessDetailed(right_frame)
                    self.canvas.load_image(npndarray2qimage(left))
                    cv2.imshow('right pic', right_frame)
                    ret = self.volumeMeasure.detect(left, right, left_frame, right_frame)
                    if ret == 1:
                        messageBox_info('please binarify the image(eg.gray, canny or thresh)', self)
                    if not self.bContinuouslyCaptured or ret != 0:
                        cv2.destroyWindow('right pic')
                        break

    def someProcessDetailed(self, cv_image):
        if cv_image.shape[2] == 4:
            cv_image = cv2.cvtColor(cv_image, cv2.COLOR_BGRA2BGR)
        if len(cv_image.shape) > 2 and self.dockWidget.grayCheckBox.isChecked():
            cv_image = cv2.cvtColor(cv_image, cv2.COLOR_BGR2GRAY)
        if self.dockWidget.gaussianBlurCheckBox.isChecked():
            gaussianBlurKSize = self.cfg.getint('process', 'gaussianBlurKSize')
            cv_image = cv2.GaussianBlur(cv_image, (gaussianBlurKSize, gaussianBlurKSize), 1, 1)
        if self.dockWidget.CLAHECheckBox.isChecked():
            clahe = cv2.createCLAHE(clipLimit=self.cfg.getfloat('process', 'CLAHECoeff'))
            if len(cv_image.shape) == 2:
                cv_image = clahe.apply(cv_image)
            else:
                clahe_image = cv2.cvtColor(cv_image, cv2.COLOR_BGR2Lab)
                splited = cv2.split(clahe_image)
                dst = clahe.apply(splited[0])
                np.copyto(splited[0], dst)
                cv_image = cv2.merge(splited)
                cv_image = cv2.cvtColor(cv_image, cv2.COLOR_Lab2BGR)
        if self.dockWidget.sharpenCheckBox.isChecked():
            laplacian = np.ndarray([[0, -1, 0], [-1, 5, -1], [0, -1, 0]])
            cv_image = cv2.filter2D(cv_image, -1, laplacian)
        if self.dockWidget.laplacianCheckBox.isChecked():
            laplacian = np.ndarray([[0, -1, 0], [-1, 4, -1], [0, -1, 0]])
            cv_image = cv2.filter2D(cv_image, -1, laplacian)
        if self.dockWidget.OTSUCheckBox.isChecked():
            if len(cv_image.shape) == 2:
                _, cv_image = cv2.threshold(cv_image, 0, 255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)
            else:
                splited = cv2.split(cv_image)
                for i in range(3):
                    _, splited[i] = cv2.threshold(splited[i], 0, 255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)
                cv_image = cv2.merge(splited)
        if self.dockWidget.RCFContourDetectCheckBox.isChecked():
            pass
        if self.dockWidget.cannyCheckBox.isChecked():
            cannyLowThresh = self.cfg.getint('process', 'cannyLowThresh')
            cannyHighThresh = self.cfg.getint('process', 'cannyHighThresh')
            if len(cv_image.shape) == 2:
                cv_image = cv2.Canny(cv_image, cannyLowThresh, cannyHighThresh)
            else:
                splited = cv2.split(cv_image)
                for i in range(3):
                    splited[i] = cv2.Canny(splited[i], cannyLowThresh, cannyHighThresh)
                cv_image = cv2.merge(splited)
        return cv_image

    def showCameras(self):
        if self.leftCameraId < 0 or self.rightCameraId < 0:
            messageBox_info('please get your binocular cameras', self)
            self.openCameras()
        else:
            while True:
                _, left_frame = self.left_cap.read()
                _, right_frame = self.right_cap.read()
                cv2.imshow('left_frame', left_frame)
                cv2.imshow('right_frame', right_frame)
                c = cv2.waitKey(25)
                if c == ord('q') or c == 27:
                    break
            cv2.destroyAllWindows()

    def closeEvent(self, *args, **kwargs):
        if self.leftCameraId > -1 or self.rightCameraId > -1:
            self.left_cap.release()
            self.right_cap.release()
        cv2.destroyAllWindows()
        if self.dockWidget.grayCheckBox.isChecked():
            self.cfg.set('process', 'bGray', '1')
        else:
            self.cfg.set('process', 'bGray', '0')
        if self.dockWidget.gaussianBlurCheckBox.isChecked():
            self.cfg.set('process', 'bGaussianBlur', '1')
        else:
            self.cfg.set('process', 'bGaussianBlur', '0')
        if self.dockWidget.CLAHECheckBox.isChecked():
            self.cfg.set('process', 'bCLAHE', '1')
        else:
            self.cfg.set('process', 'bCLAHE', '0')
        if self.dockWidget.sharpenCheckBox.isChecked():
            self.cfg.set('process', 'bSharpen', '1')
        else:
            self.cfg.set('process', 'bSharpen', '0')
        if self.dockWidget.laplacianCheckBox.isChecked():
            self.cfg.set('process', 'bLaplacian', '1')
        else:
            self.cfg.set('process', 'bLaplacian', '0')
        if self.dockWidget.OTSUCheckBox.isChecked():
            self.cfg.set('process', 'bOTSU', '1')
        else:
            self.cfg.set('process', 'bOTSU', '0')
        if self.dockWidget.RCFContourDetectCheckBox.isChecked():
            self.cfg.set('process', 'bRCFContourDetect', '1')
        else:
            self.cfg.set('process', 'bRCFContourDetect', '0')
        if self.dockWidget.cannyCheckBox.isChecked():
            self.cfg.set('process', 'bCanny', '1')
        else:
            self.cfg.set('process', 'bCanny', '0')
        if self.dockWidget.rectRadioButton.isChecked():
            self.cfg.set('draw', 'drawWay', 'Rect')
        elif self.dockWidget.circleRadioButton.isChecked():
            self.cfg.set('draw', 'drawWay', 'Circle')
        elif self.dockWidget.polyRadioButton.isChecked():
            self.cfg.set('draw', 'drawWay', 'Poly')
        else:
            self.cfg.set('draw', 'drawWay', 'Eraser')
        if self.dockWidget.picRadioButton.isChecked():
            self.cfg.set('binocular_cameras', 'captureWay', 'Pic')
        elif self.dockWidget.captureSelectedPicRadioButton.isChecked():
            self.cfg.set('binocular_cameras', 'captureWay', 'CaptureSelectedPic')
        else:
            self.cfg.set('binocular_cameras', 'captureWay', 'Capture')
        if self.dockWidget.cameraToGroundHeightCheckBox.isChecked():
            self.cfg.set('binocular_cameras', 'bFromTop', '1')
        else:
            self.cfg.set('binocular_cameras', 'bFromTop', '0')
        self.cfg.set('binocular_cameras', 'cameraToGroundHeight', self.dockWidget.cameraToGroundHeightLE.text())
        if self.dockWidget.bVisualizedCheckBox.isChecked():
            self.cfg.set('binocular_cameras', 'bVisualized', '1')
        else:
            self.cfg.set('binocular_cameras', 'bVisualized', '0')
        if self.dockWidget.bCollectedCheckBox.isChecked():
            self.cfg.set('binocular_cameras', 'bCollected', '1')
        else:
            self.cfg.set('binocular_cameras', 'bCollected', '0')
        if self.dockWidget.bTimeRecordedCheckBox.isChecked():
            self.cfg.set('binocular_cameras', 'bTimeRecorded', '1')
        else:
            self.cfg.set('binocular_cameras', 'bTimeRecorded', '0')
        if self.dockWidget.bmmCheckBox.isChecked():
            self.cfg.set('binocular_cameras', 'bmm', '1')
        else:
            self.cfg.set('binocular_cameras', 'bmm', '0')
        if self.dockWidget.bAutoCheckBox.isChecked():
            self.cfg.set('binocular_cameras', 'bAuto', '1')
        else:
            self.cfg.set('binocular_cameras', 'bAuto', '0')
        self.cfg.set('binocular_cameras', 'processThing', self.dockWidget.processThingComboBox.currentText())
        self.cfg.set('binocular_cameras', 'disparityStyle', self.dockWidget.disparityStyleComboBox.currentText())
        self.cfg.set('binocular_cameras', 'constructionStyle', self.dockWidget.constructionStyleComboBox.currentText())
        self.cfg.set('binocular_cameras', 'sizeCalculation', self.dockWidget.sizeCalculationComboBox.currentText())

if __name__ == '__main__':
    app = QApplication([])
    win = SWQT()
    win.show()
    sys.exit(app.exec_())