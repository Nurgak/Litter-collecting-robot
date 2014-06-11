#include "VideoBoard.h"
#include "raspivid.h"
#include "VideoBoardTypes.h"

const unsigned int RaspiVid::METHOD_MMAP = VideoBoard::METHOD_MMAP;
const unsigned int RaspiVid::METHOD_READ = VideoBoard::METHOD_READ;

RaspiVid::RaspiVid(const char * device, int width, int height) {
	_impl = new VideoBoard(device, width, height);
}
RaspiVid::~RaspiVid() {
	_impl->destroy();
	delete _impl;
}

int RaspiVid::getWidth() {
	return _impl->getWidth();
}
int RaspiVid::getHeight() {
	return _impl->getHeight();
}
bool RaspiVid::initialize(unsigned int method) {
	return _impl->initialize(method);
}
void RaspiVid::destroy() {
	_impl->destroy();
}
void RaspiVid::startCapturing() {
	_impl->startCapturing();
}
VideoBuffer RaspiVid::grabFrame() {
	return _impl->grabFrame();
}
void RaspiVid::releaseFrame(VideoBuffer * buffer) {
	_impl->releaseFrame(buffer);
}
void RaspiVid::setBrightness(int brightness) {
	_impl->setBrightness(brightness);
}
