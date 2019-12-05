import os
import cv2
import numpy as np
import json
import time

def gen_time_dir():
    return time.strftime('%Y%m%d%H%M%S', time.localtime(time.time()))

def calib_collect_main():
    put_dir = 'calibed_pics'
    conf_file = 'calib_collect_pic_conf.json'
    last_dir = ''
    count_file = 'calib_pic_count.txt'
    if not os.path.exists(conf_file):
        last_dir = gen_time_dir()
        print('欢迎第一次标定图片采集，我们为您创建的图片目录是%s'%last_dir)
        config_dict = {'last_dir': last_dir}
        with open(conf_file, 'w') as f:
            json.dump(config_dict, f)
        with open(count_file, 'w') as f:
            f.write(str(0))
    else:
        with open(conf_file, 'r') as f:
            config_dict = json.load(f)
        last_dir = config_dict['last_dir']
        if os.path.exists(last_dir):
            while True:
                choice = input('您上次标定标定图片存储的目录是%s，本次采集还是要将图片保存在该目录中吗？(y/n)'%last_dir)
                if choice == 'y' or choice == 'Y':
                    break
                elif choice == 'n' or choice == 'N':
                    last_dir = gen_time_dir()
                    print('本次为您创建标定图片存储的目录：%s'%last_dir)
                    config_dict['last_dir'] = last_dir
                    with open(conf_file, 'w') as f:
                        json.dump(config_dict, f)
                    with open(count_file, 'w') as f:
                        f.write(str(0))
                    break
                else:
                    continue
        else:
            last_dir = gen_time_dir()
            print('这次为您创建标定图片的保存目录为%s'%last_dir)
            config_dict['last_dir'] = last_dir
            with open(conf_file, 'w') as f:
                json.dump(config_dict, f)
            with open(count_fiile, 'w') as f:
                f.write(str(0))
    if not os.path.exists(last_dir):
        os.mkdir(last_dir)
    cv2.namedWindow('left')
    cv2.namedWindow('right')
    cv2.moveWindow('left', 0, 0)
    cv2.moveWindow('right', 400, 0)
    left_camera = cv2.VideoCapture(2)
    right_camera = cv2.VideoCapture(0)
    with open(count_file, 'r') as f:
        count = int(f.readline().strip())
    while left_camera.isOpened() and right_camera.isOpened():
        retl, left_frame = left_camera.read()
        retr, right_frame = right_camera.read()
        if retl:
            cv2.imshow('left', left_frame)
        if retr:
            cv2.imshow('right', right_frame)
        key = cv2.waitKey(1)
        if key == ord('q') or key == 27:
            break
        elif key == ord('s'):
            cv2.imwrite('%s/%d.jpg'%(last_dir, count), np.concatenate([left_frame, right_frame], axis=1))
            count += 1
            with open(count_file, 'w') as f:
                f.write(str(count))
    left_camera.release()
    right_camera.release()
    cv2.destroyWindow('left')
    cv2.destroyWindow('right')

def prepro_for_matlab(width):
    while True:
        source_dir = input('可预处理的标定图片文件夹有：%s，请填写需要处理的目录名：'%str([d for d in os.listdir() if os.path.isdir(d)]))
        if os.path.exists(source_dir):
            break
        else:
            continue
    if not os.path.exists(source_dir+'left'):
        os.mkdir(source_dir+'left')
    if not os.path.exists(source_dir+'right'):
        os.mkdir(source_dir+'right')
    for pic in os.listdir(source_dir):
        img = cv2.imread(source_dir+'/'+pic)
        cv2.imwrite(source_dir+'left/'+pic, img[:, :width, :])
        cv2.imwrite(source_dir+'right/'+pic, img[:, width:, :])
    print('处理完毕！！！')

if __name__ == '__main__':
    while True:
        choice = int(input('请选择：1.标定    2.处理成matlab能使用的标定图片格式'))
        if choice == 1:
            calib_collect_main()
            break
        elif choice == 2:
            prepro_for_matlab(640)
            break
        else:
            continue
