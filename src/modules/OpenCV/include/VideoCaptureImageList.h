/*!
 * @file VideoCaptureImageList.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/18
 * @date Last Change:2013/Dec/02.
 */
#ifndef __SKL_VIDEO_CAPTURE_IMAGE_LIST_H__
#define __SKL_VIDEO_CAPTURE_IMAGE_LIST_H__

// STL
#include <vector>


#include "sklcv.h"

namespace skl{

	/*!
	 * @brief 画像列が記述されたファイルリストからファイルを読み込む
	 */
	class VideoCaptureImageList: public VideoCaptureInterface<VideoCaptureImageList>{

		public:
			using _VideoCaptureInterface::get;
			using _VideoCaptureInterface::set;
			VideoCaptureImageList();
			virtual ~VideoCaptureImageList();
			virtual bool open(const std::string& filename);
			inline bool isOpened()const{return !img_list.empty();}
			void release();
			bool grab();
			bool retrieve(cv::Mat& image, int channel=0);
			virtual bool isPseudoCapture(){
				return !(img_list[checked_frame_pos].second);}

			bool set(capture_property_t prop_id,double val);
			double get(capture_property_t prop_id);
		protected:
			int index;
			std::vector<std::pair<size_t,bool> > img_list;
			std::vector<std::string> img_list_;
			int bayer_change;
			int imread_option;
			int checked_frame_pos;
			void generateIndex(const std::vector<std::string>& list,std::vector<std::pair<size_t,bool> >& index);
		private:
			// 画像列が記されたファイルが専門で、
			// カメラなどのデバイスは読み込まない
			inline bool open(int device){return false;}
			bool _forbidFake;
	};

} // skl

#endif // __SKL_VIDEO_CAPTURE_IMAGE_LIST_H__

