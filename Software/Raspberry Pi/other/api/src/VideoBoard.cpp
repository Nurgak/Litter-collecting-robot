#include "VideoBoard.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))
const unsigned int VideoBoard::METHOD_MMAP = (1 << 0);
const unsigned int VideoBoard::METHOD_READ = (1 << 1);

int VideoBoard::xioctl(int fd, int request, void * arg) {
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

int VideoBoard::readFrame(Buffer * buffer) {
	while (read(fd, buffer->start, buffer->length) == -1) {
		switch (errno) {
			case EAGAIN:
				continue;
			case EINTR:
				continue;
			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				cout << "Reading Error!\n";
				return errno;
		}
	}
	return 0;
}

int VideoBoard::readFrameMmap(Buffer * buffer) {
	CLEAR(buf);
	
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	
	while (xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
		switch (errno) {
			case EAGAIN:
				continue;
			case EINTR:
				continue;
			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				cout << "Error: VIDIOC_DQBUF in readFrameMmap(Buffer*)\n";
				return errno;
		}
	}
	
	if (buf.index >= numBuffers)
		return ERANGE;
	
	buffer->start = buffers[buf.index].start;
	buffer->length = buf.bytesused;
	
	return 0;
}

bool VideoBoard::openDevice() {
	struct stat st; 
	
	if (stat(device, &st) == -1) {
		cout << "Cannot identify " << device << " with error: " << strerror(errno) << "\n";
		return false;
	}
	
	if (!S_ISCHR(st.st_mode)) {
		cout << "Error: " << device << " is not a device!\n";
		return false;
	}
	
	fd = open (device, O_RDWR | O_NONBLOCK, 0);
	
	if (fd == -1) {
		cout << "Error: Cannot open " << device << " with error: " << strerror(errno) << "\n";
		return false;
	}
	return true;
}

bool VideoBoard::initializeDevice() {
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	
	CLEAR (cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect; /* reset to default */
	
	CLEAR (fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width; 
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	
	if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
		if (EINVAL == errno) {
			cout << "Error: " << device << " is not a V4L2 device!\n";
		} else {
			cout << "Error: VIDIOC_QUERYCAP\n";
		}
	} else if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		cout << "Error: " << device << " does not support video capture\n";
	} else if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
		cout << "Error: " << device << " does not support read i/o\n";
	} else if (xioctl(fd, VIDIOC_CROPCAP, &cropcap) && false) { // Errors ignored
		
	} else if (xioctl (fd, VIDIOC_S_CROP, &crop) == -1 && errno != EINVAL) {
		cout << "Error: Cropping not supported!\n";
	} else if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
		cout << "Error: VIDIOC_S_FMT\n";
	} else {
		bufferSize = fmt.fmt.pix.sizeimage;
		return true;
	}
	return false;
}

bool VideoBoard::createBuffers(unsigned int count) {
	for (unsigned int i = 0; i < count; i++) {
		struct v4l2_buffer buf;
		CLEAR(buf);
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;
		
		if (xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
			cout << "Error: VIDIOC_QUERYBUF\n";
			return false;
		}
		
		buffers[i].length = buf.length;
		buffers[i].start = (unsigned char *) mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		
		if (buffers[i].start == MAP_FAILED) {
			cout << "Error: mmap\n";
			return false;
		}
	}
	return true;
}

bool VideoBoard::initMmap() {
	struct v4l2_requestbuffers req;
	
	CLEAR(req);
	
	req.count = numBuffers;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	
	if (xioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
		if (errno == EINVAL) {
			cout << "Error: " << device << " does not support memory mapping\n";
		} else {
			cout << "Error: VIDIOC_REQBUFS\n";
		}
	} else if (req.count < 2) {
		cout << "Error: Not enough memory on " << device << " !\n";
	} else if (!(buffers = new Buffer[req.count])) {
		cout << "Error: Out of memory!\n";
	} else if (!createBuffers(req.count)) {
		// Error messages handled in createBuffers()
	} else {
		return true;
	}
	return false;
}

VideoBoard::VideoBoard(const char * device, int width, int height) {
	this->device      = device;
	this->width       = width;
	this->height      = height;
	this->initialized = false;
	this->numBuffers  = 3;
	this->fd          = -1;
}

VideoBoard::~VideoBoard() {
	destroy();
}

int VideoBoard::getWidth() {
	return width;
}

int VideoBoard::getHeight() {
	return height;
}

bool VideoBoard::initialize(unsigned int method) {
	if (!initialized) {
		if (!openDevice()) {
			cout << "Unable to open device.\n";
			return false;
		}
		if (!initializeDevice()) {
			cout << "Unable to initialize device.\n";
			return false;
		}
		if (method == METHOD_MMAP && !initMmap()) {
			cout << "Unable to initialize mmap.\n";
			return false;
		} else if (method == METHOD_READ) {
			preAllocatedBuffer = new unsigned char[bufferSize];
		}
		this->method = method;
		initialized = true;
		return true;
	}
}

void VideoBoard::destroy() {
	if (initialized) {
		if (method == METHOD_READ)
			delete preAllocatedBuffer;
		if (close(fd) == -1) {
			cout << "Error: Failed to close device\n";
			return;
		}
		fd = -1;
		initialized = false;
	}
}

void VideoBoard::startCapturing() {
	if (!initialized) return;
	if (method == METHOD_MMAP) {
		enum v4l2_buf_type type;
		for (unsigned int i = 0; i < numBuffers; i++) {
			struct v4l2_buffer buf;
		
			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
		
			if (xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
				cout << "Error: VIDIOC_QBUF\n";
				break;
			}
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (xioctl(fd, VIDIOC_STREAMON, &type) == -1) {
			cout << "Error: VIDIOC_STREAMON\n";
		}
	}
}

VideoBuffer VideoBoard::grabFrame() {
	if (!initialized) { cout << "Uninitialized! You must initialize before calling grabFrame. \n"; return VideoBuffer(this, NULL, 0); }
	Buffer b;
	b.start = NULL;
	b.length = bufferSize;
	int error = 0;
	if (method == METHOD_READ) {
		b.start = preAllocatedBuffer;
		error = readFrame(&b);
	} else if (method == METHOD_MMAP) {
		error = readFrameMmap(&b);
	}
	if (error != 0) {
		cout << "Error: Grab frame failed with errorno #" << error << "\n";
	}
	return VideoBuffer(this, b.start, b.length);
}

void VideoBoard::setBrightness(int brightness) {
	struct v4l2_queryctrl queryctrl;
	struct v4l2_control control;
	
	memset(&queryctrl, 0, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_BRIGHTNESS;
	
	if (xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) == -1) {
		cout << "V4L2_CID_BRIGHTNESS is unable to be queried.\n";
		return;
	}
	if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
		cout << "V4L2_CID_BRIGHTNESS is not supported.\n";
		return;
	}
	if (brightness < queryctrl.minimum)
		brightness = queryctrl.minimum;
	else if (brightness > queryctrl.maximum)
		brightness = queryctrl.maximum;
	
	memset(&control, 0, sizeof(control));
	control.id = V4L2_CID_BRIGHTNESS;
	control.value = brightness;
	
	if (xioctl(fd, VIDIOC_S_CTRL, &control) == -1) {
		cout << "Failed to set V4L2_CID_BRIGHTNESS value.\n";
		return;
	}
}

void VideoBoard::releaseFrame(VideoBuffer * buffer) {
	if (!initialized) return;
	if (method == METHOD_READ) {
		preAllocatedBuffer = buffer->data();
	} else if (method == METHOD_MMAP) {
		if (xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
			cout << "Error: VIDIOC_QBUF in releaseFrame()\n";
		}
	}
}

