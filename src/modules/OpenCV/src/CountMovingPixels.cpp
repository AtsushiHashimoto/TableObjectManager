/*!
 * @file CountMovingPixels.cpp
 * @author a_hasimoto
 * @date Date Created: 2013/Nov/21
 * @date Last Change: 2014/Jan/10.
 */
#include "CountMovingPixels.h"

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
CountMovingPixels::CountMovingPixels(float thresh, int step, int life,bool shallow_copy):
	_thresh(thresh),
	_step(step),
	_shallow_copy(step),
	prevs(life),
	phase(0)
{
	for(size_t i=0;i<prevs.size();i++){
		prevs[i] = cv::Mat();
	}
}

/*!
 * @brief デストラクタ
 */
CountMovingPixels::~CountMovingPixels(){

}


template<class PixelValue> size_t countMovingPixels_Vec(const cv::Mat& img,const cv::Mat& prev, int step, float thresh){
	float dist = 0.f;
	size_t count = 0;

	assert(img.channels()>1);
	for(int y=0;y<img.rows;y+=step){
		const PixelValue *curr_img = img.ptr<PixelValue>(y);
		const PixelValue *prev_img = prev.ptr<PixelValue>(y);
		for(int x=0;x<img.cols;x+=step){
			dist = std::sqrt(cv::norm(curr_img[x]-prev_img[x]));
			if(dist > thresh){
				count++;
			}
		}
	}
	return count;
}

template<class PixelValue> size_t countMovingPixels_Scalar(const cv::Mat& img,const cv::Mat& prev, int step,float thresh){
	float dist = 0.f;
	size_t count = 0;

	assert(img.channels()==1);
	for(int y=0;y<img.rows;y+=step){
		const PixelValue *curr_img = img.ptr<PixelValue>(y);
		const PixelValue *prev_img = prev.ptr<PixelValue>(y);
		for(int x=0;x<img.cols;x+=step){
			dist = std::abs(static_cast<float>(curr_img[x])-static_cast<float>(prev_img[x]));
//			std::cerr << dist << std::endl;
			if(dist>thresh){
				count++;
			}
		}
	}
	return count;
}


float CountMovingPixels::compute(const cv::Mat& img){
	size_t moving_pixel_count =0;

	cv::Mat& prev = prevs[phase];
	if(prev.empty()){
		cv::bitwise_not(img,prev);
	}

//	cv::imshow("prev",prev);
//	cv::imshow("curr",img);
//	cv::waitKey(1000);

	if(img.channels()==1){
		cvMatDepthTemplateCall(img.depth(),countMovingPixels_Scalar,moving_pixel_count, img, prev, _step, _thresh);
	}
	else{
		cvMatDepthTemplateCall4Vec(img.depth(),countMovingPixels_Vec,moving_pixel_count, img, prev, _step, _thresh);
	}

	phase++;
	if(phase==prevs.size()) phase=0;
	prev = prevs[phase];
	if(_shallow_copy){
		prev = img;
	}
	else{
		prev = img.clone();
	}
//	std::cerr << moving_pixel_count << std::endl;
	return static_cast<float>(moving_pixel_count*_step*_step)/(img.rows*img.cols);
}

