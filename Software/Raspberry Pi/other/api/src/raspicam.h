#ifndef _RaspiCam_H
#define _RaspiCam_H

#include "CameraBoardTypes.h"
class CameraBoard;
class RaspiCam{
	//the implementation of the camera
	CameraBoard *_impl;

public:
	RaspiCam();
	~RaspiCam();


	int initialize();
	int startCapture(ImageTakenCallback userCallback, unsigned char * preallocated_data, unsigned int offset, unsigned int length);
	void stopCapture();
	void close();
	int takePicture(unsigned char * preallocated_data, unsigned int length);
//	void bufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	void commitParameters();
	void setWidth(unsigned int width);
	void setHeight(unsigned int height);
	void setCaptureSize(unsigned int width, unsigned int height);
	void setBrightness(unsigned int brightness);
	void setQuality(unsigned int quality);
	void setRotation(int rotation);
	void setISO(int iso);
	void setSharpness(int sharpness);
	void setContrast(int contrast);
	void setSaturation(int saturation);
	void setEncoding(CAMERA_BOARD_ENCODING encoding);
	void setExposure(CAMERA_BOARD_EXPOSURE exposure);
	void setAWB(CAMERA_BOARD_AWB awb);
	void setImageEffect(CAMERA_BOARD_IMAGE_EFFECT imageEffect);
	void setMetering(CAMERA_BOARD_METERING metering);
	void setHorizontalFlip(bool hFlip);
	void setVerticalFlip(bool vFlip);
	
	unsigned int getWidth();
	unsigned int getHeight();
	unsigned int getBrightness();
	unsigned int getRotation();
	unsigned int getQuality();
	int getISO();
	int getSharpness();
	int getContrast();
	int getSaturation();
	CAMERA_BOARD_ENCODING getEncoding();
	CAMERA_BOARD_EXPOSURE getExposure();
	CAMERA_BOARD_AWB getAWB();
	CAMERA_BOARD_IMAGE_EFFECT getImageEffect();
	CAMERA_BOARD_METERING getMetering();
	bool isHorizontallyFlipped();
	bool isVerticallyFlipped();
	
};
#endif

