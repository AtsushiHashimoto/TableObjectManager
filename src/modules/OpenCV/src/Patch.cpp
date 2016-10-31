/*!
 * @file Patch.cpp
 * @author a_hasimoto
 * @date Last Change:2012/Sep/28.
 */

#include "PatchModel.h"
#include "skl.h"
#include "sklcv.h"
#include <highgui.h>
#include <iostream>

using namespace skl;

Patch::Patch(){

}

Patch::~Patch(){

}

Patch::Patch(const cv::Mat& mask, const cv::Mat& img, const cv::Mat& fg_edge, const cv::Rect& roi){
	set(mask,img,fg_edge,roi);
}

Patch::Patch(const Patch& other){
	for(size_t i=0;i<2;i++){
		_mask[i] = other._mask[i];
		_image[i] = other._image[i];
		_roi[i] = other._roi[i];
	}
	_edge = other._edge;
	_covered_state = other._covered_state;
	base_size = other.base_size;
	_points = other._points;
	_edge_points = other._edge_points;
}

void Patch::cvtG2L(int* x,int* y, Type type)const{
	*x -= _roi[type].x;
	*y -= _roi[type].y;
}

bool Patch::isIn(int local_x,int local_y, Type type)const{
	if( local_x < 0 || _roi[type].width <= local_x ) return false;
	if( local_y < 0 || _roi[type].height <= local_y ) return false;
	return true;
}

void Patch::setCoveredState(const cv::Rect& rect, const cv::Mat& mask, bool isCovered){
	assert(rect && _roi[original]);
	cv::Rect common_rect = rect & _roi[original];
	common_rect.height += common_rect.y;
	common_rect.width += common_rect.x;

	for(int y = common_rect.y; y < common_rect.height; y++){
		for(int x= common_rect.x; x < common_rect.width; x++){
			if(mask.at<float>(y - rect.y,x - rect.x)>0.0){
				setCoveredState(x, y, isCovered);
			}
		}
	}

	//CHECK!
//	cvShowImage("visibility",this->visibility.getIplImage());
//	cvShowImage("patch",this->image[original].getIplImage());
//	cvWaitKey(-1);
}

void Patch::setCoveredState(int absx,int absy,bool isCovered){
	CvRect roi = _roi[original];
	assert(roi.x <= absx && absx < roi.x + roi.width);
	assert(roi.y <= absy && absy < roi.y + roi.height);
	int lx = absx;
	int ly = absy;
	cvtG2L(&lx,&ly,original);

	float val = 0.0f;
	if(isCovered) val = 1.0f;
	_covered_state.at<float>(ly,lx) = val;
}



void Patch::set(const cv::Mat& __mask, const cv::Mat& img, const cv::Mat& fg_edge, const cv::Rect& roi){
	_roi[original] = roi;

	base_size.width = __mask.cols;
	base_size.height = __mask.rows;

	_roi[dilate] = cv::Rect(
		roi.x - PATCH_DILATE,
		roi.y - PATCH_DILATE,
		roi.x + roi.width + PATCH_DILATE,
		roi.y + roi.height + PATCH_DILATE);
	_roi[dilate].x = _roi[dilate].x > 0 ? _roi[dilate].x : 0;
	_roi[dilate].y = _roi[dilate].y > 0 ? _roi[dilate].y : 0;
	_roi[dilate].width = _roi[dilate].width < img.cols ? _roi[dilate].width - _roi[dilate].x : img.cols - _roi[dilate].x;
	_roi[dilate].height = _roi[dilate].height < img.rows ? _roi[dilate].height - _roi[dilate].y : img.rows - _roi[dilate].y;

	cv::Rect relative_roi(
			_roi[original].x - _roi[dilate].x,
			_roi[original].y - _roi[dilate].y,
			_roi[original].width, _roi[original].height);

	_image[dilate] = cv::Mat(img, _roi[dilate]).clone();
	_mask[dilate] = cv::Mat::zeros(_image[dilate].size(),CV_32FC1);


	cv::Mat mask_central = cv::Mat(_mask[dilate],relative_roi);
	cv::Mat temp_mask;
	cv::Mat(__mask, _roi[original]).convertTo(temp_mask,CV_32F,1.f/255.f);
	temp_mask.copyTo(mask_central);

/*
	std::cerr << "show dilate" << std::endl;
	cv::imshow("patch_image",_image[dilate]);
	cv::imshow("patch_mask",_mask[dilate]);
	cv::waitKey(-1);
*/

	_mask[dilate] = blur_mask(_mask[dilate],PATCH_DILATE);


	_image[original] = cv::Mat(_image[dilate],relative_roi);
	_mask[original] = cv::Mat(_image[original].size(),CV_32FC1);
	cv::Mat(__mask, _roi[original]).convertTo(_mask[original],CV_32F,1.f/255.f);

	_covered_state = cv::Mat::zeros(_mask[original].size(),CV_32FC1);

	for(int y = 0; y < _mask[original].rows; y += PATCH_MODEL_BLOCK_SIZE){
		for(int x = 0; x < _mask[original].cols; x += PATCH_MODEL_BLOCK_SIZE){
			if(0.0f == _mask[original].at<float>(y,x)) continue;
			_points.push_back(cv::Point(x,y));
		}
	}

	_edge = cv::Mat(fg_edge,roi).clone();
	_edge &= cv::Mat(__mask,_roi[original]);
	getPoints<unsigned char>(_edge,_edge_points);

/*
	std::cerr << "show dilate" << std::endl;
	cv::imshow("patch_image",_image[original]);
	cv::imshow("patch_mask",_mask[original]);
	cv::imshow("patch_covered_state",_covered_state);
	cv::imshow("patch_edge",_edge);
	cv::waitKey(-1);
*/
}


void Patch::save(const std::string& filename,Type type,const std::string& edge_filename)const{
	CvRect roi = _roi[type];
	assert(roi.width <= base_size.width);
	assert(roi.height <= base_size.height);
	cv::Mat dest = cv::Mat(base_size,_image[type].type(),cv::Scalar(SKL_GRAY,SKL_GRAY,SKL_GRAY));
	cv::Mat temp = cv::Mat(dest,_roi[type]);
	skl::blending<cv::Vec3b,float>(_image[type],temp,_mask[type],temp);

	cv::imwrite(filename,dest);
	if(!edge_filename.empty()){
		dest = cv::Mat::zeros(dest.size(),_edge.type());
		temp = cv::Mat(dest,_roi[original]);
		_edge.copyTo(temp);
		cv::imwrite(edge_filename,dest);
	}
}

cv::Mat Patch::blur_mask(const cv::Mat& mask, size_t blur_width){
	cv::Mat dest = mask.clone();
	cv::Size kernel_size(blur_width,blur_width);

	cv::blur(dest, dest, kernel_size);
	if(mask.depth()==CV_8U){
		cv::threshold(dest, dest, 0,255,CV_THRESH_BINARY);
	}
	else if(mask.depth()==CV_32F){
		cv::threshold(dest, dest, 0.0f,1.0f,CV_THRESH_BINARY);
	}
	else{
		bool DepthIsNotSupported = true;
		assert(DepthIsNotSupported);
	}
	cv::blur(dest, dest, kernel_size);

	return dest;
}

/********** Accessor **********/
float Patch::maskValue(int x, int y, Type type)const{
	cvtG2L(&x,&y,type);
	if(!isIn(x,y,type)) return 0.0;
	return _mask[type].at<float>(y,x);
}

void Patch::edge(const cv::Mat& __edge){
	_edge = __edge.clone();
	getPoints<unsigned char>(_edge,_edge_points);
}

cv::Mat Patch::print_image(Type type)const{
	cv::Mat dest = cv::Mat::zeros(base_size, CV_8UC3);
	cv::Mat dest_roi = cv::Mat(dest,_roi[type]);
	for(int y = 0; y < dest_roi.rows; y++){
		for(int x = 0; x < dest_roi.cols; x++){
			if(_mask[type].at<float>(y,x)==0)continue;
			dest_roi.at<cv::Vec3b>(y,x) = _image[type].at<cv::Vec3b>(y,x);
		}
	}
	return dest;
}

cv::Mat Patch::print_mask(Type type)const{
	cv::Mat dest = cv::Mat::zeros(base_size, CV_8UC1);
	cv::Mat dest_roi = cv::Mat(dest,_roi[type]);
	for(int y = 0; y < dest_roi.rows; y++){
		for(int x = 0; x < dest_roi.cols; x++){
			if(_mask[type].at<float>(y,x)==0)continue;
			dest_roi.at<unsigned char>(y,x) = _mask[type].at<float>(y,x) * 255;
		}
	}
	return dest;
}
