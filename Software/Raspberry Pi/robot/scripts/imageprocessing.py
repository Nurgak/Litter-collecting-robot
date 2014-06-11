import io
import time
import picamera
import cv2
import sys
import numpy as np

width = 256
height = 128

cv2.namedWindow("Camera", flags=cv2.cv.CV_WINDOW_AUTOSIZE)

outputImageType = 6

#cascadeXml = cv2.CascadeClassifier('haar/training2/cars.xml')
#cascadeXml = cv2.CascadeClassifier('haar/face1.xml')
cascadeXml = cv2.CascadeClassifier('haar/bottle.xml')
#template = cv2.imread("template.png")

color = 36
def changeColorValue(newColor):
    global color
    color = newColor

colorWidth = 1
def changeColorWidth(newColorWidth):
    global colorWidth
    colorWidth = newColorWidth

x = -1
y = -1
def mouseCallback(event, _x, _y, flags, param):
    global x, y
    if event == cv2.cv.CV_EVENT_LBUTTONDOWN:
        x = _x
        y = _y

imgPos = 0
imgNeg = 0
def saveImage(type, img):
    global imgPos, imgNeg
    if type == True:
        # Let the user crop the ROI
        # ??? must this crop the actual image?

        # Save a positive image       
        cv2.imwrite("pos_%04i.jpg" %imgPos, img)
        imgPos += 1
    else:
        # Save a negative image
        cv2.imwrite("neg_%04i.jpg" %imgNeg, img)
        imgNeg += 1

cv2.setMouseCallback("Camera", mouseCallback)

cv2.createTrackbar("HSV", "Camera", 5, 115, changeColorValue)
cv2.createTrackbar("Color width", "Camera", 1, 20, changeColorWidth)

# Create the in-memory stream
stream = io.BytesIO()

camera = picamera.PiCamera()
camera.resolution = (width, height)
#camera.shutter_speed = 10000
# 100 to boost colors, -100 for grayscale image
camera.saturation = 100
# awb_mode: {u'horizon': 9, u'off': 0, u'cloudy': 3, u'shade': 4, u'fluorescent': 6, u'tungsten': 5, u'auto': 1, u'flash': 8, u'sunlight': 2, u'incandescent': 7}
#camera.awb_mode = u'off'
# exposure_modes: {u'auto': 1, u'fireworks': 12, u'verylong': 9, u'fixedfps': 10, u'backlight': 4, u'antishake': 11, u'snow': 7, u'sports': 6, u'nightpreview': 3, u'night': 2, u'beach': 8, u'spotlight': 5}
#camera.exposure_mode = u'fixedfps'
# meter_mode: {u'average': 0, u'spot': 1, u'matrix': 3, u'backlit': 2}
#camera.meter_mode = u'spot'
#camera.exposure_compensation = 10
#camera.framerate = 2
#camera.image_effect = {u'sketch': 6, u'posterise': 19, u'gpen': 11, u'colorbalance': 21, u'film': 14, u'pastel': 12, u'emboss': 8, u'denoise': 7, u'negative': 1, u'hatch': 10, u'colorswap': 17, u'colorpoint': 20, u'saturation': 16, u'blackboard': 5, u'blur': 15, u'posterize': 3, u'watercolor': 13, u'cartoon': 22, u'none': 0, u'whiteboard': 4, u'washedout': 18, u'solarize': 2, u'oilpaint': 9}
image_effects = ['sketch', 'posterise', 'gpen', 'colorbalance', 'film', 'pastel', 'emboss', 'denoise', 'negative', 'hatch', 'colorswap', 'none', 'whiteboard', 'washedout', 'solarize', 'oilpaint']
effectIndex = 0
#camera.sharpness = 0
#camera.video_stabilization = True

colorFloorL = np.array([8, 30, 30], np.uint8)
colorFloorH = np.array([28, 255, 255], np.uint8)
visibleMask = cv2.imread("mask3.png", 0)

start = time.time()

while True:
    camera.capture(stream, format='jpeg', use_video_port=True)

    # Construct a numpy array from the stream
    data = np.fromstring(stream.getvalue(), dtype=np.uint8)

    stream.truncate()
    stream.seek(0)

    # "Decode" the image from the array, preserving colour in BGR format
    img = cv2.imdecode(data, cv2.CV_LOAD_IMAGE_COLOR)

    if outputImageType == 0:
        colorL = np.array([color-colorWidth, 30, 30], np.uint8)
        colorH = np.array([color+colorWidth, 255, 255], np.uint8)

        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        colorMask = cv2.inRange(hsv, colorL, colorH)
        # Sum mask values vertically
        obj = np.sum(colorMask, axis = 0)
        # Sum values in 4 sectors
        split = [np.sum(obj[0:63]), np.sum(obj[64:127]), np.sum(obj[128:191]), np.sum(obj[192:255])]
        print split
        cv2.imshow("Camera", colorMask)

        if x != -1 or y != -1:
            color = hsv[y][x][0]
            cv2.setTrackbarPos("HSV", "Camera", color)
            x = -1
            y = -1

    elif outputImageType == 1:
        cv2.imshow("Camera", img)
    elif outputImageType == 2:
        gry = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        edge = cv2.Canny(gry, 100, 200)
        cv2.imshow("Camera", edge)
    elif outputImageType == 3:
        gry = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        haar = cascadeXml.detectMultiScale(gry, 1.1, 6)

        for (x,y,w,h) in haar:
            cv2.rectangle(img, (x,y), (x+w, y+h), (255, 255, 255), 1)
        cv2.imshow("Camera", img)
    elif outputImageType == 4:
        cv2.imshow("Camera", img)
    elif outputImageType == 5:
        gry = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        gry2 = gry[64:]
        ret, gry = cv2.threshold(gry2, 80, 255, cv2.THRESH_BINARY)
        cv2.imshow("Camera", gry)
    elif outputImageType == 6:
        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        colorMask = cv2.inRange(hsv, colorFloorL, colorFloorH)
        #colorMask = np.invert(colorMask[63:])
        colorMask = np.invert(colorMask)
        
        '''gry = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        b, g, r = cv2.split(img)
        k = cv2.bitwise_and(b, g)
        k = cv2.bitwise_and(k, r)
        c = k.astype(bool).astype(int) * 0xff
        #gry = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        #h, s, v = cv2.split(hsv)
        #gry = cv2.bitwise_and(gry, b)
        #gry = cv2.bitwise_and(gry, g)
        #gry = cv2.bitwise_and(gry, r)
        ret, obs = cv2.threshold(gry, 128, 255, cv2.THRESH_BINARY)'''
        
        cv2.erode(colorMask, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (7, 7)), colorMask, (-1, -1), 1)
        np.bitwise_and(colorMask, visibleMask, colorMask)
        
        sumVertical = np.sum(colorMask, axis = 0)
        # Sum values in 4 sectors
        splitHorizontal = [np.sum(sumVertical[0:63]), np.sum(sumVertical[64:127]), np.sum(sumVertical[128:191]), np.sum(sumVertical[192:255])]
        print "H", splitHorizontal
        
        sumHorizontal = np.sum(colorMask, axis = 1)
        splitVertical = [np.sum(sumHorizontal[0:31]), np.sum(sumHorizontal[32:63]), np.sum(sumHorizontal[64:95]), np.sum(sumHorizontal[96:127])]
        print "V", splitVertical
        
        '''img[:,:,0] = np.bitwise_and(img[:,:,0], colorMask)
        img[:,:,1] = np.bitwise_and(img[:,:,1], colorMask)
        img[:,:,2] = np.bitwise_and(img[:,:,2], colorMask)'''
        
        # horizontal lines
        cv2.line(img, (0, 31), (255, 31), (255, 0, 0))
        cv2.line(img, (0, 63), (255, 63), (255, 0, 0))
        cv2.line(img, (0, 95), (255, 95), (255, 0, 0))
        
        # vertical lines
        cv2.line(img, (63, 0), (63, 127), (255, 0, 0))
        cv2.line(img, (127, 0), (127, 127), (255, 0, 0))
        cv2.line(img, (191, 0), (191, 127), (255, 0, 0))
        
        #colorMask = colorMask[48:]
        cv2.imshow("Camera", colorMask)
    elif outputImageType == 7:
        #hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        b, g, r = cv2.split(img)
        k = cv2.bitwise_and(b, g)
        k = cv2.bitwise_and(k, r)
        c = k.astype(bool).astype(int) * 0xff
        #gry = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        #h, s, v = cv2.split(hsv)
        #gry = cv2.bitwise_and(gry, b)
        #gry = cv2.bitwise_and(gry, g)
        #gry = cv2.bitwise_and(gry, r)
        cv2.imshow("Camera", k)

    key = cv2.waitKey(10) & 0xff

    if key == 27:
        break
    elif key == 48: # '0'
		print "Obstacle detection"
		outputImageType = 0
    elif key == 49: # '1'
		print "Default"
		outputImageType = 1
    elif key == 50: # '2'
        print "Canny edge filter"
        outputImageType = 2
    elif key == 51: # '3'
        print "Bottle detection"
        outputImageType = 3
    elif key == 52: # '4'
        print "Obstacle detection 2"
        outputImageType = 4
    elif key == 53: # '5'
        print "Threshold"
        outputImageType = 5
    elif key == 54: # '6'
        print "Split channels"
        outputImageType = 6
    elif key == 55: # '7'
        print "Save picture"
        cv2.imwrite("template.png", img)
    elif key != 255:
		print str(key)+"\n"

    # count and show fps every second
    '''now = time.time()
    sys.stdout.write("%.02fFPS    \r" % (1 / (now - start)))
    sys.stdout.flush()
    start = now'''

camera.stop_preview()
cv2.destroyAllWindows()
