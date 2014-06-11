#include "VideoBoard.h"
#include "VideoBuffer.h"

VideoBuffer::VideoBuffer(VideoBoard * board, unsigned char * data, unsigned int length) {
	this->_board       = board;
	this->_data        = data;
	this->_length      = length;
	this->_initialized = true;
}

VideoBuffer::~VideoBuffer() {
	release();
}

void VideoBuffer::release() {
	if (_initialized) {
		_board->releaseFrame(this);
		_initialized = false;
	}
}

unsigned char * VideoBuffer::data() {
	return _data;
}

unsigned int VideoBuffer::length() {
	return _length;
}

