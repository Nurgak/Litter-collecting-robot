#include <iostream>
#include "raspivid.h"
#include <sys/time.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;
using namespace cv;

int thres = 100;

unsigned long getmsofday()
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (long long)tv.tv_sec*1000 + tv.tv_usec/1000;
}

int main()
{
	const int width = 320;
	const int height = 240;
	RaspiVid v("/dev/video0", width, height);
	if (!v.initialize(RaspiVid::METHOD_MMAP))
	{
		cout << "Unable to initialize!\n";
		return -1;
	}
	v.setBrightness(50);
	v.startCapturing();
	long start = getmsofday();

	Mat canny;

	namedWindow("Camera", CV_WINDOW_AUTOSIZE);
	//createTrackbar("Threshold: ", "Camera", &thres, 500, NULL);

	while(1)
	{
		// Receive key-press updates, it is required if you want to output images
		if(waitKey(1) == 27)
		{
			cout << "Stop\n";
			break;
		}
		// Grab a frame from the vision API
		VideoBuffer buffer = v.grabFrame();
		// Put the frame into an OpenCV image matrix
		Mat image(height, width, CV_8UC1, buffer.data(), false);
		// Show the greyscale image with OpenCV on the screen
		imshow("Camera", image);
		// Threshold the image into a new matrix with a minimum value of 1 and maximum of 255
		//inRange(image, Scalar(1), Scalar(255), thresh);
		// Show the thresholded image with OpenCV on the screen
		//imshow("Threshold", thresh);
		// Apply Canny edge filter
		//Canny(image, canny, thres, thres * 2, 3);
		//imshow("Canny", canny);
		// Find all the contours in the thresholded image
		// The original thresholded image will be destroyed while finding contours
		//vector <vector<Point> > contours;
		//findContours(thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		// Output information
		long now = getmsofday();
		// The spaces in the end are required so that the line would be completely refreshed when less characters are shown
		//cout << "Frame #" << (i+1) << "   Frame Time: " << (now - start) << "ms   Estimated FPS: " << (1000 / (now-start)) << " FPS        \r";
                cout << "Frame Time: " << (now - start) << "ms\t   Estimated FPS: " << (1000 / (now-start)) << " FPS        \r";
		cout.flush();
		start = now;
	}
	v.destroy();
}
