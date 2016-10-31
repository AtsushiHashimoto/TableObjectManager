#ifndef __SKL_CV_TYPES_H__
#define __SKL_CV_TYPES_H__
#include <highgui.h>
namespace skl{
	typedef enum{
		// modes of the controlling registers (can be: auto, manual, auto single push, absolute Latter allowed with any other mode)
		// every feature can have only one mode turned on at a time
		DC1394_OFF         = -4,  //turn the feature off (not controlled manually nor automatically)
		DC1394_MODE_MANUAL = -3, //set automatically when a value of the feature is set by the user
		DC1394_MODE_AUTO = -2,
		DC1394_MODE_ONE_PUSH_AUTO = -1
	} camera_mode_t;

	typedef enum{
		POS_MSEC       =0,
		POS_FRAMES     =1,
		POS_AVI_RATIO  =2,
		FRAME_WIDTH    =3,
		FRAME_HEIGHT   =4,
		FPS            =5,
		FOURCC         =6,
		FRAME_COUNT    =7,
		FORMAT         =8,
		MODE           =9,
		BRIGHTNESS    =10,
		CONTRAST      =11,
		SATURATION    =12,
		HUE           =13,
		GAIN          =14,
		EXPOSURE      =15,
		CONVERT_RGB   =16,
		WHITE_BALANCE_BLUE_U =17,
		RECTIFICATION =18,
		MONOCROME     =19,
		SHARPNESS     =20,
		AUTO_EXPOSURE =21, // exposure control done by camera,
		// user can adjust refernce level
		// using this feature
		GAMMA         =22,
		TEMPERATURE   =23,
		TRIGGER       =24,
		TRIGGER_DELAY =25,
		WHITE_BALANCE_RED_V =26,
		MAX_DC1394    =27,

		// properties add by skl, which target FlyCapture2 Library
		IRIS = 50,
		FOCUS = 51,
		ZOOM = 52,
		PAN = 53,
		TILT = 54,
		SHUTTER = 55,


		AUTOGRAB      =1024, // property for highgui class CvCapture_Android only
		SUPPORTED_PREVIEW_SIZES_STRING=1025, // readonly, tricky property, returns cpnst char* indeed
		PREVIEW_FORMAT=1026, // readonly, tricky property, returns cpnst char* indeed

		// Properties of cameras available through OpenNI interfaces
		OPENNI_OUTPUT_MODE      = 100,
		OPENNI_FRAME_MAX_DEPTH  = 101, // in mm
		OPENNI_BASELINE         = 102, // in mm
		OPENNI_FOCAL_LENGTH     = 103, // in pixels
		OPENNI_REGISTRATION_ON  = 104, // flag
		OPENNI_REGISTRATION     = CV_CAP_PROP_OPENNI_REGISTRATION_ON, // flag that synchronizes the remapping depth map to image map
		// by changing depth generator's view point (if the flag is "on") or
		// sets this view point to its normal one (if the flag is "off").

		// Properties of cameras available through GStreamer interface
		PVAPI_MULTICASTIP   = 300, // ip for anable multicast master mode. 0 for disable multicast

		// Properties of cameras available through XIMEA SDK interface
		XI_DOWNSAMPLING  = 400,      // Change image resolution by binning or skipping.  
		XI_DATA_FORMAT   = 401,       // Output data format.
		XI_OFFSET_X      = 402,      // Horizontal offset from the origin to the area of interest (in pixels).
		XI_OFFSET_Y      = 403,      // Vertical offset from the origin to the area of interest (in pixels).
		XI_TRG_SOURCE    = 404,      // Defines source of trigger.
		XI_TRG_SOFTWARE  = 405,      // Generates an internal trigger. PRM_TRG_SOURCE must be set to TRG_SOFTWARE.
		XI_GPI_SELECTOR  = 406,      // Selects general purpose input 
		XI_GPI_MODE      = 407,      // Set general purpose input mode
		XI_GPI_LEVEL     = 408,      // Get general purpose level
		XI_GPO_SELECTOR  = 409,      // Selects general purpose output 
		XI_GPO_MODE      = 410,      // Set general purpose output mode
		XI_LED_SELECTOR  = 411,      // Selects camera signalling LED 
		XI_LED_MODE      = 412,      // Define camera signalling LED functionality
		XI_MANUAL_WB     = 413,      // Calculates White Balance(must be called during acquisition)
		XI_AUTO_WB       = 414,      // Automatic white balance
		XI_AEAG          = 415,      // Automatic exposure/gain
		XI_EXP_PRIORITY  = 416,      // Exposure priority (0.5 - exposure 50%, gain 50%).
		XI_AE_MAX_LIMIT  = 417,      // Maximum limit of exposure in AEAG procedure
		XI_AG_MAX_LIMIT  = 418,      // Maximum limit of gain in AEAG procedure
		XI_AEAG_LEVEL    = 419,       // Average intensity of output signal AEAG should achieve(in %)
		XI_TIMEOUT       = 420,       // Image capture timeout in milliseconds

		// Properties of cameras available through AVFOUNDATION interface
		IOS_DEVICE_FOCUS = 9001,
		IOS_DEVICE_EXPOSURE = 9002,
		IOS_DEVICE_FLASH = 9003,
		IOS_DEVICE_WHITEBALANCE = 9004,
		IOS_DEVICE_TORCH = 9005,

		// skl original parameters
		SKIP_FAKE_CAPTURE = 10001
	} capture_property_t;
}
#endif // __SKL_CV_TYPES_H__
