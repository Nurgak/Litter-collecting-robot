#ifndef VIDEOBOARD_H
#define VIDEOBOARD_H

#include "VideoBoardTypes.h"

class VideoBoard {
	private:
	const char * device;
	bool initialized;
	int fd;
	int width;
	int height;
	unsigned int method;
	unsigned int numBuffers;
	unsigned int bufferSize;
	unsigned char * preAllocatedBuffer;
	Buffer * buffers;
	struct v4l2_buffer buf;
	
	int xioctl(int fd, int request, void * arg);
	int readFrame(Buffer * buffer);
	int readFrameMmap(Buffer * buffer);
	//bool readSelect();
	bool createBuffers(unsigned int count);
	bool openDevice();
	bool initializeDevice();
	bool initMmap();
	
	public:
	static const unsigned int METHOD_MMAP;
	static const unsigned int METHOD_READ;
	// Constructor / Destructor
	VideoBoard(const char * device, int width, int height);
	~VideoBoard();
	
	// Get/Set
	int getWidth();
	int getHeight();
	void setBrightness(int brightness);
	
	// Camera functions
	bool initialize(unsigned int method);
	void startCapturing();
	VideoBuffer grabFrame();
	void releaseFrame(VideoBuffer * buffer);
	void destroy();
};

#endif // VIDEOBOARD_H

