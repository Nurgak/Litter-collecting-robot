#ifndef RaspiCam_CV_H
#define RaspiCam_CV_H
#include <opencv2/core/core.hpp>

class CameraBoard;
/**Class for using Raspberry camera as in opencv
 */
class RaspiCam_Cv {
    CameraBoard *_cam_impl;
    public:
    /**Constructor
     */
    RaspiCam_Cv();
    /**Destructor
     */
    ~RaspiCam_Cv();
    /** Open  capturing device for video capturing
     */
    bool open ( void );
    /**
     * Returns true if video capturing has been initialized already.
     */
    bool isOpened() const;
    /**
    *Closes video file or capturing device.
    */
    void release();

    /**
     * Grabs the next frame from video file or capturing device.
     */
    bool grab();

    /**
    *Decodes and returns the grabbed video frame.
     */
    bool retrieve ( cv::Mat& image );

    /**Returns the specified VideoCapture property
     */

    double get ( int propId );

    /**Sets a property in the VideoCapture.
     */

    bool set ( int propId, double value );

    private:
    bool _isInitialized;
    //buffer for image data
    struct buff {

        buff() {
            size=0;
        }
        void allocate ( int bytes ) {
            size=bytes;
            ptr=new uchar[bytes];
        }
        cv::Ptr<uchar>  ptr;
        int size;

    };
    buff _imgData;
    int _width,_height;

 
 

};
#endif


