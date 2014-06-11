#include "CameraBoard.h"
#include "raspicam.h"

RaspiCam::RaspiCam():_impl(new CameraBoard()) {
	
}
RaspiCam::~RaspiCam(){
	delete _impl;
}

int RaspiCam::initialize() {
    return _impl->initialize();
}
int RaspiCam::startCapture(ImageTakenCallback userCallback, unsigned char * preallocated_data, unsigned int offset, unsigned int length) {
    return _impl->startCapture(userCallback, preallocated_data,offset,length);
}
void RaspiCam::stopCapture() {
    _impl->stopCapture();
}
void RaspiCam::close() {
	_impl->close();
}
int RaspiCam::takePicture(unsigned char * preallocated_data, unsigned int length) {
	return _impl->takePicture(preallocated_data, length);
}
/*void RaspiCam::bufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    _impl->bufferCallback(port,buffer);
}*/
void RaspiCam::commitParameters() {
    _impl-> commitParameters();
}
void RaspiCam::setWidth(unsigned int width) {
    _impl-> setWidth(width);
}
void RaspiCam::setHeight(unsigned int height) {
    _impl->setHeight(height);
}
void RaspiCam::setCaptureSize(unsigned int width, unsigned int height) {
    _impl->setCaptureSize(width,height);
}
void RaspiCam::setBrightness(unsigned int brightness) {
    _impl->setBrightness(brightness);
}
void RaspiCam::setQuality(unsigned int quality) {
	_impl->setQuality(quality);
}
void RaspiCam::setRotation(int rotation) {
    _impl-> setRotation(rotation);
}
void RaspiCam::setISO(int iso) {
    _impl-> setISO(iso);
}
void RaspiCam::setSharpness(int sharpness) {
    _impl->setSharpness(sharpness);
}
void RaspiCam::setContrast(int contrast) {
    _impl->setContrast(contrast);
}
void RaspiCam::setSaturation(int saturation) {
    _impl->setSaturation(saturation);
}
void RaspiCam::setEncoding(CAMERA_BOARD_ENCODING encoding) {
    _impl->setEncoding(encoding);
}
void RaspiCam::setExposure(CAMERA_BOARD_EXPOSURE exposure) {
    _impl->setExposure(exposure);
}
void RaspiCam::setAWB(CAMERA_BOARD_AWB awb) {
    _impl->setAWB(awb);
}
void RaspiCam::setImageEffect(CAMERA_BOARD_IMAGE_EFFECT imageEffect) {
    _impl-> setImageEffect(imageEffect);
}
void RaspiCam::setMetering(CAMERA_BOARD_METERING metering) {
    _impl->setMetering(metering);
}
void RaspiCam::setHorizontalFlip(bool hFlip) {
    _impl->setHorizontalFlip(hFlip);
}
void RaspiCam::setVerticalFlip(bool vFlip) {
    _impl->setVerticalFlip(vFlip);
}

unsigned int RaspiCam::getWidth() {
    return _impl->getWidth();
}
unsigned int RaspiCam::getHeight() {
    return _impl->getHeight();
}
unsigned int RaspiCam::getBrightness() {
    return _impl->getBrightness();
}
unsigned int RaspiCam::getRotation() {
    return _impl->getRotation();
}
unsigned int RaspiCam::getQuality() {
	return _impl->getQuality();
}
int RaspiCam::getISO() {
    return _impl->getISO();
}
int RaspiCam::getSharpness() {
    return _impl->getSharpness();
}
int RaspiCam::getContrast() {
    return _impl->getContrast();
}
int RaspiCam::getSaturation() {
    return _impl->getSaturation();
}
CAMERA_BOARD_ENCODING RaspiCam::getEncoding() {
	return _impl->getEncoding();
}
CAMERA_BOARD_EXPOSURE RaspiCam::getExposure() {
    return _impl->getExposure ();
}
CAMERA_BOARD_AWB RaspiCam::getAWB() {
    return _impl->getAWB();
}
CAMERA_BOARD_IMAGE_EFFECT RaspiCam::getImageEffect() {
    return _impl->getImageEffect();
}
CAMERA_BOARD_METERING RaspiCam::getMetering() {
    return _impl->getMetering();
}
bool RaspiCam::isHorizontallyFlipped() {
    return _impl->isHorizontallyFlipped();
}
bool RaspiCam::isVerticallyFlipped() {
    return _impl->isVerticallyFlipped();
}

