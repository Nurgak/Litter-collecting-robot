#ifndef VideoBoardTypes_H
#define VideoBoardTypes_H

#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "VideoBuffer.h"
using namespace std;

struct Buffer {
	unsigned char * start;
	unsigned int length;
};
#endif // VideoBoardTypes_H
