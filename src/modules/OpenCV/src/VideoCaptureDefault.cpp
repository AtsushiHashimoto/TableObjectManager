/*!
 * @file VideoCaptureDefault.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/18
 * @date Last Change: 2012/Jan/18.
 */
#include "VideoCaptureDefault.h"

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
VideoCaptureDefault::VideoCaptureDefault():cv::VideoCapture(){

}

/*!
 * @brief デストラクタ
 */
VideoCaptureDefault::~VideoCaptureDefault(){

}

bool VideoCaptureDefault::set(capture_property_t prop_id,double val){
	return params.set(prop_id,val) && cv::VideoCapture::set(prop_id,val);
}


double VideoCaptureDefault::get(capture_property_t prop_id){
	double val = cv::VideoCapture::get(prop_id);
	if(val==0.0) return 0.0;
	params.set(prop_id,val);
	return val;
}

