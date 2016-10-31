/*!
 * @file MotionHistory.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/06
 * @date Last Change:2012/Jul/25.
 */
#ifndef __SKL_MOTION_HISTORY_H__
#define __SKL_MOTION_HISTORY_H__


#include "FilterMat2Mat.h"


namespace skl{

/*!
 * @brief MotionHistoryImageを作る
 */
 class MotionHistory: public FilterMat2Mat<void>{

	public:
		MotionHistory(int history_length=8);
		virtual ~MotionHistory();
		void clear();
		void compute(const cv::Mat& mask);
		void compute(const cv::Mat& mask, cv::Mat& dest);

		inline const cv::Mat& motion_history_image()const{return prev;}
		inline int history_length()const{return (offset+1) / step;}
		inline void history_length(int __history_length){
			offset = 255 - 256 % __history_length;
			step = 256 / __history_length;
		}
	protected:
		int step;
		unsigned char offset;
		cv::Mat prev;
		cv::Size size;
	private:
		void compute(const cv::Mat& src, const cv::Mat& mask, cv::Mat& dest);
		
};

} // skl

#endif // __SKL_MOTION_HISTORY_H__

