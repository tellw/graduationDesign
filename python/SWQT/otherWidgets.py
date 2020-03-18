from PyQt5.QtWidgets import QWidget, QVBoxLayout, QLabel, QHBoxLayout, QSlider, QLineEdit, QGridLayout, QCheckBox, QRadioButton, QComboBox, QFrame, QDialog, QPushButton, QButtonGroup
from PyQt5.QtCore import Qt, QSize
from PyQt5.QtGui import QDoubleValidator, QIntValidator
from configparser import ConfigParser
from volumeMeasure import stereo_calibrator

class SWQTDocker(QWidget):
    def __init__(self, volumeMeasure, parent=None):
        super(SWQTDocker, self).__init__(parent)
        self.initUI()
        self.picScale = 1.0
        self.volumeMeasure = volumeMeasure

    def initUI(self):
        cfg = ConfigParser()
        cfg.read('SWQT.ini')
        dockLayout = QVBoxLayout(self)
        dockLayout.setAlignment(Qt.AlignTop)

        picScaleLayout = QHBoxLayout()
        picScaleLabel = QLabel('Scale:')
        self.picScaleSlider = QSlider(Qt.Horizontal)
        self.picScaleSlider.setMinimum(5)
        self.picScaleSlider.setMaximum(30)
        self.picScaleSlider.setValue(10)
        self.picScaleSlider.setSingleStep(1)
        self.picScaleSlider.setPageStep(5)
        self.picScaleSlider.valueChanged.connect(self.picScaleSliderValueChanged)
        self.picScaleLE = QLineEdit('1')
        picScaleLEValidator = QDoubleValidator(0.5, 3, 1, self.picScaleLE)
        picScaleLEValidator.setNotation(QDoubleValidator.StandardNotation)
        self.picScaleLE.setValidator(picScaleLEValidator)
        self.picScaleLE.setFixedWidth(27)
        self.picScaleLE.textChanged.connect(self.picScaleLETextChanged)
        picScaleLayout.addWidget(picScaleLabel)
        picScaleLayout.addWidget(self.picScaleSlider)
        picScaleLayout.addWidget(self.picScaleLE)

        someProcessLayout = QGridLayout()
        self.gaussianBlurCheckBox = QCheckBox('Gaussian Blur')
        self.cannyCheckBox = QCheckBox('Canny')
        self.CLAHECheckBox = QCheckBox('CLAHE')
        self.sharpenCheckBox = QCheckBox('Sharpen')
        self.laplacianCheckBox = QCheckBox('Laplacian')
        self.grayCheckBox = QCheckBox('Gray')
        self.OTSUCheckBox = QCheckBox('OTSU')
        self.RCFContourDetectCheckBox = QCheckBox('RCF Contour Detect')
        if cfg.getboolean('process', 'bGray'):
            self.grayCheckBox.setChecked(True)
        if cfg.getboolean('process', 'bGaussianBlur'):
            self.gaussianBlurCheckBox.setChecked(True)
        if cfg.getboolean('process', 'bCLAHE'):
            self.CLAHECheckBox.setChecked(True)
        if cfg.getboolean('process', 'bSharpen'):
            self.sharpenCheckBox.setChecked(True)
        if cfg.getboolean('process', 'bLaplacian'):
            self.laplacianCheckBox.setChecked(True)
        if cfg.getboolean('process', 'bOTSU'):
            self.OTSUCheckBox.setChecked(True)
        if cfg.getboolean('process', 'bRCFContourDetect'):
            self.RCFContourDetectCheckBox.setChecked(True)
        if cfg.getboolean('process', 'bCanny'):
            self.cannyCheckBox.setChecked(True)
        self.RCFContourDetectCheckBox.setToolTip('RCF Contour Detect')
        someProcessLayout.addWidget(self.grayCheckBox, 0, 0)
        someProcessLayout.addWidget(self.gaussianBlurCheckBox, 0, 1)
        someProcessLayout.addWidget(self.cannyCheckBox, 3, 1)
        someProcessLayout.addWidget(self.CLAHECheckBox, 1, 0)
        someProcessLayout.addWidget(self.sharpenCheckBox, 1, 1)
        someProcessLayout.addWidget(self.laplacianCheckBox, 2, 0)
        someProcessLayout.addWidget(self.OTSUCheckBox, 2, 1)
        someProcessLayout.addWidget(self.RCFContourDetectCheckBox, 3, 0)

        drawRadiosLayout = QGridLayout()
        self.rectRadioButton = QRadioButton('Rect')
        self.circleRadioButton = QRadioButton('Circle')
        self.polyRadioButton = QRadioButton('Poly')
        self.eraserRadioButton = QRadioButton('Eraser')
        drawWay = cfg.get('draw', 'drawWay')
        if drawWay == 'Rect':
            self.rectRadioButton.setChecked(True)
        elif drawWay == 'Circle':
            self.circleRadioButton.setChecked(True)
        elif drawWay == 'Poly':
            self.polyRadioButton.setChecked(True)
        elif drawWay == 'Eraser':
            self.eraserRadioButton.setChecked(True)
        drawRadiosLayout.addWidget(self.rectRadioButton, 0, 0)
        drawRadiosLayout.addWidget(self.circleRadioButton, 0, 1)
        drawRadiosLayout.addWidget(self.polyRadioButton, 1, 0)
        drawRadiosLayout.addWidget(self.eraserRadioButton, 1, 1)

        selectionStatusLayout = QHBoxLayout()
        self.bScaleCheckBox = QCheckBox('bScale')
        self.bDrawCheckBox = QCheckBox('bDraw')
        self.bScaleCheckBox.setEnabled(False)
        self.bDrawCheckBox.setEnabled(False)
        selectionStatusLayout.addWidget(self.bScaleCheckBox)
        selectionStatusLayout.addWidget(self.bDrawCheckBox)

        getRadiosLayout = QGridLayout()
        self.picRadioButton = QRadioButton('Pic')
        self.captureSelectedPicRadioButton = QRadioButton('CaptureSelectedPic')
        self.captureRadioButton = QRadioButton('Capture')
        captureWay = cfg.get('binocular_cameras', 'captureWay')
        if captureWay == 'Pic':
            self.picRadioButton.setChecked(True)
        elif captureWay == 'CaptureSelectedPic':
            self.captureSelectedPicRadioButton.setChecked(True)
        else:
            self.captureRadioButton.setChecked(True)
        getRadiosLayout.addWidget(self.picRadioButton, 0, 0)
        getRadiosLayout.addWidget(self.captureSelectedPicRadioButton, 0, 1)
        getRadiosLayout.addWidget(self.captureRadioButton, 1, 0)

        sizeMeasureOptionLayout = QGridLayout()
        self.cameraToGroundHeightCheckBox = QCheckBox('Camera to ground height: ')
        self.cameraToGroundHeightCheckBox.setToolTip('Camera to ground height: ')
        if cfg.getboolean('binocular_cameras', 'bFromTop'):
            self.cameraToGroundHeightCheckBox.setChecked(True)
        self.cameraToGroundHeightLE = QLineEdit(cfg.get('binocular_cameras', 'cameraToGroundHeight'))
        cameraToGroundHeightLEValidator = QDoubleValidator(0, 1000, 2, self.cameraToGroundHeightLE)
        cameraToGroundHeightLEValidator.setNotation(QDoubleValidator.StandardNotation)
        self.cameraToGroundHeightLE.setValidator(cameraToGroundHeightLEValidator)
        self.cameraToGroundHeightLE.textChanged.connect(self.cameraToGroundHeightLETextChanged)
        cmLabel = QLabel('cm')
        sizeMeasureOptionLayout.addWidget(self.cameraToGroundHeightCheckBox, 0, 0, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.cameraToGroundHeightLE, 0, 2)
        sizeMeasureOptionLayout.addWidget(cmLabel, 0, 3)
        self.bVisualizedCheckBox = QCheckBox('bVisualized')
        if cfg.getboolean('binocular_cameras', 'bVisualized'):
            self.bVisualizedCheckBox.setChecked(True)
        self.bCollectedCheckBox = QCheckBox('bCollected')
        if cfg.getboolean('binocular_cameras', 'bCollected'):
            self.bCollectedCheckBox.setChecked(True)
        self.bTimeRecordedCheckBox = QCheckBox('bTimeRecorded')
        if cfg.getboolean('binocular_cameras', 'bTimeRecorded'):
            self.bTimeRecordedCheckBox.setChecked(True)
        self.bmmCheckBox = QCheckBox('bmm')
        if cfg.getboolean('binocular_cameras', 'bmm'):
            self.bmmCheckBox.setChecked(True)
        self.bAutoCheckBox = QCheckBox('bAuto')
        if cfg.getboolean('binocular_cameras', 'bAuto'):
            self.bAutoCheckBox.setChecked(True)
        sizeMeasureOptionLayout.addWidget(self.bVisualizedCheckBox, 1, 0, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.bCollectedCheckBox, 1, 2, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.bTimeRecordedCheckBox, 2, 0, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.bmmCheckBox, 2, 2, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.bAutoCheckBox, 3, 0, 1, 2)
        self.cameraToGroundHeightCheckBox.stateChanged.connect(lambda:self.bCheckBoxStateChanged(self.cameraToGroundHeightCheckBox))
        self.bVisualizedCheckBox.stateChanged.connect(lambda:self.bCheckBoxStateChanged(self.bVisualizedCheckBox))
        self.bCollectedCheckBox.stateChanged.connect(lambda:self.bCheckBoxStateChanged(self.bCollectedCheckBox))
        self.bTimeRecordedCheckBox.stateChanged.connect(lambda:self.bCheckBoxStateChanged(self.bTimeRecordedCheckBox))
        self.bmmCheckBox.stateChanged.connect(lambda:self.bCheckBoxStateChanged(self.bmmCheckBox))
        self.bAutoCheckBox.stateChanged.connect(lambda:self.bCheckBoxStateChanged(self.bAtuoCheckBox))
        processThinglabel = QLabel('Process thing: ')
        disparityStyleLabel = QLabel('Disparity style: ')
        constructionStyleLabel = QLabel('Construction style: ')
        sizeCalculationLabel = QLabel('Size calculation: ')
        self.processThingList = ['Contour looking for rects']
        self.disparityStyleList = ['SGBMDisparity']
        self.constructionStyleList = ['Easy style']
        self.sizeCalculationList = ['Rect calculation']
        self.processThingComboBox = QComboBox(self)
        self.disparityStyleComboBox = QComboBox(self)
        self.constructionStyleComboBox = QComboBox(self)
        self.sizeCalculationComboBox = QComboBox(self)
        self.processThingComboBox.addItems(self.processThingList)
        self.disparityStyleComboBox.addItems(self.disparityStyleList)
        self.constructionStyleComboBox.addItems(self.constructionStyleList)
        self.sizeCalculationComboBox.addItems(self.sizeCalculationList)
        self.processThingComboBox.setCurrentText(cfg.get('binocular_cameras', 'processThing'))
        self.disparityStyleComboBox.setCurrentText(cfg.get('binocular_cameras', 'disparityStyle'))
        self.constructionStyleComboBox.setCurrentText(cfg.get('binocular_cameras', 'constructionStyle'))
        self.sizeCalculationComboBox.setCurrentText(cfg.get('binocular_cameras', 'sizeCalculation'))
        self.processThingComboBox.currentIndexChanged.connect(self.processThingComboBoxCurrentIndexChanged)
        self.disparityStyleComboBox.currentIndexChanged.connect(self.disparityStyleComboBoxCurrentIndexChanged)
        self.constructionStyleComboBox.currentIndexChanged.connect(self.constructionStyleComboBoxCurrentIndexChanged)
        self.sizeCalculationComboBox.currentIndexChanged.connect(self.sizeCalculationComboBoxCurrentIndexChanged)
        self.processThingComboBox.setToolTip(self.processThingComboBox.currentText())
        self.disparityStyleComboBox.setToolTip(self.disparityStyleComboBox.currentText())
        self.constructionStyleComboBox.setToolTip(self.constructionStyleComboBox.currentText())
        self.sizeCalculationComboBox.setToolTip(self.sizeCalculationComboBox.currentText())
        sizeMeasureOptionLayout.addWidget(processThinglabel, 4, 0, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.processThingComboBox, 4, 2, 1, 2)
        sizeMeasureOptionLayout.addWidget(disparityStyleLabel, 5, 0, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.disparityStyleComboBox, 5, 2, 1, 2)
        sizeMeasureOptionLayout.addWidget(constructionStyleLabel, 6, 0, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.constructionStyleComboBox, 6, 2, 1, 2)
        sizeMeasureOptionLayout.addWidget(sizeCalculationLabel, 7, 0, 1, 2)
        sizeMeasureOptionLayout.addWidget(self.sizeCalculationComboBox, 7, 2, 1, 2)

        line1 = QFrame()
        line1.setFrameShape(QFrame.HLine)
        line2 = QFrame()
        line2.setFrameShape(QFrame.HLine)
        line3 = QFrame()
        line3.setFrameShape(QFrame.HLine)
        line4 = QFrame()
        line4.setFrameShape(QFrame.HLine)
        line5 = QFrame()
        line5.setFrameShape(QFrame.HLine)
        dockLayout.addLayout(picScaleLayout)
        dockLayout.addWidget(line1)
        dockLayout.addLayout(someProcessLayout)
        dockLayout.addWidget(line2)
        dockLayout.addLayout(drawRadiosLayout)
        dockLayout.addWidget(line3)
        dockLayout.addLayout(selectionStatusLayout)
        dockLayout.addWidget(line4)
        dockLayout.addLayout(getRadiosLayout)
        dockLayout.addWidget(line5)
        dockLayout.addLayout(sizeMeasureOptionLayout)

        self.setLayout(dockLayout)

    def picScaleSliderValueChanged(self, value):
        self.picScale = value/10
        self.picScaleLE.setText(str(self.picScale))

    def picScaleLETextChanged(self, text):
        self.picScale = float(text)
        if self.picScale > 3:
            self.picScale = 3
        elif self.picScale < 0.5:
            self.picScale = 0.5
        self.picScaleSlider.setValue(int(self.picScale*10))

    def sizeHint(self):
        return QSize(240, 600)

    def bCheckBoxStateChanged(self, checkBox):
        if checkBox.text() == 'Camera to ground height: ':
            if checkBox.isChecked():
                self.volumeMeasure.bFromTop = True
            else:
                self.volumeMeasure.bFromTop = False
        elif checkBox.text() == 'bVisualized':
            if checkBox.isChecked():
                self.volumeMeasure.bVisualized = True
            else:
                self.volumeMeasure.bVisualized = False
        elif checkBox.text() == 'bCollected':
            if checkBox.isChecked():
                self.volumeMeasure.bCollected = True
            else:
                self.volumeMeasure.bCollected = False
        elif checkBox.text() == 'bTimeRecorded':
            if checkBox.isChecked():
                self.volumeMeasure.bTimeRecorded = True
            else:
                self.volumeMeasure.bTimeRecorded = False
        elif checkBox.text() == 'bmm':
            if checkBox.isChecked():
                self.volumeMeasure.bmm = True
            else:
                self.volumeMeasure.bmm = False
        else:
            if checkBox.isChecked():
                self.volumeMeasure.bAuto = True
            else:
                self.volumeMeasure.bAuto = False

    def cameraToGroundHeightLETextChanged(self, text):
        print('camera height is %s cm'%text)
        self.volumeMeasure.camera_height = float(text)

    def processThingComboBoxCurrentIndexChanged(self, index):
        print('processThing is %s, %s'%(self.processThingList[index], self.processThingComboBox.currentText()))
        self.volumeMeasure.processThing = self.processThingList[index]
        self.processThingComboBox.setCurrentIndex(index)

    def disparityStyleComboBoxCurrentIndexChanged(self, index):
        print('disparityStyle is %s, %s'%(self.disparityStyleList[index], self.disparityStyleComboBox.currentText()))
        self.volumeMeasure.disparityStyle = self.disparityStyleList[index]
        self.disparityStyleComboBox.setCurrentIndex(index)

    def constructionStyleComboBoxCurrentIndexChanged(self, index):
        print('constructionStyle is %s, %s'%(self.constructionStyleList[index], self.constructionStyleComboBox.currentText()))
        self.volumeMeasure.constructionStyle = self.constructionStyleList[index]
        self.constructionStyleComboBox.setCurrentIndex(index)

    def sizeCalculationComboBoxCurrentIndexChanged(self, index):
        print('sizeCalculation is %s, %s'%(self.sizeCalculationList[index], self.sizeCalculationComboBox.currentText()))
        self.volumeMeasure.sizeCalculation = self.sizeCalculationList[index]
        self.sizeCalculationComboBox.setCurrentIndex(index)

class SettingsDialog(QDialog):
    def __init__(self, volumeMeasure, parent=None):
        super(SettingsDialog, self).__init__(parent)
        self.cfg = ConfigParser()
        self.cfg.read('SWQT.ini')
        self.initUI()
        self.volumeMeasure = volumeMeasure

    def initUI(self):
        CLAHECoeffLabel = QLabel('CLAHECoeff')
        cannyHighThreshLabel = QLabel('cannyHighThreshLabel')
        cannyLowThreshLabel = QLabel('cannyLowThresh')
        gaussianBlurKSizeLabel = QLabel('gaussianBlurKSize')
        cut_rateLabel = QLabel('cut_rate')
        xmlfileLabel = QLabel('xmlfile')
        leftCameraIdLabel = QLabel('leftCameraId')
        rightCameraIdLabel = QLabel('rightCameraId')
        self.CLAHECoeffLE = QLineEdit(self.cfg.get('process', 'CLAHECoeff'))
        self.cannyHighThreshLE = QLineEdit(self.cfg.get('process', 'cannyHighThresh'))
        self.cannyLowThreshLE = QLineEdit(self.cfg.get('process', 'cannyLowThresh'))
        self.gaussianBlurKSizeLE = QLineEdit(self.cfg.get('process', 'gaussianBlurKSize'))
        self.cut_rateLE = QLineEdit(self.cfg.get('binocular_cameras', 'cut_rate'))
        self.xmlfileLE = QLineEdit(self.cfg.get('binocular_cameras', 'xmlfile'))
        self.leftCameraIdLE = QLineEdit(self.cfg.get('binocular_cameras', 'leftCameraId'))
        self.rightCameraIdLE = QLineEdit(self.cfg.get('binocular_cameras', 'rightCameraId'))
        CLAHECoeffLEValidator = QDoubleValidator(0, 100, 1, self.CLAHECoeffLE)
        CLAHECoeffLEValidator.setNotation(QDoubleValidator.StandardNotation)
        self.CLAHECoeffLE.setValidator(CLAHECoeffLEValidator)
        self.CLAHECoeffLE.setFixedWidth(27)
        cannyHighThreshLEValidator = QIntValidator(0, 255, self.cannyHighThreshLE)
        self.cannyHighThreshLE.setValidator(cannyHighThreshLEValidator)
        self.cannyHighThreshLE.setFixedWidth(27)
        cannyLowThreshLEValidator = QIntValidator(0, 255, self.cannyLowThreshLE)
        self.cannyLowThreshLE.setValidator(cannyLowThreshLEValidator)
        self.cannyLowThreshLE.setFixedWidth(27)
        gaussianBlurKSizeLEValidator = QIntValidator(1, 10, self.gaussianBlurKSizeLE)
        self.gaussianBlurKSizeLE.setValidator(gaussianBlurKSizeLEValidator)
        self.gaussianBlurKSizeLE.setFixedWidth(27)
        cut_rateLEValidator = QDoubleValidator(0, 0.5, 2, self.cut_rateLE)
        cut_rateLEValidator.setNotation(QDoubleValidator.StandardNotation)
        self.cut_rateLE.setValidator(cut_rateLEValidator)
        self.cut_rateLE.setFixedWidth(27)
        leftCameraIdLEValidator = QIntValidator(-2, 10, self.leftCameraIdLE)
        self.leftCameraIdLE.setValidator(leftCameraIdLEValidator)
        self.leftCameraIdLE.setFixedWidth(27)
        rightCameraIdLEValidator = QIntValidator(-2, 10, self.rightCameraIdLE)
        self.rightCameraIdLE.setValidator(rightCameraIdLEValidator)
        self.rightCameraIdLE.setFixedWidth(27)
        saveButton = QPushButton('Save')
        cancelButton = QPushButton('Cancel')
        settingsDialogLayout = QGridLayout()
        settingsDialogLayout.addWidget(CLAHECoeffLabel, 0, 0)
        settingsDialogLayout.addWidget(self.CLAHECoeffLE, 0, 1)
        settingsDialogLayout.addWidget(cannyHighThreshLabel, 1, 0)
        settingsDialogLayout.addWidget(self.cannyHighThreshLE, 1, 1)
        settingsDialogLayout.addWidget(cannyLowThreshLabel, 2, 0)
        settingsDialogLayout.addWidget(self.cannyLowThreshLE, 2, 1)
        settingsDialogLayout.addWidget(gaussianBlurKSizeLabel, 3, 0)
        settingsDialogLayout.addWidget(self.gaussianBlurKSizeLE, 3, 1)
        settingsDialogLayout.addWidget(cut_rateLabel, 4, 0)
        settingsDialogLayout.addWidget(self.cut_rateLE, 4, 1)
        settingsDialogLayout.addWidget(xmlfileLabel, 5, 0)
        settingsDialogLayout.addWidget(self.xmlfileLE, 5, 1)
        settingsDialogLayout.addWidget(leftCameraIdLabel, 6, 0)
        settingsDialogLayout.addWidget(self.leftCameraIdLE, 6, 1)
        settingsDialogLayout.addWidget(rightCameraIdLabel, 7, 0)
        settingsDialogLayout.addWidget(self.rightCameraIdLE, 7, 1)
        settingsDialogLayout.addWidget(saveButton, 8, 0)
        settingsDialogLayout.addWidget(cancelButton, 8, 1)
        self.setLayout(settingsDialogLayout)
        saveButton.clicked.connect(self.saveButtonClicked)
        cancelButton.clicked.connect(self.close)

    def saveButtonClicked(self):
        self.cfg.set('process', 'CLAHECoeff', self.CLAHECoeffLE.text())
        self.cfg.set('process', 'cannyHighThresh', self.cannyHighThreshLE.text())
        self.cfg.set('process', 'cannyLowThresh', self.cannyLowThreshLE.text())
        self.cfg.set('process', 'gaussianBlurKSize', self.gaussianBlurKSizeLE.text())
        if self.cfg.get('binocular_cameras', 'cut_rate') != self.cut_rateLE.text():
            self.cfg.set('binocular_cameras', 'cut_rate', self.cut_rateLE.text())
            self.volumeMeasure.cut_rate = float(self.cut_rateLE.text())
        if self.cfg.get('binocular_cameras', 'xmlfile') != self.xmlfileLE.text():
            self.cfg.set('binocular_cameras', 'xmlfile', self.xmlfileLE.text())
            self.volumeMeasure.calibrator = stereo_calibrator(self.xmlfileLE.text(), 'static')
        self.cfg.set('binocular_cameras', 'leftCameraId', self.leftCameraIdLE.text())
        self.cfg.set('binocular_cameras', 'rightCameraId', self.rightCameraIdLE.text())
        self.cfg.write(open('SWQT.ini', 'w'))
        self.close()