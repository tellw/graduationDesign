from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QIcon, QImage, qRgb
import numpy as np
import cv2
import math

gray_color_table = [qRgb(i, i, i) for i in range(256)]

def bounding_rect(points):
    xmin = min(points[0])
    xmax = max(points[0])
    ymin = min(points[1])
    ymax = max(points[1])
    return (xmin, ymin, xmax, ymax)

def create_action(icon_path, submenu, parent_widget, shortcut, tip, menu, toolbar):
    the_action = QAction(QIcon(icon_path), submenu, parent_widget)
    the_action.setShortcut(shortcut)
    the_action.setStatusTip(tip)
    the_action.setToolTip(tip)
    menu.addAction(the_action)
    toolbar.addAction(the_action)
    return the_action

def npndarray2qimage(image, copy=False):
    if image is None:
        return QImage()
    if image.dtype == np.float64:
        image = np.uint8(image)
    if image.dtype == np.uint8:
        if len(image.shape) == 2:
            qim = QImage(image.data, image.shape[1], image.shape[0], image.strides[0], QImage.Format_Indexed8).rgbSwapped()
            qim.setColorTable(gray_color_table)
            return qim.copy() if copy else qim
        elif len(image.shape) == 3:
            if image.shape[2] == 3:
                qim = QImage(image.data, image.shape[1], image.shape[0], image.strides[0], QImage.Format_RGB888).rgbSwapped()
                return qim.copy() if copy else qim
            elif image.shape[2] == 4:
                qim = QImage(image.data, image.shape[1], image.shape[0], image.strides[0], QImage.Format_ARGB32).rgbSwapped()
                return qim.copy() if copy else qim

def convex_area(contour):
    return cv2.contourArea(cv2.convexHull(contour))

def _distance_2D(a, b):
    return math.sqrt((a[0]-b[0])**2+(a[1]-b[1])**2)

def regularize_output_approx(approx, image_width, image_height, bVisualized, toleratedMinDistCoeff=0.1):
    res = []
    firstPointDist = []
    for i in range(1, len(approx)):
        distance = _distance_2D(approx[0][0], approx[i][0])
        firstPointDist.append(distance)
        if bVisualized:
            print('distance between 0 and %d point is %f'%(i, distance))
    firstPointDist.sort()
    if len(approx) > 4:
        toleratedMinDist = toleratedMinDistCoeff*firstPointDist[-3]
        if bVisualized:
            print('toleratedMinDist is %f'%toleratedMinDist)
        for idx, firstdist in enumerate(firstPointDist):
            if firstdist < toleratedMinDist:
                approx = np.delete(approx, idx+1, 0)
        flag = [0 for i in range(len(approx))]
        for i in range(1, len(approx)):
            if flag[i] == 1:
                continue
            for j in range(i+1, len(approx)):
                if _distance_2D(approx[i][0], approx[j][0]) < toleratedMinDist:
                    flag[j] = 1
        red = []
        for i in range(len(approx)):
            if flag[i] == 0:
                red.append(approx[i])
        approx = np.array(red)
    if bVisualized:
        print('red has %d points: '%len(approx), end='')
        for a in approx:
            print('(%d, %d), '%(a[0][0], a[0][1]), end='')
        print()
    if len(approx) == 4:
        res.append(approx[0])
        approxFirstPointDist = []
        for i in range(3):
            approxFirstPointDist.append(_distance_2D(approx[0][0], approx[i+1][0]))
        approxFirstPointDistMaxIndex = approxFirstPointDist.index(max(approxFirstPointDist))
        approxFirstPointDistMinIndex = approxFirstPointDist.index(min(approxFirstPointDist))
        approxFirstPointDistMidIndex = 3-approxFirstPointDistMaxIndex-approxFirstPointDistMinIndex
        res.append(approx[approxFirstPointDistMinIndex+1])
        res.append(approx[approxFirstPointDistMaxIndex+1])
        res.append(approx[approxFirstPointDistMidIndex+1])
    elif len(approx) == 3:
        triAngleCos = []
        for i in range(3):
            triAngleCos.append(((approx[i][0][0]-approx[(i+2)%3][0][0])*(approx[i][0][0]-approx[(i+1)%3][0][0])+(approx[i][0][1]-approx[(i+2)%3][0][1])*(approx[i][0][1]-approx[(i+1)%3][0][1]))/(_distance_2D(approx[i][0], approx[(i+2)%3][0])*_distance_2D(approx[i][0], approx[(i+1)%3][0])))
        cosMinIndex = triAngleCos.index(min(triAngleCos))
        res.append(approx[cosMinIndex])
        res.append(approx[(cosMinIndex+1)%3])
        caledForthPointX = approx[(cosMinIndex+2)%3][0][0]+approx[(cosMinIndex+1)%3][0][0]-approx[cosMinIndex][0][0]
        caledForthPointY = approx[(cosMinIndex+2)%3][0][1]+approx[(cosMinIndex+1)%3][0][1]-approx[cosMinIndex][0][1]
        if caledForthPointX < 0:
            caledForthPointX = 0
        elif caledForthPointX >= image_width:
            caledForthPointX = image_width-1
        if caledForthPointY < 0:
            caledForthPointY = 0
        elif caledForthPointY >= image_height:
            caledForthPointY = image_height-1
        caledForthPoint = [[caledForthPointX, caledForthPointY]]
        res.append(caledForthPoint)
        res.append(approx[(cosMinIndex+2)%3])
    elif len(approx) > 4:
        disTo0 = []
        k = []
        for i in range(len(approx)):
            disTo0.append(math.sqrt(approx[i][0][0]**2+approx[i][0][1]**2))
            k.append(approx[i][0][1]/approx[i][0][0] if approx[i][0][0] != 0 else 10000)
        disMinIndex = disTo0.index(min(disTo0))
        res.append(approx[disMinIndex])
        disTo0.remove(disTo0[disMinIndex])
        k.remove(k[disMinIndex])
        approx = np.delete(approx, disMinIndex, 0)
        kMaxIndex = k.index(max(k))
        res.append(approx[kMaxIndex])
        disTo0.remove(disTo0[kMaxIndex])
        k.remove(k[kMaxIndex])
        approx = np.delete(approx, kMaxIndex, 0)
        disMaxIndex = disTo0.index(max(disTo0))
        res.append(approx[disMaxIndex])
        disTo0.remove(disTo0[disMaxIndex])
        k.remove(k[disMaxIndex])
        approx = np.delete(approx, disMaxIndex, 0)
        kMinIndex = k.index(min(k))
        res.append(approx[kMinIndex])
    return np.array(res)

def _find_polys(image, bVisualized, minAreaCoeff=0.02, approxPolyDPCoeff=0.05):
    polys = []
    contours, hierarchy = cv2.findContours(image, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)
    if bVisualized:
        contoursShow = np.zeros((image.shape[0], image.shape[1], 3), dtype='uint8')
        cv2.drawContours(contoursShow, contours, -1, (0, 255, 0))
        cv2.putText(contoursShow, 'found %d contours'%len(contours), (0, 40), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 1)
        cv2.imshow('Contour Show', contoursShow)
        cv2.waitKey()
    min_area = minAreaCoeff*image.shape[0]*image.shape[1]
    if bVisualized:
        print('min_area=%f'%min_area)
    for idx, contour in enumerate(contours):
        convex_esti_area = convex_area(contour)
        if bVisualized:
            print('%dth seeking: %f'%(idx+1, convex_esti_area), end=' ')
        if convex_esti_area < min_area:
            if bVisualized:
                print('the area of the contour is invalid')
            continue
        leng = cv2.arcLength(contour, True)
        approx = cv2.approxPolyDP(contour, leng*approxPolyDPCoeff, False)
        if bVisualized:
            contourShow = np.zeros((image.shape[0], image.shape[1], 3), dtype='uint8')
            cv2.drawContours(contourShow, contour, -1, (255, 255, 0))
            cv2.drawContours(contourShow, approx, -1, (0, 255, 255))
            cv2.imshow('contourAndApproxShow', contourShow)
            print('\napprox has %d points, '%len(approx), end ='')
            for approx_point in approx:
                print('(%d, %d), '%(approx_point[0][0], approx_point[0][1]), end='')
            print()
            cv2.waitKey()
        reg = regularize_output_approx(approx, image.shape[1], image.shape[0], bVisualized)
        if len(reg) <= 2:
            continue
        if bVisualized:
            print('reg has %d points, '%len(reg),end='')
            for i in range(4):
                print('(%d, %d), '%(reg[i][0][0], reg[i][0][1]), end='')
            print()
        polys.append(reg)
        rect = cv2.boundingRect(reg)
        if bVisualized:
            c = np.zeros((image.shape[0], image.shape[1], 3), dtype='uint8')
            for pt in contour:
                c[pt[0][1]][pt[0][0]] = (255, 0, 0)
            for pt in reg:
                cv2.circle(c, (pt[0][0], pt[0][1]), 5, (255, 255, 0))
            cv2.rectangle(c, (rect[0],rect[1]), (rect[0]+rect[2], rect[1]+rect[3]), (0, 0, 255))
            cv2.fillPoly(c, reg, (255, 255, 255))
            cv2.imshow('patternDetect', c)
            cv2.waitKey()
    return np.array(polys)

def compute_disparity(left, right):
    win_size = 3
    matcher_left = cv2.StereoSGBM_create(0, 16*7, 2, 8*2*win_size*win_size, 32*10*win_size*win_size, 8, 31, 1, 8, 8, cv2.STEREO_SGBM_MODE_SGBM_3WAY)
    displ = matcher_left.compute(left, right).astype(np.float32)/16.0
    #true_displ = displ.astype(np.uint8)
    matcher_right = cv2.ximgproc.createRightMatcher(matcher_left)
    dispr = matcher_right.compute(left, right).astype(np.float32)/16.0
    #true_dispr = dispr.astype(np.uint8)
    filter = cv2.ximgproc.createDisparityWLSFilter(matcher_left)
    filter.setLambda(3000)
    filter.setSigmaColor(10)
    #filtered_displ = filter.filter(true_displ, left, disparity_map_right=true_dispr)
    filtered_displ = filter.filter(displ, left, disparity_map_right=dispr)
    return filtered_displ

def visualize_disparity(disp):
    visual_disp = disp.copy()
    minV = np.min(visual_disp)
    maxV = np.max(visual_disp)
    scale = 255/(maxV-minV)
    visual_disp = (visual_disp-minV)*scale
    return visual_disp
    #cv2.normalize(disp, disp, 0, 255, cv2.NORM_MINMAX)
    #return disp

def reproject(disparity, q, bmm, zclamp=[100, 1000]):
    print(disparity.shape)
    image3d = cv2.reprojectImageTo3D(disparity, q).reshape(disparity.shape[0], disparity.shape[1], 3)
    if not bmm:
        zclamp[0] /= 1000
        zclamp[1] /= 1000
    for r_idx, ir in enumerate(image3d):
        for i_idx, i in enumerate(ir):
            if i[2] < zclamp[0] or i[2] > zclamp[1]:
                image3d[r_idx][i_idx] = np.array([0, 0, 0])
    return image3d

def _distance(p1, p2):
    return math.sqrt((p1[0]-p2[0])**2+(p1[1]-p2[1])**2+(p1[2]-p2[2])**2)

def messageBox_info(msg, parent):
    message = QMessageBox(QMessageBox.Information, 'Information', msg, QMessageBox.Yes, parent)
    message.exec_()