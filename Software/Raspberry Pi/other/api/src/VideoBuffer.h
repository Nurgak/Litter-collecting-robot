#ifndef VIDEOBUFFER_H
#define VIDEOBUFFER_H

class VideoBoard;
class VideoBuffer {
	private:
	VideoBoard * _board;
	unsigned char * _data;
	unsigned int _length;
	bool _initialized;
	
	public:
	/** Creates a VideoBuffer. */
	VideoBuffer(VideoBoard * board, unsigned char * data, unsigned int length);
	
	/** Deallocates and releases the VideoBuffer */
	~VideoBuffer();
	
	/** Manually releases the VideoBuffer, it is called automatically by the destructor */
	void release();
	
	/** @return the data in the frame */
	unsigned char * data();
	
	/** @return the size of the data in the frame */
	unsigned int length();
	
};

#endif
