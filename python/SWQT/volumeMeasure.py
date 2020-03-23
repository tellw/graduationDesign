from configparser import ConfigParser
import cv2
import numpy as np
import time
from utils import _find_polys, compute_disparity, visualize_disparity, reproject, _distance

class stereo_calibrator:
    def __init__(self, xmlfile, initmode):
        self.subpix_window_size = [4, 4]
        self.min_good_images = 1
        self.left_intrinsic = np.zeros((3, 3))
        self.left_coeffs = np.zeros((1, 5))
        self.right_intrinsic = np.zeros((3, 3))
        self.right_coeffs = np.zeros((1, 5))
        self.rotation = np.zeros((3, 3))
        self.transform = np.zeros((3, 1))
        self.r1 = np.zeros((3, 3))
        self.r2 = np.zeros((3, 3))
        self.p1 = np.zeros((3, 4))
        self.p2 = np.zeros((3, 4))
        self.q = np.zeros((4, 4))
        if initmode == 'static':
            assert self.load_params(xmlfile), 'failed to open the parameter file'
        elif initmode == 'calib':
            assert self.chessboard_calibrate(xmlfile), 'error occurred on calibration'
        elif initmode == 'matlab_calib':
            assert self.matlab_calibrate(xmlfile), 'error occurred on calibration'
        else:
            input('mode not supported.')

    def load_params(self, xmlfile):
        infile = cv2.FileStorage(xmlfile, cv2.FILE_STORAGE_READ)
        if not infile.isOpened():
            return False
        self.left_intrinsic = infile.getNode('left_intrinsic').mat()
        self.left_coeffs = infile.getNode('left_coeffs').mat()
        self.right_intrinsic = infile.getNode('right_intrinsic').mat()
        self.right_coeffs = infile.getNode('right_coeffs').mat()
        self.rotation = infile.getNode('rotation').mat()
        self.transform = infile.getNode('transform').mat()
        self.r1 = infile.getNode('r1').mat()
        self.r2 = infile.getNode('r2').mat()
        self.p1 = infile.getNode('p1').mat()
        self.p2 = infile.getNode('p2').mat()
        self.q = infile.getNode('q').mat()
        infile.release()
        return True

    def chessboard_calibrate(self, xmlfile):
        storage = cv2.FileStorage(xmlfile, cv2.FILE_STORAGE_READ)
        if not storage.isOpened():
            return False
        image_size = [int(i) for i in storage.getNode('image_size').string().strip().split(' ')]
        pattern = [int(i) for i in storage.getNode('pattern').string().strip().split(' ')]#pattern 按行 列的含义排列
        grid_size = [float(i) for i in storage.getNode('grid_size').string().strip().split(' ')]
        path = storage.getNode('path').string().strip().split(';')
        storage.release()
        lefts = []
        rights = []
        for file in path:
            origin = cv2.imread(file, 0)
            assert origin.shape[0] == image_size[1] and origin.shape[1] == image_size[0]*2
            lefts.append(origin[:, :image_size[0]])
            rights.append(origin[:, image_size[0]:])
        return self._calibrate(lefts, rights, image_size, pattern, grid_size, self.subpix_window_size)

    def _calibrate(self, lefts, rights, image_size, pattern, grid_size, win_size):
        assert len(lefts) == 0
        assert len(lefts) == len(rights)
        total_frames = len(lefts)
        count_good = 0
        left_corners = []
        right_corners = []
        for i in range(total_frames):
            retl, left_corner = cv2.findChessboardCorners(lefts[i], (pattern[0], pattern[1]), None)
            retr, right_corner = cv2.findChessboardCorners(rights[i], (pattern[0], pattern[1]), None)
            if not retl or not retr:
                print('Bad frames %d'%i)
                continue
            retl, left_corner = cv2.find4QuadCornerSubpix(lefts[i], left_corner, win_size)
            retr, right_corner = cv2.find4QuadCornerSubpix(rights[i], right_corner, win_size)
            left_corners.append(left_corner)
            right_corners.append(right_corner)
            count_good+=1
        if count_good < self.min_good_images:
            return False
        corner_positions = self._cal_corner_positions(pattern, grid_size)
        corner_positions_vec = [corner_positions for i in range(count_good)]
        calib_left_ret, self.left_intrinsic, self.left_coeffs, left_rvecs, left_tvecs = cv2.calibrateCamera(corner_positions_vec, left_corners, image_size, self.left_intrinsic, self.left_coeffs)
        calib_right_ret, self.right_intrinsic, self.right_coeffs, right_rvecs, right_tvecs = cv2.calibrateCamera(corner_positions_vec, right_corners, image_size, self.right_intrinsic, self.right_coeffs)
        print('left calibration error: {}, right calibration error: {}'.format(calib_left_ret, calib_right_ret))
        stereo_calib_ret, self.left_intrinsic, self.left_coeffs, self.right_intrinsic, self.right_coeffs, self.rotation, self.transform, E, F = cv2.stereoCalibrate(corner_positions_vec, left_corners, right_corners, self.left_intrinsic, self.left_coeffs, self.right_intrinsic, self.right_coeffs, image_size)
        print('stereo calibration error: {}'.format(stereo_calib_ret))
        self.r1, self.r2, self.p1, self.p2, self.q = cv2.stereoRectify(self.left_intrinsic, self.left_coeffs, self.right_intrinsic, self.right_coeffs, image_size, self.rotation, self.transform)
        return True

    def matlab_calibrate(self, xmlfile):
        if not self.load_params_matlab(xmlfile):
            return False
        img_width = int(input('标定时设置的单张图片宽度：'))
        img_height = int(input('标定时设置的单张图片高度：'))
        self.r1, self.r2, self.p1, self.p2, self.q = cv2.stereoRectify(self.left_intrinsic, self.left_coeffs, self.right_intrinsic, self.right_coeffs, (img_width, img_height), self.rotation, self.transform)
        return True

    def load_params_matlab(self, xmlfile):
        storage = cv2.FileStorage(xmlfile, cv2.FILE_STORAGE_READ)
        if not storage.isOpened():
            return False
        self.left_intrinsic = storage.getNode('left_intrinsic').mat()
        self.left_coeffs = storage.getNode('left_coeffs').mat()
        self.right_intrinsic = storage.getNode('right_intrinsic').mat()
        self.right_coeffs = storage.getNode('right_coeffs').mat()
        self.rotation = storage.getNode('rotation').mat()
        self.transform = storage.getNode('transform').mat()
        storage.release()
        return True

    def rectify_left(self, left_img):
        new_size = (left_img.shape[1], left_img.shape[0])
        mapx, mapy = cv2.initUndistortRectifyMap(self.left_intrinsic, self.left_coeffs, self.r1, self.p1, new_size, cv2.CV_8U)
        rectified = cv2.remap(left_img, mapx, mapy, cv2.INTER_LINEAR)
        return rectified

    def rectify_right(self, right_img):
        new_size = (right_img.shape[1], right_img.shape[0])
        mapx, mapy = cv2.initUndistortRectifyMap(self.right_intrinsic, self.right_coeffs, self.r2, self.p2, new_size, cv2.CV_8U)
        rectified = cv2.remap(right_img, mapx, mapy, cv2.INTER_LINEAR)
        return rectified



    def save_params(self, xmlfile):
        storage = cv2.FileStorage(xmlfile, cv2.FILE_STORAGE_WRITE+cv2.FILE_STORAGE_FORMAT_XML)
        if not storage.isOpened():
            return False
        storage.write('left_intrinsic', self.left_intrinsic)
        storage.write('left_coeffs', self.left_coeffs)
        storage.write('right_intrinsic', self.right_intrinsic)
        storage.write('right_coeffs', self.right_coeffs)
        storage.write('rotation', self.rotation)
        storage.write('transform', self.transform)
        storage.write('r1', self.r1)
        storage.write('r2', self.r2)
        storage.write('p1', self.p1)
        storage.write('p2', self.p2)
        storage.write('q', self.q)
        storage.release()
        return True

    def _cal_corner_positions(self, pattern, grid_size):
        result = []
        for col in range(pattern[1]):
            for row in range(pattern[0]):
                result.append([col*grid_size[0], row*grid_size[1], 0])#x轴，y轴分别是图像坐标系的两个轴，z轴垂直于地面竖直向上，grid_size还是按照图像坐标系的习惯索引0即是x轴方向的单网格长度，索引1即是y轴方向的单网格长度
        return result

class VolumeMeasure:
    def __init__(self):
        self.cfg = ConfigParser()
        self.cfg.read('SWQT.ini')
        self.bFromTop = self.cfg.getboolean('binocular_cameras', 'bFromTop')
        self.bVisualized = self.cfg.getboolean('binocular_cameras', 'bVisualized')
        self.bCollected = self.cfg.getboolean('binocular_cameras', 'bCollected')
        self.bTimeRecorded = self.cfg.getboolean('binocular_cameras', 'bTimeRecorded')
        self.bmm = self.cfg.getboolean('binocular_cameras', 'bmm')
        self.bAuto = self.cfg.getboolean('binocular_cameras', 'bAuto')
        self.detectCount = self.cfg.getint('binocular_cameras', 'detectCount')
        self.xmlfile = self.cfg.get('binocular_cameras', 'xmlfile')
        self.camera_height = self.cfg.getfloat('binocular_cameras', 'cameraToGroundHeight')
        self.processThing = self.cfg.get('binocular_cameras', 'processThing')
        self.disparityStyle = self.cfg.get('binocular_cameras', 'disparityStyle')
        self.constructionStyle = self.cfg.get('binocular_cameras', 'constructionStyle')
        self.sizeCalculation = self.cfg.get('binocular_cameras', 'sizeCalculation')
        self.picWidth = self.cfg.getint('binocular_cameras', 'picWidth')
        self.picHeight = self.cfg.getint('binocular_cameras', 'picHeight')
        self.calibrator = stereo_calibrator(self.xmlfile, 'static')
        self.bShowed = self.bVisualized and not self.bTimeRecorded
        self.cut_rate = self.cfg.getfloat('binocular_cameras', 'cut_rate')

    def detect(self, left, right, left_src, right_src, target_area):
        if self.bCollected:
            cv2.imwrite('collect/left/%d.jpg'%self.detectCount, left_src)
            cv2.imwrite('collect/right/%d.jpg'%self.detectCount, right_src)
        if self.bTimeRecorded:
            startTime = time.time()
        rectified_left = self.calibrator.rectify_left(left)
        rectified_right = self.calibrator.rectify_right(right)
        rectified_left_src = self.calibrator.rectify_left(left_src)
        rectified_right_src = self.calibrator.rectify_right(right_src)
        if self.bShowed:
            cv2.imshow('rectified_l', rectified_left)
            cv2.imshow('rectified_r', rectified_right_src)
            cv2.waitKey()
        left_roi = rectified_left[int(self.cut_rate*self.picHeight):int((1-self.cut_rate)*self.picHeight), int(self.cut_rate*self.picWidth):int((1-self.cut_rate)*self.picWidth)]
        right_roi = rectified_right[int(self.cut_rate*self.picHeight):int((1-self.cut_rate)*self.picHeight), int(self.cut_rate*self.picWidth):int((1-self.cut_rate)*self.picWidth)]
        left_src_roi = rectified_left_src[int(self.cut_rate*self.picHeight):int((1-self.cut_rate)*self.picHeight), int(self.cut_rate*self.picWidth):int((1-self.cut_rate)*self.picWidth)]
        right_src_roi = rectified_right_src[int(self.cut_rate*self.picHeight):int((1-self.cut_rate)*self.picHeight), int(self.cut_rate*self.picWidth):int((1-self.cut_rate)*self.picWidth)]
        if self.bShowed:
            cv2.imshow('rectified_l', left_roi)
            cv2.imshow('rectified_r', right_src_roi)
            cv2.waitKey()
        if self.processThing == 'Contour looking for rects':
            if len(left_roi.shape) == 2:
                polys = _find_polys(left_roi, self.bShowed)
            else:
                return 1
        if self.disparityStyle == 'SGBMDisparity':
            disp = compute_disparity(left_src_roi, right_src_roi)
            cv2.imshow('disp', visualize_disparity(disp))
        if self.constructionStyle == 'Easy style':
            cloud = reproject(disp, self.calibrator.q, self.bmm)
        if len(rectified_left_src) == 1:
            resShow = np.stack((rectified_left_src,)*3, axis=-1)
        else:
            resShow = rectified_left_src.copy()
        if self.sizeCalculation == 'Rect calculation':
            for poly in polys:
                pt0 = cloud[poly[0][0][1]][poly[0][0][0]]
                pt1 = cloud[poly[1][0][1]][poly[1][0][0]]
                pt2 = cloud[poly[2][0][1]][poly[2][0][0]]
                pt3 = cloud[poly[3][0][1]][poly[3][0][0]]
                if self.bShowed:
                    print('(%d, %d)\'s world coordinate is (%d, %d, %d)'%(poly[0][0][0], poly[0][0][1], pt0[0], pt0[1], pt0[2]))
                    print('(%d, %d)\'s world coordinate is (%d, %d, %d)'%(poly[1][0][0], poly[1][0][1], pt1[0], pt1[1], pt1[2]))
                    print('(%d, %d)\'s world coordinate is (%d, %d, %d)'%(poly[2][0][0], poly[2][0][1], pt2[0], pt2[1], pt2[2]))
                    print('(%d, %d)\'s world coordinate is (%d, %d, %d)'%(poly[3][0][0], poly[3][0][1], pt3[0], pt3[1], pt3[2]))
                a = (_distance(pt0, pt1)+_distance(pt2, pt3))/2
                b = (_distance(pt0, pt3)+_distance(pt1, pt2))/2
                c = self.camera_height-min([pt0[2], pt1[2], pt2[2], pt3[2], self.camera_height])
                if not self.bmm:
                    a*=1000
                    b*=1000
                    c*=1000
                astr = '%.3fcm'%(a/10)
                bstr = '%.3fcm'%(b/10)
                cstr = '%.3fcm'%(c/10)
                vstr = '%.3fdm^3'%(a*b*c/1000000)
                if self.bTimeRecorded:
                    duration = time.time()-startTime
                    print('costs %ds, fps: %f'%(int(duration), 1/duration))
                elif self.bShowed:
                    print('a: %s; b: %s; c: %s; v: %s'%(astr, bstr, cstr, vstr))
                cv2.line(resShow, (poly[0][0][0]+int(self.picWidth*self.cut_rate), poly[0][0][1]+int(self.picHeight*self.cut_rate)), (poly[1][0][0]+int(self.picWidth*self.cut_rate), poly[1][0][1]+int(self.picHeight*self.cut_rate)), (0, 0, 255))
                cv2.putText(resShow, astr, (int((poly[0][0][0]+poly[1][0][0])/2+self.picWidth*self.cut_rate), int((poly[0][0][1]+poly[1][0][1])/2+self.picHeight*self.cut_rate)), cv2.FONT_HERSHEY_PLAIN, 1, (255, 255, 0))
                cv2.line(resShow, (poly[2][0][0] + int(self.picWidth * self.cut_rate), poly[2][0][1] + int(self.picHeight * self.cut_rate)), (poly[1][0][0] + int(self.picWidth * self.cut_rate), poly[1][0][1] + int(self.picHeight * self.cut_rate)), (0, 0, 255))
                cv2.putText(resShow, bstr, (int((poly[2][0][0] + poly[1][0][0]) / 2 + self.picWidth * self.cut_rate), int((poly[2][0][1] + poly[1][0][1]) / 2 + self.picHeight * self.cut_rate)), cv2.FONT_HERSHEY_PLAIN, 1, (255, 255, 0))
                cv2.putText(resShow, cstr, (int((poly[2][0][0] + poly[0][0][0]) / 2 + self.picWidth * self.cut_rate), int((poly[2][0][1] + poly[0][0][1]) / 2 + self.picHeight * self.cut_rate)), cv2.FONT_HERSHEY_PLAIN, 1, (0, 255, 0))
                VStrPosX = poly[1][0][0]+int(self.picWidth*self.cut_rate) if poly[1][0][0]+int(self.picWidth*self.cut_rate) < self.picWidth-40 else self.picWidth-40
                cv2.putText(resShow, vstr, (VStrPosX, poly[1][0][1]+int(self.picHeight*self.cut_rate)), cv2.FONT_HERSHEY_PLAIN, 1, (0, 255, 255))
                rect = cv2.boundingRect(poly)
                cv2.rectangle(resShow, (rect[0]+int(self.picWidth*self.cut_rate), rect[1]+int(self.picHeight*self.cut_rate)), (rect[0]+rect[2]+int(self.picWidth*self.cut_rate), rect[1]+rect[3]+int(self.picHeight*self.cut_rate)), (255, 0, 0))
        cv2.imshow('measure', resShow)
        if self.bCollected:
            cv2.imwrite('detectresult/%d.jpg'%self.detectCount, resShow)
        cv2.waitKey()
        self.detectCount+=1
        self.cfg.set('binocular_cameras', 'detectCount', str(self.detectCount))
        self.cfg.write(open('SWQT.ini', 'w'))
        cv2.destroyAllWindows()
        return 0