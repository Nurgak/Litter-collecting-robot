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
#include "CameraBoard.h"
#include "mmal/mmal_buffer.h"
#include "mmal/mmal_default_components.h"
#include "mmal/mmal_util.h"
#include "mmal/mmal_util_params.h"
#include <iostream>
#include <semaphore.h>
using namespace std;

static void control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
	CAMERA_BOARD_USERDATA *userdata = (CAMERA_BOARD_USERDATA*)port->userdata;
	if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED) {
	} else {
		// Unexpected control callback event!
		(userdata->unexpectedControlCallbacks)++;
	}
	mmal_buffer_header_release(buffer);
}

static void buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
	CAMERA_BOARD_USERDATA *userdata = (CAMERA_BOARD_USERDATA*)port->userdata;
	if (userdata == NULL || userdata->cameraBoard == NULL) {
		
	} else {
		bool isRGB = userdata->cameraBoard->getEncoding() == CAMERA_BOARD_ENCODING_RGB;
		bool canCopy = true;
		if (userdata->offset + buffer->length >= userdata->length) {
			canCopy = false;
			if (userdata->errorStream)
				*userdata->errorStream << userdata->cameraBoard->API_NAME << ": Buffer provided was too small! Failed to copy data into buffer.\n";
			userdata->cameraBoard = NULL;
		}
		mmal_buffer_header_mem_lock(buffer);
		if (canCopy) {
			unsigned int i = (isRGB) ? 54 : 0;
			for (; i < buffer->length; i++, userdata->offset++) {
				userdata->data[userdata->offset] = buffer->data[i];
			}
		}
		mmal_buffer_header_mem_unlock(buffer);
		if ((buffer->flags & (MMAL_BUFFER_HEADER_FLAG_FRAME_END | MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED)) != 0) {
			if (userdata->mutex == NULL) {
				userdata->imageCallback(userdata->data, userdata->startingOffset, userdata->length - userdata->startingOffset);
			} else {
				sem_post(userdata->mutex);
			}
		}
	}
	mmal_buffer_header_release(buffer);
	if (port->is_enabled) {
		MMAL_BUFFER_HEADER_T *new_buffer = mmal_queue_get(userdata->encoderPool->queue);
		if (new_buffer) mmal_port_send_buffer(port, new_buffer);
	}
}

void CameraBoard::setDefaults() {
	width = 640;
	height = 480;
	encoding = CAMERA_BOARD_ENCODING_BMP;
	encoder = NULL;
	encoder_connection = NULL;
	sharpness = 0;
	contrast = 0;
	brightness = 50;
	quality = 85;
	saturation = 0;
	iso = 400;
	//videoStabilisation = 0;
	//exposureCompensation = 0;
	exposure = CAMERA_BOARD_EXPOSURE_AUTO;
	metering = CAMERA_BOARD_METERING_AVERAGE;
	awb = CAMERA_BOARD_AWB_AUTO;
	imageEffect = CAMERA_BOARD_IMAGE_EFFECT_NONE;
	//colourEffects.enable = 0;
	//colourEffects.u = 128;
	//colourEffects.v = 128;
	rotation = 0;
	changedSettings = true;
	horizontalFlip = false;
	verticalFlip = false;
	//roi.x = params->roi.y = 0.0;
	//roi.w = params->roi.h = 1.0;
}

void CameraBoard::commitParameters() {
	if (!changedSettings) return;
	commitSharpness();
	commitContrast();
	commitBrightness();
	commitQuality();
	commitSaturation();
	commitISO();
	commitExposure();
	commitMetering();
	commitAWB();
	commitImageEffect();
	commitRotation();
	commitFlips();
	// Set Video Stabilization
	if (mmal_port_parameter_set_boolean(camera->control, MMAL_PARAMETER_VIDEO_STABILISATION, 0) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set video stabilization parameter.\n";
	// Set Exposure Compensation
	if (mmal_port_parameter_set_int32(camera->control, MMAL_PARAMETER_EXPOSURE_COMP , 0) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set exposure compensation parameter.\n";
	// Set Color Efects
	MMAL_PARAMETER_COLOURFX_T colfx = {{MMAL_PARAMETER_COLOUR_EFFECT,sizeof(colfx)}, 0, 0, 0};
	colfx.enable = 0;
	colfx.u = 128;
	colfx.v = 128;
	if (mmal_port_parameter_set(camera->control, &colfx.hdr) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set color effects parameter.\n";
	// Set ROI
	MMAL_PARAMETER_INPUT_CROP_T crop = {{MMAL_PARAMETER_INPUT_CROP, sizeof(MMAL_PARAMETER_INPUT_CROP_T)}, {0, 0, 0, 0}};
	crop.rect.x = (65536 * 0);
	crop.rect.y = (65536 * 0);
	crop.rect.width = (65536 * 1);
	crop.rect.height = (65536 * 1);
	if (mmal_port_parameter_set(camera->control, &crop.hdr) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set ROI parameter.\n";
	// Set encoder encoding
	if (encoder_output_port != NULL) {
		encoder_output_port->format->encoding = convertEncoding(encoding);
		mmal_port_format_commit(encoder_output_port);
	}
	changedSettings = false;
}

MMAL_STATUS_T CameraBoard::connectPorts(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection) {
	MMAL_STATUS_T status =  mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	if (status == MMAL_SUCCESS) {
		status =  mmal_connection_enable(*connection);
		if (status != MMAL_SUCCESS)
			mmal_connection_destroy(*connection);
	}

	return status;
}

int CameraBoard::createCamera() {
	if (mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera)) {
		if (errorStream)
			*errorStream << API_NAME << ": Failed to create camera component.\n";
		destroyCamera();
		return -1;
	}
	
	if (!camera->output_num) {
		if (errorStream)
			*errorStream << API_NAME << ": Camera does not have output ports!\n";
		destroyCamera();
		return -1;
	}
	
	camera_still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];
	
	// Enable the camera, and set the control callback function
	if (mmal_port_enable(camera->control, control_callback)) {
		if (errorStream)
			*errorStream << API_NAME << ": Could not enable control port.\n";
		destroyCamera();
		return -1;
	}
	
	MMAL_PARAMETER_CAMERA_CONFIG_T camConfig = {
		{MMAL_PARAMETER_CAMERA_CONFIG, sizeof(camConfig)},
		width, // max_stills_w
		height, // max_stills_h
		0, // stills_yuv422
		1, // one_shot_stills
		width, // max_preview_video_w
		height, // max_preview_video_h
		3, // num_preview_video_frames
		0, // stills_capture_circular_buffer_height
		0, // fast_preview_resume
		MMAL_PARAM_TIMESTAMP_MODE_RESET_STC // use_stc_timestamp
	};
	if (mmal_port_parameter_set(camera->control, &camConfig.hdr) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Could not set port parameters.\n";
	
	commitParameters();
	
	MMAL_ES_FORMAT_T * format = camera_still_port->format;
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->es->video.width = width;
	format->es->video.height = height;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = width;
	format->es->video.crop.height = height;
	format->es->video.frame_rate.num = STILLS_FRAME_RATE_NUM;
	format->es->video.frame_rate.den = STILLS_FRAME_RATE_DEN;
	
	if (camera_still_port->buffer_size < camera_still_port->buffer_size_min)
		camera_still_port->buffer_size = camera_still_port->buffer_size_min;
	
	camera_still_port->buffer_num = camera_still_port->buffer_num_recommended;
	
	if (mmal_port_format_commit(camera_still_port)) {
		if (errorStream)
			*errorStream << API_NAME << ": Camera still format could not be set.\n";
		destroyCamera();
		return -1;
	}
	
	if (mmal_component_enable(camera)) {
		if (errorStream)
			*errorStream << API_NAME << ": Camera component could not be enabled.\n";
		destroyCamera();
		return -1;
	}
	
	if (!(encoder_pool = mmal_port_pool_create(camera_still_port, camera_still_port->buffer_num, camera_still_port->buffer_size))) {
		if (errorStream)
			*errorStream << API_NAME << ": Failed to create buffer header pool for camera.\n";
		destroyCamera();
		return -1;
	}
	
	return 0;
}

int CameraBoard::createEncoder() {
	if (mmal_component_create(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER, &encoder)) {
		if (errorStream)
			*errorStream << API_NAME << ": Could not create encoder component.\n";
		destroyEncoder();
		return -1;
	}
	if (!encoder->input_num || !encoder->output_num) {
		if (errorStream)
			*errorStream << API_NAME << ": Encoder does not have input/output ports.\n";
		destroyEncoder();
		return -1;
	}
	
	encoder_input_port = encoder->input[0];
	encoder_output_port = encoder->output[0];
	
	mmal_format_copy(encoder_output_port->format, encoder_input_port->format);
	encoder_output_port->format->encoding = convertEncoding(encoding); // Set output encoding
	encoder_output_port->buffer_size = encoder_output_port->buffer_size_recommended;
	if (encoder_output_port->buffer_size < encoder_output_port->buffer_size_min)
		encoder_output_port->buffer_size = encoder_output_port->buffer_size_min;
	encoder_output_port->buffer_num = encoder_output_port->buffer_num_recommended;
	if (encoder_output_port->buffer_num < encoder_output_port->buffer_num_min)
		encoder_output_port->buffer_num = encoder_output_port->buffer_num_min;
	
	if (mmal_port_format_commit(encoder_output_port)) {
		if (errorStream)
			*errorStream << API_NAME << ": Could not set format on encoder output port.\n";
		destroyEncoder();
		return -1;
	}
	if (mmal_component_enable(encoder)) {
		if (errorStream)
			*errorStream << API_NAME << ": Could not enable encoder component.\n";
		destroyEncoder();
		return -1;
	}
	if (!(encoder_pool = mmal_port_pool_create(encoder_output_port, encoder_output_port->buffer_num, encoder_output_port->buffer_size))) {
		if (errorStream)
			*errorStream << API_NAME << ": Failed to create buffer header pool for encoder output port.\n";
		destroyEncoder();
		return -1;
	}
	return 0;
}

void CameraBoard::destroyCamera() {
	if (camera) {
		mmal_component_destroy(camera);
		camera = NULL;
	}
}

void CameraBoard::destroyEncoder() {
	if (encoder_pool)
		mmal_port_pool_destroy(encoder->output[0], encoder_pool);
	if (encoder) {
		mmal_component_destroy(encoder);
		encoder = NULL;
	}
}

int CameraBoard::initialize() {
	if (!closed)
		return -1;
	if (createCamera()) {
		if (errorStream)
			*errorStream << API_NAME << ": Failed to create camera component.\n";
		destroyCamera();
		return -1;
	} else if (createEncoder()) {
		if (errorStream)
			*errorStream << API_NAME << ": Failed to create encoder component.\n";
		destroyCamera();
		return -1;
	} else {
		camera_still_port	= camera->output[MMAL_CAMERA_CAPTURE_PORT];
		encoder_input_port  = encoder->input[0];
		encoder_output_port = encoder->output[0];
		if (connectPorts(camera_still_port, encoder_input_port, &encoder_connection) != MMAL_SUCCESS) {
			if (errorStream)
				*errorStream << "ERROR: Could not connect encoder ports!\n";
			return -1;
		}
		userdata = new CAMERA_BOARD_USERDATA;
		userdata->cameraBoard = this;
		userdata->encoderPool = encoder_pool;
		userdata->mutex = new sem_t;
		userdata->errorStream = NULL;
		userdata->data = NULL;
		userdata->bufferPosition = 0;
		userdata->offset = 0;
		userdata->startingOffset = 0;
		userdata->length = 0;
		userdata->unexpectedControlCallbacks = 0;
		userdata->imageCallback = NULL;
	}
	return 0;
}

int CameraBoard::takePicture(unsigned char * preallocated_data, unsigned int length) {
	int ret = 0;
	sem_init(userdata->mutex, 0, 0);
	// Initialize userdata
	userdata->errorStream = errorStream;
	userdata->data = preallocated_data;
	userdata->bufferPosition = 0;
	userdata->offset = 0;
	userdata->startingOffset = 0;
	userdata->length = length;
	encoder_output_port->userdata = (struct MMAL_PORT_USERDATA_T *) userdata;
	if ((ret = startCapture()) != 0) return ret;
	sem_wait(userdata->mutex);
	stopCapture();
	return ret;
}

int CameraBoard::startCapture(ImageTakenCallback userCallback, unsigned char * preallocated_data, unsigned int offset, unsigned int length) {
	// Initialize userdata
	userdata->mutex = NULL;
	userdata->errorStream = errorStream;
	userdata->data = preallocated_data;
	userdata->bufferPosition = 0;
	userdata->offset = offset;
	userdata->startingOffset = offset;
	userdata->length = length;
	userdata->imageCallback = userCallback;
	encoder_output_port->userdata = (struct MMAL_PORT_USERDATA_T *) userdata;
	startCapture();
}

int CameraBoard::startCapture() {
	// If the parameters were changed and this function wasn't called, it will be called here
	// However if the parameters weren't changed, the function won't do anything - it will return right away
	commitParameters();
	
	if (encoder_output_port->is_enabled) {
		if (errorStream)
			*errorStream << API_NAME << ": Could not enable encoder output port. Try waiting longer before attempting to take another picture.\n";
		return -1;
	}
	if (mmal_port_enable(encoder_output_port, buffer_callback) != MMAL_SUCCESS) {
		if (errorStream)
			*errorStream << API_NAME << ": Could not enable encoder output port.\n";
		return -1;
	}
	int num = mmal_queue_length(encoder_pool->queue);
	for (int b = 0; b < num; b++) {
		MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(encoder_pool->queue);
		
		if (!buffer)
			if (errorStream)
				*errorStream << API_NAME << ": Could not get buffer (#" << b << ") from pool queue.\n";
		
		if (mmal_port_send_buffer(encoder_output_port, buffer)!= MMAL_SUCCESS)
			if (errorStream)
				*errorStream << API_NAME << ": Could not send a buffer (#" << b << ") to encoder output port.\n";
	}
	if (mmal_port_parameter_set_boolean(camera_still_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS) {
		if (errorStream)
			*errorStream << API_NAME << ": Failed to start capture.\n";
		return -1;
	}
	return 0;
}

void CameraBoard::stopCapture() {
	if (!encoder_output_port->is_enabled) return;
	if (mmal_port_disable(encoder_output_port)) return;
}

void CameraBoard::close() {
	stopCapture();
	delete userdata;
	if (camera_still_port->is_enabled)
		mmal_port_disable(camera_still_port);
	destroyEncoder();
	destroyCamera();
	closed = true;
}

void CameraBoard::setErrorOutput(int mode) {
	switch (mode) {
		case 0:
			errorStream = NULL;
			break;
		case 1:
			errorStream = &cout;
			break;
		case 2:
			errorStream = &cerr;
			break;
		default:
			errorStream = &cout;
			break;
	}
}

void CameraBoard::setWidth(unsigned int width) {
	this->width = width;
	changedSettings = true;
}

void CameraBoard::setHeight(unsigned int height) {
	this->height = height;
	changedSettings = true;
}

void CameraBoard::setCaptureSize(unsigned int width, unsigned int height) {
	setWidth(width);
	setHeight(height);
}

void CameraBoard::setBrightness(unsigned int brightness) {
	if (brightness > 100)
		brightness = brightness % 100;
	this->brightness = brightness;
	changedSettings = true;
}

void CameraBoard::setQuality(unsigned int quality) {
	if (quality > 100)
		quality = 100;
	this->quality = quality;
	changedSettings = true;
}

void CameraBoard::setRotation(int rotation) {
	while (rotation < 0)
		rotation += 360;
	if (rotation >= 360)
		rotation = rotation % 360;
	this->rotation = rotation;
	changedSettings = true;
}

void CameraBoard::setISO(int iso) {
	this->iso = iso;
	changedSettings = true;
}

void CameraBoard::setSharpness(int sharpness) {
	if (sharpness < -100) sharpness = -100;
	if (sharpness > 100) sharpness = 100;
	this->sharpness = sharpness;
	changedSettings = true;
}

void CameraBoard::setContrast(int contrast) {
	if (contrast < -100) contrast = -100;
	if (contrast > 100) contrast = 100;
	this->contrast = contrast;
	changedSettings = true;
}

void CameraBoard::setSaturation(int saturation) {
	if (saturation < -100) saturation = -100;
	if (saturation > 100) saturation = 100;
	this->saturation = saturation;
	changedSettings = true;
}

void CameraBoard::setEncoding(CAMERA_BOARD_ENCODING encoding) {
	this->encoding = encoding;
	changedSettings = true;
}

void CameraBoard::setExposure(CAMERA_BOARD_EXPOSURE exposure) {
	this->exposure = exposure;
	changedSettings = true;
}

void CameraBoard::setAWB(CAMERA_BOARD_AWB awb) {
	this->awb = awb;
	changedSettings = true;
}

void CameraBoard::setImageEffect(CAMERA_BOARD_IMAGE_EFFECT imageEffect) {
	this->imageEffect = imageEffect;
	changedSettings = true;
}

void CameraBoard::setMetering(CAMERA_BOARD_METERING metering) {
	this->metering = metering;
	changedSettings = true;
}

void CameraBoard::setHorizontalFlip(bool hFlip) {
	horizontalFlip = hFlip;
	changedSettings = true;
}

void CameraBoard::setVerticalFlip(bool vFlip) {
	verticalFlip = vFlip;
	changedSettings = true;
}

unsigned int CameraBoard::getWidth() {
	return width;
}

unsigned int CameraBoard::getHeight() {
	return height;
}

unsigned int CameraBoard::getBrightness() {
	return brightness;
}

unsigned int CameraBoard::getRotation() {
	return rotation;
}

unsigned int CameraBoard::getQuality() {
	return quality;
}

int CameraBoard::getISO() {
	return iso;
}

int CameraBoard::getSharpness() {
	return sharpness;
}

int CameraBoard::getContrast() {
	return contrast;
}

int CameraBoard::getSaturation() {
	return saturation;
}

CAMERA_BOARD_ENCODING CameraBoard::getEncoding() {
	return encoding;
}

CAMERA_BOARD_EXPOSURE CameraBoard::getExposure() {
	return exposure;
}

CAMERA_BOARD_AWB CameraBoard::getAWB() {
	return awb;
}

CAMERA_BOARD_IMAGE_EFFECT CameraBoard::getImageEffect() {
	return imageEffect;
}

CAMERA_BOARD_METERING CameraBoard::getMetering() {
	return metering;
}

bool CameraBoard::isHorizontallyFlipped() {
	return horizontalFlip;
}

bool CameraBoard::isVerticallyFlipped() {
	return verticalFlip;
}

void CameraBoard::commitBrightness() {
	mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_BRIGHTNESS, (MMAL_RATIONAL_T) {brightness, 100});
}

void CameraBoard::commitQuality() {
	if (encoder_output_port != NULL)
		mmal_port_parameter_set_uint32(encoder_output_port, MMAL_PARAMETER_JPEG_Q_FACTOR, quality);
}

void CameraBoard::commitRotation() {
	int rotation = int(this->rotation / 90) * 90;
	mmal_port_parameter_set_int32(camera->output[0], MMAL_PARAMETER_ROTATION, rotation);
	mmal_port_parameter_set_int32(camera->output[1], MMAL_PARAMETER_ROTATION, rotation);
	mmal_port_parameter_set_int32(camera->output[2], MMAL_PARAMETER_ROTATION, rotation);
}

void CameraBoard::commitISO() {
	if (mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_ISO, iso) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set ISO parameter.\n";
}

void CameraBoard::commitSharpness() {
	if (mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_SHARPNESS, (MMAL_RATIONAL_T) {sharpness, 100}) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set sharpness parameter.\n";
}

void CameraBoard::commitContrast() {
	if (mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_CONTRAST, (MMAL_RATIONAL_T) {contrast, 100}) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set contrast parameter.\n";
}

void CameraBoard::commitSaturation() {
	if (mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_SATURATION, (MMAL_RATIONAL_T) {saturation, 100}) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set saturation parameter.\n";
}

void CameraBoard::commitExposure() {
	MMAL_PARAMETER_EXPOSUREMODE_T exp_mode = {{MMAL_PARAMETER_EXPOSURE_MODE,sizeof(exp_mode)}, convertExposure(exposure)};
	if (mmal_port_parameter_set(camera->control, &exp_mode.hdr) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set exposure parameter.\n";
}

void CameraBoard::commitAWB() {
	MMAL_PARAMETER_AWBMODE_T param = {{MMAL_PARAMETER_AWB_MODE,sizeof(param)}, convertAWB(awb)};
	if (mmal_port_parameter_set(camera->control, &param.hdr) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set AWB parameter.\n";
}

void CameraBoard::commitImageEffect() {
	MMAL_PARAMETER_IMAGEFX_T imgFX = {{MMAL_PARAMETER_IMAGE_EFFECT,sizeof(imgFX)}, convertImageEffect(imageEffect)};
	if (mmal_port_parameter_set(camera->control, &imgFX.hdr) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set image effect parameter.\n";
}

void CameraBoard::commitMetering() {
	MMAL_PARAMETER_EXPOSUREMETERINGMODE_T meter_mode = {{MMAL_PARAMETER_EXP_METERING_MODE, sizeof(meter_mode)}, convertMetering(metering)};
	if (mmal_port_parameter_set(camera->control, &meter_mode.hdr) != MMAL_SUCCESS)
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set metering parameter.\n";
}

void CameraBoard::commitFlips() {
	MMAL_PARAMETER_MIRROR_T mirror = {{MMAL_PARAMETER_MIRROR, sizeof(MMAL_PARAMETER_MIRROR_T)}, MMAL_PARAM_MIRROR_NONE};
	if (horizontalFlip && verticalFlip)
		mirror.value = MMAL_PARAM_MIRROR_BOTH;
	else if (horizontalFlip)
		mirror.value = MMAL_PARAM_MIRROR_HORIZONTAL;
	else if (verticalFlip)
		mirror.value = MMAL_PARAM_MIRROR_VERTICAL;
	if (mmal_port_parameter_set(camera->output[0], &mirror.hdr) != MMAL_SUCCESS ||
		mmal_port_parameter_set(camera->output[1], &mirror.hdr) != MMAL_SUCCESS ||
		mmal_port_parameter_set(camera->output[2], &mirror.hdr))
		if (errorStream)
			*errorStream << API_NAME << ": Failed to set horizontal/vertical flip parameter.\n";
}

MMAL_FOURCC_T CameraBoard::convertEncoding(CAMERA_BOARD_ENCODING encoding) {
	switch (encoding) {
		case CAMERA_BOARD_ENCODING_JPEG: return MMAL_ENCODING_JPEG;
		case CAMERA_BOARD_ENCODING_BMP: return MMAL_ENCODING_BMP;
		case CAMERA_BOARD_ENCODING_GIF: return MMAL_ENCODING_GIF;
		case CAMERA_BOARD_ENCODING_PNG: return MMAL_ENCODING_PNG;
		case CAMERA_BOARD_ENCODING_RGB: return MMAL_ENCODING_BMP;
		default: return -1;
	}
}

MMAL_PARAM_EXPOSUREMETERINGMODE_T CameraBoard::convertMetering(CAMERA_BOARD_METERING metering) {
	switch (metering) {
		case CAMERA_BOARD_METERING_AVERAGE: return MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
		case CAMERA_BOARD_METERING_SPOT: return  MMAL_PARAM_EXPOSUREMETERINGMODE_SPOT;
		case CAMERA_BOARD_METERING_BACKLIT: return MMAL_PARAM_EXPOSUREMETERINGMODE_BACKLIT;
		case CAMERA_BOARD_METERING_MATRIX: return MMAL_PARAM_EXPOSUREMETERINGMODE_MATRIX;
		default: return MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
	}
}

MMAL_PARAM_EXPOSUREMODE_T CameraBoard::convertExposure(CAMERA_BOARD_EXPOSURE exposure) {
	switch (exposure) {
		case CAMERA_BOARD_EXPOSURE_OFF: return MMAL_PARAM_EXPOSUREMODE_OFF;
		case CAMERA_BOARD_EXPOSURE_AUTO: return MMAL_PARAM_EXPOSUREMODE_AUTO;
		case CAMERA_BOARD_EXPOSURE_NIGHT: return MMAL_PARAM_EXPOSUREMODE_NIGHT;
		case CAMERA_BOARD_EXPOSURE_NIGHTPREVIEW: return MMAL_PARAM_EXPOSUREMODE_NIGHTPREVIEW;
		case CAMERA_BOARD_EXPOSURE_BACKLIGHT: return MMAL_PARAM_EXPOSUREMODE_BACKLIGHT;
		case CAMERA_BOARD_EXPOSURE_SPOTLIGHT: return MMAL_PARAM_EXPOSUREMODE_SPOTLIGHT;
		case CAMERA_BOARD_EXPOSURE_SPORTS: return MMAL_PARAM_EXPOSUREMODE_SPORTS;
		case CAMERA_BOARD_EXPOSURE_SNOW: return MMAL_PARAM_EXPOSUREMODE_SNOW;
		case CAMERA_BOARD_EXPOSURE_BEACH: return MMAL_PARAM_EXPOSUREMODE_BEACH;
		case CAMERA_BOARD_EXPOSURE_VERYLONG: return MMAL_PARAM_EXPOSUREMODE_VERYLONG;
		case CAMERA_BOARD_EXPOSURE_FIXEDFPS: return MMAL_PARAM_EXPOSUREMODE_FIXEDFPS;
		case CAMERA_BOARD_EXPOSURE_ANTISHAKE: return MMAL_PARAM_EXPOSUREMODE_ANTISHAKE;
		case CAMERA_BOARD_EXPOSURE_FIREWORKS: return MMAL_PARAM_EXPOSUREMODE_FIREWORKS;
		default: return MMAL_PARAM_EXPOSUREMODE_AUTO;
	}
}

MMAL_PARAM_AWBMODE_T CameraBoard::convertAWB(CAMERA_BOARD_AWB awb) {
	switch (awb) {
		case CAMERA_BOARD_AWB_OFF: return MMAL_PARAM_AWBMODE_OFF;
		case CAMERA_BOARD_AWB_AUTO: return MMAL_PARAM_AWBMODE_AUTO;
		case CAMERA_BOARD_AWB_SUNLIGHT: return MMAL_PARAM_AWBMODE_SUNLIGHT;
		case CAMERA_BOARD_AWB_CLOUDY: return MMAL_PARAM_AWBMODE_CLOUDY;
		case CAMERA_BOARD_AWB_SHADE: return MMAL_PARAM_AWBMODE_SHADE;
		case CAMERA_BOARD_AWB_TUNGSTEN: return MMAL_PARAM_AWBMODE_TUNGSTEN;
		case CAMERA_BOARD_AWB_FLUORESCENT: return MMAL_PARAM_AWBMODE_FLUORESCENT;
		case CAMERA_BOARD_AWB_INCANDESCENT: return MMAL_PARAM_AWBMODE_INCANDESCENT;
		case CAMERA_BOARD_AWB_FLASH: return MMAL_PARAM_AWBMODE_FLASH;
		case CAMERA_BOARD_AWB_HORIZON: return MMAL_PARAM_AWBMODE_HORIZON;
		default: return MMAL_PARAM_AWBMODE_AUTO;
	}
}

MMAL_PARAM_IMAGEFX_T CameraBoard::convertImageEffect(CAMERA_BOARD_IMAGE_EFFECT imageEffect) {
	switch (imageEffect) {
		case CAMERA_BOARD_IMAGE_EFFECT_NONE: return MMAL_PARAM_IMAGEFX_NONE;
		case CAMERA_BOARD_IMAGE_EFFECT_NEGATIVE: return MMAL_PARAM_IMAGEFX_NEGATIVE;
		case CAMERA_BOARD_IMAGE_EFFECT_SOLARIZE: return MMAL_PARAM_IMAGEFX_SOLARIZE;
		case CAMERA_BOARD_IMAGE_EFFECT_SKETCH: return MMAL_PARAM_IMAGEFX_SKETCH;
		case CAMERA_BOARD_IMAGE_EFFECT_DENOISE: return MMAL_PARAM_IMAGEFX_DENOISE;
		case CAMERA_BOARD_IMAGE_EFFECT_EMBOSS: return MMAL_PARAM_IMAGEFX_EMBOSS;
		case CAMERA_BOARD_IMAGE_EFFECT_OILPAINT: return MMAL_PARAM_IMAGEFX_OILPAINT;
		case CAMERA_BOARD_IMAGE_EFFECT_HATCH: return MMAL_PARAM_IMAGEFX_HATCH;
		case CAMERA_BOARD_IMAGE_EFFECT_GPEN: return MMAL_PARAM_IMAGEFX_GPEN;
		case CAMERA_BOARD_IMAGE_EFFECT_PASTEL: return MMAL_PARAM_IMAGEFX_PASTEL;
		case CAMERA_BOARD_IMAGE_EFFECT_WATERCOLOR: return MMAL_PARAM_IMAGEFX_WATERCOLOUR;
		case CAMERA_BOARD_IMAGE_EFFECT_FILM: return MMAL_PARAM_IMAGEFX_FILM;
		case CAMERA_BOARD_IMAGE_EFFECT_BLUR: return MMAL_PARAM_IMAGEFX_BLUR;
		case CAMERA_BOARD_IMAGE_EFFECT_SATURATION: return MMAL_PARAM_IMAGEFX_SATURATION;
		case CAMERA_BOARD_IMAGE_EFFECT_COLORSWAP: return MMAL_PARAM_IMAGEFX_COLOURSWAP;
		case CAMERA_BOARD_IMAGE_EFFECT_WASHEDOUT: return MMAL_PARAM_IMAGEFX_WASHEDOUT;
		case CAMERA_BOARD_IMAGE_EFFECT_POSTERISE: return MMAL_PARAM_IMAGEFX_POSTERISE;
		case CAMERA_BOARD_IMAGE_EFFECT_COLORPOINT: return MMAL_PARAM_IMAGEFX_COLOURPOINT;
		case CAMERA_BOARD_IMAGE_EFFECT_COLORBALANCE: return MMAL_PARAM_IMAGEFX_COLOURBALANCE;
		case CAMERA_BOARD_IMAGE_EFFECT_CARTOON: return MMAL_PARAM_IMAGEFX_CARTOON;
	}
}


