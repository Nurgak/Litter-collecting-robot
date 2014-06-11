#ifndef CameraBoardTypes_H
#define CameraBoardTypes_H
#include "mmal/mmal.h"
#include "mmal/mmal_connection.h"
#include <iostream>
#include <semaphore.h>
typedef void (*ImageTakenCallback)(unsigned char * data, unsigned int image_offset, unsigned int length);

class CameraBoard;
typedef struct {
	CameraBoard * cameraBoard;
	MMAL_POOL_T * encoderPool;
	std::ostream * errorStream;
	ImageTakenCallback imageCallback;
	sem_t * mutex;
	unsigned char * data;
	unsigned int bufferPosition;
	unsigned int startingOffset;
	unsigned int offset;
	unsigned int length;
	unsigned int unexpectedControlCallbacks;
} CAMERA_BOARD_USERDATA;

typedef enum CAMERA_BOARD_EXPOSURE {
	CAMERA_BOARD_EXPOSURE_OFF,
	CAMERA_BOARD_EXPOSURE_AUTO,
	CAMERA_BOARD_EXPOSURE_NIGHT,
	CAMERA_BOARD_EXPOSURE_NIGHTPREVIEW,
	CAMERA_BOARD_EXPOSURE_BACKLIGHT,
	CAMERA_BOARD_EXPOSURE_SPOTLIGHT,
	CAMERA_BOARD_EXPOSURE_SPORTS,
	CAMERA_BOARD_EXPOSURE_SNOW,
	CAMERA_BOARD_EXPOSURE_BEACH,
	CAMERA_BOARD_EXPOSURE_VERYLONG,
	CAMERA_BOARD_EXPOSURE_FIXEDFPS,
	CAMERA_BOARD_EXPOSURE_ANTISHAKE,
	CAMERA_BOARD_EXPOSURE_FIREWORKS
} CAMERA_BOARD_EXPOSURE;

typedef enum CAMERA_BOARD_AWB {
	CAMERA_BOARD_AWB_OFF,
	CAMERA_BOARD_AWB_AUTO,
	CAMERA_BOARD_AWB_SUNLIGHT,
	CAMERA_BOARD_AWB_CLOUDY,
	CAMERA_BOARD_AWB_SHADE,
	CAMERA_BOARD_AWB_TUNGSTEN,
	CAMERA_BOARD_AWB_FLUORESCENT,
	CAMERA_BOARD_AWB_INCANDESCENT,
	CAMERA_BOARD_AWB_FLASH,
	CAMERA_BOARD_AWB_HORIZON
} CAMERA_BOARD_AWB;

typedef enum CAMERA_BOARD_IMAGE_EFFECT {
	CAMERA_BOARD_IMAGE_EFFECT_NONE,
	CAMERA_BOARD_IMAGE_EFFECT_NEGATIVE,
	CAMERA_BOARD_IMAGE_EFFECT_SOLARIZE,
	CAMERA_BOARD_IMAGE_EFFECT_SKETCH,
	CAMERA_BOARD_IMAGE_EFFECT_DENOISE,
	CAMERA_BOARD_IMAGE_EFFECT_EMBOSS,
	CAMERA_BOARD_IMAGE_EFFECT_OILPAINT,
	CAMERA_BOARD_IMAGE_EFFECT_HATCH,
	CAMERA_BOARD_IMAGE_EFFECT_GPEN,
	CAMERA_BOARD_IMAGE_EFFECT_PASTEL,
	CAMERA_BOARD_IMAGE_EFFECT_WATERCOLOR,
	CAMERA_BOARD_IMAGE_EFFECT_FILM,
	CAMERA_BOARD_IMAGE_EFFECT_BLUR,
	CAMERA_BOARD_IMAGE_EFFECT_SATURATION,
	CAMERA_BOARD_IMAGE_EFFECT_COLORSWAP,
	CAMERA_BOARD_IMAGE_EFFECT_WASHEDOUT,
	CAMERA_BOARD_IMAGE_EFFECT_POSTERISE,
	CAMERA_BOARD_IMAGE_EFFECT_COLORPOINT,
	CAMERA_BOARD_IMAGE_EFFECT_COLORBALANCE,
	CAMERA_BOARD_IMAGE_EFFECT_CARTOON
} CAMERA_BOARD_IMAGE_EFFECT;

typedef enum CAMERA_BOARD_METERING {
	CAMERA_BOARD_METERING_AVERAGE,
	CAMERA_BOARD_METERING_SPOT,
	CAMERA_BOARD_METERING_BACKLIT,
	CAMERA_BOARD_METERING_MATRIX
} CAMERA_BOARD_METERING;

typedef enum CAMERA_BOARD_ENCODING {
	CAMERA_BOARD_ENCODING_JPEG,
	CAMERA_BOARD_ENCODING_BMP,
	CAMERA_BOARD_ENCODING_GIF,
	CAMERA_BOARD_ENCODING_PNG,
	CAMERA_BOARD_ENCODING_RGB
} CAMERA_BOARD_ENCODING;

#endif

