/*!
 * @file VideoCapture.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/12
 * @date Last Change:2013/Dec/01.
 */
#ifndef __SKL_VIDEO_CAPTURE_H__
#define __SKL_VIDEO_CAPTURE_H__

#include "skl.h"
#include "cvtypes.h"
#include <cv.h>
#include <vector>
#include "VideoCaptureInterface.h"

namespace skl{

	/*!
	 * @brief パラメタの読み込み機能などを強化したVideoCapture
	 */
	class VideoCapture: public VideoCaptureInterface<VideoCapture>{
		public:
			using _VideoCaptureInterface::set;
			using _VideoCaptureInterface::get;
			using VideoCaptureInterface<VideoCapture>::operator>>;
			VideoCapture();
			virtual ~VideoCapture();

			inline bool open(const std::string& filename){
				release();
				return push_back(filename);
			}
			inline bool open(const std::string& filename, cv::Ptr<_VideoCaptureInterface> cam){
				release();
				return push_back(filename,cam);
			}
			inline bool open(int device){
				release();
				return push_back(device);
			}
			inline bool open(int device, cv::Ptr<_VideoCaptureInterface> cam){
				release();
				return push_back(device,cam);
			}

			template<class Iter> bool open(Iter first,Iter last);
			template<class Iter,class camIter> bool open(Iter first,Iter last);

			bool isOpened()const;
			void release();
			bool grab();
			virtual bool retrieve(cv::Mat& image, int channel=0);

			bool set(capture_property_t prop_id,double val);
			double get(capture_property_t prop_id);

			_VideoCaptureInterface& operator[](int device);
			inline size_t size()const{return cam_interface.size();}

			virtual bool push_back(const std::string& filename);
			virtual bool push_back(int device);
			virtual bool push_back(const std::string& filename, cv::Ptr<_VideoCaptureInterface> cam);
			virtual bool push_back(int device, cv::Ptr<_VideoCaptureInterface> cam);

			VideoCapture& operator>>(std::vector<cv::Mat>& mat_vec);

			inline cv::Ptr<_VideoCaptureInterface>& operator[](size_t i){return cam_interface[i];}
			inline const cv::Ptr<_VideoCaptureInterface>& operator[](size_t i)const{return cam_interface[i];}

		protected:
			std::vector<cv::Ptr<_VideoCaptureInterface> > cam_interface;
		private:
	};


	template <class Iter> bool VideoCapture::open(Iter first, Iter last){
		size_t id = 0;
		release();
		for(Iter iter = first; iter != last; iter++,id++){
			if(!push_back(*iter)) return false;
		}
		return true;
	}

} // skl

#endif // __SKL_VIDEO_CAPTURE_H__

