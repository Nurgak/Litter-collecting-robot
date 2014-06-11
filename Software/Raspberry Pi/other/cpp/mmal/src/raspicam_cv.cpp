#include "raspicam_cv.h"
#include "CameraBoard.h"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
RaspiCam_Cv::RaspiCam_Cv() {
    _cam_impl=0;
}
RaspiCam_Cv::~RaspiCam_Cv() {
    release();
}

/** Open  capturing device for video capturing
 */
bool RaspiCam_Cv::open ( void ) {
    if ( _cam_impl ) return false; //already opened
    _cam_impl=new CameraBoard();
    _width=640;
    _height=480;
    _cam_impl->setWidth ( _width );
    _cam_impl->setHeight ( _height );
    _cam_impl->setEncoding ( CAMERA_BOARD_ENCODING_RGB );
    _imgData.allocate ( _width*_height*3 );
    _isInitialized= ( _cam_impl->initialize() ==0 );
    return _isInitialized;
}
/**
 * Returns true if video capturing has been initialized already.
 */
bool RaspiCam_Cv::isOpened() const {return _isInitialized;}
/**
*Closes video file or capturing device.
*/
void RaspiCam_Cv::release() {
    if ( _cam_impl ) delete _cam_impl;
    _cam_impl=0;
    _isInitialized=false;
}

/**
 * Grabs the next frame from video file or capturing device.
 */
bool RaspiCam_Cv::grab() {
  
    if (! _isInitialized ) return false;
 
    bool res=(_cam_impl->takePicture ( _imgData.ptr,_imgData.size ) ==0);
    return res;
}

/**
*Decodes and returns the grabbed video frame.
 */
bool RaspiCam_Cv::retrieve ( cv::Mat& image ) {
    image.create ( _height,_width,CV_8UC3 );
    if ( image.isContinuous() )
        memcpy ( image.ptr<uchar> ( 0 ),_imgData.ptr,_imgData.size );
    else {
        for ( int y=0; y<_height; y++ )
            memcpy ( image.ptr<uchar> ( y ),_imgData.ptr+_width*y*3,_width*3 );

    }
}

/**Returns the specified VideoCapture property
 */

double RaspiCam_Cv::get ( int propId ) {

    switch ( propId ) {

    case CV_CAP_PROP_FRAME_WIDTH :
        return _cam_impl->getWidth();
    case CV_CAP_PROP_FRAME_HEIGHT :
        return _cam_impl->getHeight();
    case CV_CAP_PROP_FPS:
        return -1;//NOT YET CLEAR OW TO OBTAIN IT!!!
    case CV_CAP_PROP_FORMAT :
        return CV_8UC3;
    case CV_CAP_PROP_MODE :
        return 0;
    case CV_CAP_PROP_BRIGHTNESS :
        return _cam_impl->getBrightness();
    case CV_CAP_PROP_CONTRAST :
        return _cam_impl->getContrast();
    case CV_CAP_PROP_SATURATION :
        return _cam_impl->getSaturation();
//     case CV_CAP_PROP_HUE : return _cam_impl->getSharpness();
    case CV_CAP_PROP_GAIN :
        return  _cam_impl->getISO();
    case CV_CAP_PROP_EXPOSURE :
        return _cam_impl->getExposure();
    case CV_CAP_PROP_CONVERT_RGB :
        return true;
//     case CV_CAP_PROP_WHITE_BALANCE :return _cam_impl->getAWB();
    default :
        return -1;
    };
}

/**Sets a property in the VideoCapture.
 */

bool RaspiCam_Cv::set ( int propId, double value ) {
    bool changeSize=false;

    switch ( propId ) {

    case CV_CAP_PROP_FRAME_WIDTH :
        _width=value;
        _cam_impl->setWidth ( value );
        changeSize=true;
        break;
    case CV_CAP_PROP_FRAME_HEIGHT :
        _height=value;
        _cam_impl->setHeight ( value );
        changeSize=true;
        break;
        //case CV_CAP_PROP_FPS: return _cam_impl->get;
    case CV_CAP_PROP_FORMAT ://images are returned in color
        return false;
        break;
    case CV_CAP_PROP_MODE ://nothing to  do yet
        return false;
        break;
    case CV_CAP_PROP_BRIGHTNESS :
        _cam_impl->setBrightness ( value );
        return true;
        break;
    case CV_CAP_PROP_CONTRAST :
        _cam_impl->setContrast ( value );
        return true;
        break;
    case CV_CAP_PROP_SATURATION :
        _cam_impl->setSaturation ( value );
        return true;
        break;
//     case CV_CAP_PROP_HUE : return _cam_impl->getSharpness();
    case CV_CAP_PROP_GAIN :
        _cam_impl->setISO ( value );
        return true;
        break;
    case CV_CAP_PROP_EXPOSURE :
//          _cam_impl->setExposure();
        return false;
        break;
    case CV_CAP_PROP_CONVERT_RGB :
        return true;
        break;
//     case CV_CAP_PROP_WHITE_BALANCE :return _cam_impl->getAWB();
    default :
        return -1;
    };
    if ( changeSize ) {
        _imgData.allocate ( _width*_height*3 );
    }
    return true;

}


