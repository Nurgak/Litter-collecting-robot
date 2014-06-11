/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, James Hughes
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CAMERA_BOARD_H
#define CAMERA_BOARD_H

#include "CameraBoardTypes.h"

#define MMAL_CAMERA_CAPTURE_PORT 2
#define STILLS_FRAME_RATE_NUM 30
#define STILLS_FRAME_RATE_DEN 1

class CameraBoard {
	private:
	
	// Low-level Camera Board variables
	CAMERA_BOARD_USERDATA * userdata;	// Pointer to the userdata for the encoder output port
	MMAL_COMPONENT_T * camera;					// Pointer to the camera componen
	MMAL_COMPONENT_T * encoder;					// Pointer to the encoder component
	MMAL_CONNECTION_T * encoder_connection;		// Connection from the camera to the encoder
	MMAL_POOL_T * encoder_pool;					// Pointer to the pool of buffers used by encoder output port
	MMAL_PORT_T * camera_still_port;			// Pointer to the still port on the camera
	MMAL_PORT_T * encoder_input_port;			// Pointer to the input port on the encoder
	MMAL_PORT_T * encoder_output_port;			// Pointer to the output port on the encoder
	// Control variables
	unsigned int width;
	unsigned int height;
	unsigned int rotation; // 0 to 359
	unsigned int brightness; // 0 to 100
	unsigned int quality; // 0 to 100
	int iso;
	int sharpness; // -100 to 100
	int contrast; // -100 to 100
	int saturation; // -100 to 100
	CAMERA_BOARD_ENCODING encoding;
	CAMERA_BOARD_EXPOSURE exposure;
	CAMERA_BOARD_AWB awb;
	CAMERA_BOARD_IMAGE_EFFECT imageEffect;
	CAMERA_BOARD_METERING metering;
	bool horizontalFlip;
	bool verticalFlip;
	// State variables
	bool closed;
	bool changedSettings;
	// Error variables
	std::ostream * errorStream;
	
	// Control Parameter methods
	MMAL_FOURCC_T convertEncoding(CAMERA_BOARD_ENCODING encoding);
	MMAL_PARAM_EXPOSUREMETERINGMODE_T convertMetering(CAMERA_BOARD_METERING metering);
	MMAL_PARAM_EXPOSUREMODE_T convertExposure(CAMERA_BOARD_EXPOSURE exposure);
	MMAL_PARAM_AWBMODE_T convertAWB(CAMERA_BOARD_AWB awb);
	MMAL_PARAM_IMAGEFX_T convertImageEffect(CAMERA_BOARD_IMAGE_EFFECT imageEffect);
	void commitBrightness();
	void commitQuality();
	void commitRotation();
	void commitISO();
	void commitSharpness();
	void commitContrast();
	void commitSaturation();
	void commitExposure();
	void commitAWB();
	void commitImageEffect();
	void commitMetering();
	void commitFlips();
	// Initialize/Start/Stop methods
	int startCapture();
	int createCamera();
	int createEncoder();
	int initStillPort();
	int initVideoPort();
	void destroyCamera();
	void destroyEncoder();
	void setDefaults();
	// Miscelaneous
	MMAL_STATUS_T connectPorts(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection);
	
	public:
	const char * API_NAME;
	CameraBoard() {
		API_NAME = "CameraBoard";
		setDefaults();
		setErrorOutput(1); // Standard output stream
		camera = NULL;
		encoder = NULL;
		encoder_connection = NULL;
		encoder_pool = NULL;
		camera_still_port = NULL;
		encoder_input_port = NULL;
		encoder_output_port = NULL;
		closed = true;
	}
	
	~CameraBoard() {
		if (!closed)
			close();
	}
	
	// Initialize/Start/Stop methods
	int initialize();
	int startCapture(ImageTakenCallback userCallback, unsigned char * preallocated_data, unsigned int offset, unsigned int length);
	int takePicture(unsigned char * preallocated_data, unsigned int length);
	void stopCapture();
	void close();
	// Miscelaneous
	void setErrorOutput(int mode);
	// Control Parameter methods
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

#endif // CAMERA_BOARD_H
