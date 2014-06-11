#ifndef _RaspiVideo_H
#define _RaspiVideo_H

#include "VideoBuffer.h"

class VideoBoard;
class RaspiVid{
	//the implementation of the camera
	VideoBoard *_impl;

public:
	static const unsigned int METHOD_MMAP;
	static const unsigned int METHOD_READ;
	RaspiVid(const char * device, int width, int height);
	~RaspiVid();
	
	/** @return the width of the frame */
	int getWidth();
	
	/** @return the height of the frame */
	int getHeight();
	
	/** Initializes the camera with the specified method
	 * @param method the method to use for capturing, either METHOD_MMAP or METHOD_READ
	 */
	bool initialize(unsigned int method);
	
	/** Closes any resources allocated on initialize */
	void destroy();
	
	/** Begin capturing to be able to grab a frame */
	void startCapturing();
	
	/** Grab a frame from the camera
	 * @return a video buffer that contains the data from the image */
	VideoBuffer grabFrame();
	
	/** Releases a frame so that resources are freed. Done automatically by VideoBuffer. */
	void releaseFrame(VideoBuffer * buffer);
	
	/** Sets the brightness of the camera
	 * @param brightness the brightness to set from 0 to 100 inclusive */
	void setBrightness(int brightness);
};
#endif // _RaspiVideo_H
