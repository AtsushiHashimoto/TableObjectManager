/*!
 * @file ObjectState.cpp
 * @author a_hasimoto
 * @date Date Created: 2013/Jan/08
 * @date Last Change: 2013/Jan/08.
 */
#include "ObjectState.h"

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
ObjectState::ObjectState(int location_resolution){
	cv::Size size(location_resolution,location_resolution);
	u = cv::Mat::zeros(size,CV_32FC1);
	v = cv::Mat::zeros(size,CV_32FC1);
}

/*!
 * @brief 他のインスタンスからのコンストラクタ．Shallow Copyであることに注意
 */
ObjectState::ObjectState(const Flow& other):Flow(other){
}

/*!
 * @brief デストラクタ
 */
ObjectState::~ObjectState(){

}

cv::Point ObjectState::argmax_location(HandlingState h,float& prob)const{
	cv::Point argmax(0,0);
	for(int y=0;y<resolution();y++){
		const float* p = location(h).ptr<float>(y);
		for(int x=0;x<resolution();x++){
			if(p[x]>prob){
				argmax.x = x;
				argmax.y = y;
				prob = p[x];
			}
		}
	}
	return argmax;
}

cv::Point ObjectState::argmax_location(float& prob)const{
	assert(u.cols == resolution());
	assert(u.rows == resolution());
	assert(v.cols == resolution());
	assert(v.rows == resolution());
	cv::Point argmax(0,0);
	for(int y=0;y<resolution();y++){
		const float* pu = u.ptr<float>(y);
		const float* pv = v.ptr<float>(y);
		for(int x=0;x<resolution();x++){
			float temp = pu[x]+pv[x];
			if(temp>prob){
				argmax.x = x;
				argmax.y = y;
				prob = temp;
			}
		}
	}
	return argmax;
}

float ObjectState::probHandlingState(HandlingState h)const{
	cv::Mat vec;
	cv::reduce(location(h),vec,0,CV_REDUCE_SUM);
	float sum = 0.f;
	float* pvec = vec.ptr<float>(0);
	for(int i=0;i<resolution();i++){
		sum+=pvec[i];
	}
	return sum;
}
