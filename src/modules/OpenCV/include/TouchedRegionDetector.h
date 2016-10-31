/*!
 * @file TouchedRegionDetector.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/06
 * @date Last Change:2012/Jul/25.
 */
#ifndef __SKL_TOUCHED_REGION_DETECTOR_H__
#define __SKL_TOUCHED_REGION_DETECTOR_H__

#include <list>
#include "FilterMat2Mat.h"
#include "MotionHistory.h"

namespace skl{

/*!
 * @brief 与えられたマスクから最近触られた物体のみを得る(入力はshort型のlabel画像
 */
 class TouchedRegionDetector: public FilterMat2Mat<size_t>{

	public:
		TouchedRegionDetector(int length_of_memory=8);
		virtual ~TouchedRegionDetector();

		size_t compute(const cv::Mat& object_labels,const cv::Mat& human_mask, cv::Mat& dest);
		inline void clear(){motion_history_algo.clear();}
		inline void length_of_memory(int _length_of_memory){motion_history_algo.history_length(_length_of_memory);}
		inline int length_of_memory()const{return motion_history_algo.history_length();}
		inline const cv::Mat motion_history_image()const {return motion_history_algo.motion_history_image();}
	protected:
		MotionHistory motion_history_algo;
	private:
		
};

} // skl

#endif // __SKL_TOUCHED_REGION_DETECTOR_H__

