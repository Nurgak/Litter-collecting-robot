#!/usr/bin/python
import time
import cv2
import sys
import numpy as np
import picamera
import io
import argparse

import comm

width = 300
height = 200

if len(sys.argv) > 1:
    pass

robot = comm.RobotComm()
robot.connect()

#camera = cv2.VideoCapture(0)
#camera.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH, width)
#camera.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT, height)
#camera.set(cv2.cv.CV_CAP_PROP_BRIGHTNESS, 0.8)
# exposure not supported by the raspicam
#camera.set(cv2.cv.CV_CAP_PROP_EXPOSURE, 0.5)

camera = picamera
camera = picamera.PiCamera()
camera.resolution = (width, height)

#cascadeXml = cv2.CascadeClassifier('haar/bottle.xml')
cascadeXml = cv2.CascadeClassifier('haar/training2/samples.xml')
#cascadeXml = cv2.CascadeClassifier('haar/face2.xml')

# Start the CSI camera driver with
#uv4l --driver raspicam --auto-video_nr --width 640 --height 480 --encoding jpeg

# define the LD_PRELOAD system variable
#export LD_PRELOAD=/usr/lib/uv4l/uv4lext/armv6l/libuv4lext.so

stream = io.BytesIO()

cv2.namedWindow("camera", cv2.cv.CV_WINDOW_AUTOSIZE)

start = time.time()

while True:
    # get image from camera in BGR format

    camera.capture(stream, format='jpeg', use_video_port=True)

    # Construct a numpy array from the stream
    data = np.fromstring(stream.getvalue(), dtype=np.uint8)

    stream.truncate()
    stream.seek(0)

    # "Decode" the image from the array, preserving colour in BGR format
    img = cv2.imdecode(data, cv2.CV_LOAD_IMAGE_COLOR)

    #ret, img = camera.read()
    #if img == None:
    #    print "Error getting an image from the camera"

    # convert image to gray
    gry = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # convert image to HSV format
    #hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    # make a mask for yellow elemets from HSV image
    #maskYel = cv2.inRange(hsv, yellowL, yellowH)

    # apply mask to original image
    #yel = cv2.bitwise_and(img, img, mask = maskYel)
    
    # make a mask for red elemets from HSV image
    #maskRed = cv2.inRange(hsv, redL, redH)
    
    # make a mask for green elemets from HSV image
    #maskGreen = cv2.inRange(hsv, greenL, greenH)

    '''mom = cv2.moments(gry)
    cx = int(mom['m10'] / mom['m00'])
    cy = int(mom['m01'] / mom['m00'])
    cv2.circle(img, (cx, cy), 3, 255, -1)'''

    #canny = cv2.Canny(gry, 50, 100)

    #cv2.dilate(maskRed, kernel, iterations = 10)

    #cv2.erode(maskRed, kernel, iterations = 10)

    #cv2.morphologyEx(maskRed, cv2.MORPH_OPEN, kernel)

    # apply mask to original image
    #red = cv2.bitwise_and(img, img, mask = maskRed)

    # do haarcascades for object detection
    haar = cascadeXml.detectMultiScale(gry, 1.3, 5)
    if len(haar):
        robot.led(1)
    else:
        robot.led(0)

    for (x,y,w,h) in haar:
        cv2.rectangle(img, (x,y), (x+w, y+h), (255, 255, 255), 1)

    '''both = np.zeros((height, width * 2, 3), np.uint8)
    both[:height, :width] = img
    both[:height, width:width * 2, 0] |= canny
    both[:height, width:width * 2, 1] |= canny
    both[:height, width:width * 2, 2] |= canny
    both[:height, width:width * 2, 1] |= maskGreen
    both[:height, width:width * 2, 2] |= maskRed'''

    # output to screen
    #cv2.imshow("camera", both)
    cv2.imshow("camera", img)

    # count and show fps every second
    now = time.time()
    sys.stdout.write("%.02fFPS    \r" % (1 / (now - start)))
    sys.stdout.flush()
    start = now

    # check for ESC key press, only check the first 8 bits
    key = cv2.waitKey(10) & 0xff
    if key == 27:
        break

cv2.destroyAllWindows()
robot.disconnect()
