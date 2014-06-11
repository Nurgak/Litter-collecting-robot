#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <sys/time.h>

using namespace cv;
using namespace std;

unsigned long getmsofday()
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (long long)tv.tv_sec*1000 + tv.tv_usec/1000;
}

int main()
{
	VideoCapture camera(0);   //0 is the id of video device.0 if you have only one camera.
	camera.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

	if (!camera.isOpened())
	{
		cout << "cannot open camera";
	}

	namedWindow("Canny", 1);
	Mat edges;

	long start = getmsofday();

	while(true)
	{
		Mat frame;
		// get a new frame
		//camera.read(frame);
		camera >> frame;

		cvtColor(frame, edges, CV_BGR2GRAY);
		Canny(edges, edges, 0, 30, 3);

		//imshow("Canny", edges);
		imshow("Canny", frame);

		long now = getmsofday();
                cout << "Frame Time: " << (now - start) << "ms    Estimated FPS: " << (1000 / (now-start)) << " FPS        \r";
		cout.flush();
		start = now;

		if (waitKey(30) >= 0)
		{
			break;
		}
	}
	return 0;
}
