the calib_collect_pic.py file is for processing pictures used to serve calibration.

the following i will describe the file's situation in detail.

there are two options for you to choose:

###### 1. collection picture to calibrate
the calib_pic_count.txt records the picture number you collect.<br/>
the calib_collect_pic_conf.json records the directory you used to calibrate last 
time in case that you stopped half way last time. it promises you to continue your
last job.<br/>
so every time you run the script, it will ask you whether you would like to 
restart your calibrated picture collection as much as possible. and you make 
your choice.<br/>
then you could start your collection.
**Please notice your camera's index parameter in the function cv2.VideoCapture.**
and i think the camera as our eyes to see the
world. the left and right side of the camera are told by the means.<br/>
during your collection, when you get a good position for the camera and picture, 
just press the key 's' to save the calibration frame. then the two picture from
 two cameras will concatenate to one picture named from the number.<br/>
 when you want to pause the work, you could press the key 'q' or 'esc'.<br/>
 
 ###### 2. slicing the picture collected in the above step for calibration input in 
 matlab
you should decide the picture's slicing width.

if you want to quit the script, just interrupt it.
