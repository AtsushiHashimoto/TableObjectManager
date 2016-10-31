/*!
 * @file MotionHistory.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/06
 * @date Last Change: 2012/Jul/25.
 */
#include "MotionHistory.h"

#ifdef _DEBUG
#define DEBUG_MOTION_HISTORY
#endif

#ifdef DEBUG_MOTION_HISTORY
#include <highgui.h>
#endif

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
MotionHistory::MotionHistory(int __history_length):size(0,0){
	history_length(__history_length);
#ifdef DEBUG_MOTION_HISTORY
	cv::namedWindow("motion_history",0);
#endif
}

/*!
 * @brief デストラクタ
 */
MotionHistory::~MotionHistory(){
#ifdef DEBUG_MOTION_HISTORY
	cv::destroyWindow("motion_history");
#endif
}

void MotionHistory::compute(const cv::Mat& src, const cv::Mat& mask, cv::Mat& dest){
	compute(mask,dest);
}

void MotionHistory::compute(const cv::Mat& mask, cv::Mat& dest){
	compute(mask);
	motion_history_image().copyTo(dest);
}

void MotionHistory::compute(const cv::Mat& mask){
	if(size.width==0){
		size.width = mask.cols;
		size.height = mask.rows;
		prev = cv::Mat::zeros(size,CV_8UC1);
	}
	else{
		prev -= step;
	}
	prev = cv::max(prev,offset * mask);
#ifdef DEBUG_MOTION_HISTORY
	cv::imshow("motion_history",prev);
#endif

}

void MotionHistory::clear(){
	size = cv::Size(0,0);
	prev.release();
}
