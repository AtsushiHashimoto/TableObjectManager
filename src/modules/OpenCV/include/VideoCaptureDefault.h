/*!
 * @file VideoCaptureDefault.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/18
 * @date Last Change:2012/Jan/18.
 */
#ifndef __SKL_VIDEO_CAPTURE_DEFAULT_H__
#define __SKL_VIDEO_CAPTURE_DEFAULT_H__

// OpenCV
#include <cv.h>

// SKL OpenCV Module
#include "VideoCaptureInterface.h"


namespace skl{

	/*!
	 * @brief OpenCVのcv::VideoCaptureを利用するVideoCapture
	 */
	class VideoCaptureDefault: public VideoCaptureInterface<VideoCaptureDefault>, public cv::VideoCapture{

		public:
			using _VideoCaptureInterface::get;
			using _VideoCaptureInterface::set;
			VideoCaptureDefault();
			virtual ~VideoCaptureDefault();
			inline bool open(const std::string& filename){return cv::VideoCapture::open(filename);}
			inline bool open(int device){return cv::VideoCapture::open(device);}
			inline bool isOpened()const{return cv::VideoCapture::isOpened();}
			inline void release(){cv::VideoCapture::release();}
			inline bool grab(){return cv::VideoCapture::grab();}
			inline bool retrieve(cv::Mat& image,int channel=0){
				if(!cv::VideoCapture::retrieve(buf,channel)) return false;
				image = buf.clone();
				return true;
			}

			//! カメラに値やモード(camera_mode_tがset(*,camera_mode_t mode)を通してvalに与えられる(modeは-4から-1までの整数)をセットする純粋仮想関数
			bool set(capture_property_t prop_id,double val);
			//! カメラから値をgetする純粋仮想関数(modeはインターフェイス簡素化のためサポートしない)
			double get(capture_property_t prop_id);

		protected:
			cv::Mat buf;
		private:
	};

} // skl

#endif // __SKL_VIDEO_CAPTURE_DEFAULT_H__

