/*!
 * @file VideoCapture.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/12
 * @date Last Change: 2013/Dec/02.
 */
#include "sklVideoCapture.h"
#include "VideoCaptureDefault.h"
#include "VideoCaptureImageList.h"
#include "VideoCaptureOptFlowImageList.h"
#include <sstream>
#include <fstream>

using namespace skl;

/*!
 * @brief デフォルトコンストラクタ
 */
VideoCapture::VideoCapture():VideoCaptureInterface<VideoCapture>(){
}

/*!
 * @brief デストラクタ
 */
VideoCapture::~VideoCapture(){
	release();
}


bool VideoCapture::isOpened()const{
	if(size()==0) return false;
	for(size_t i=0;i<size();i++){
		if(cam_interface[i].empty()) return false;
		if(!cam_interface[i]->isOpened()) return false;
	}
	return true;
}

void VideoCapture::release(){
	for(size_t i=0;i<size();i++){
		if(!cam_interface[i].empty()){
			cam_interface[i]->release();
		}
	}
	cam_interface.clear();
}

class ParallelGrab{
	public:
		ParallelGrab(VideoCapture* _cam,std::vector<bool>* _flags):cam(_cam),flags(_flags){}
		~ParallelGrab(){}
		void operator()(const cv::BlockedRange& r)const{
			for(int i=r.begin();i!=r.end();i++){
				(*flags)[i] = (*cam)[i].grab();
//				std::cerr << i << ": " << flags[i] << std::endl;
			}
		}
	protected:
		VideoCapture* cam;
		std::vector<bool>* flags;
};

bool VideoCapture::grab(){
	std::vector<bool> flags(size(),false);
	cv::parallel_for(cv::BlockedRange(0,size()),ParallelGrab(this,&flags));
	for(size_t i=0;i<size();i++){
		if(!flags[i]) return false;
	}

/*
	for(size_t i=0;i<size();i++){
		if(!cam_interface[i]->grab()) return false;
	}
	*/
	return true;
}

bool VideoCapture::retrieve(cv::Mat& image, int channel){
	if(cam_interface.empty()){
		image.release();
		return false;
	}
	return cam_interface[0]->retrieve(image,channel);
}

bool VideoCapture::set(capture_property_t prop_id,double val){
	for(size_t i=0;i<size();i++){
		if(!cam_interface[i]->set(prop_id,val)) return false;
	}
	return true;
}

double VideoCapture::get(capture_property_t prop_id){
	if(cam_interface.empty()) return 0.0;
	double val = cam_interface[0]->get(prop_id);
	for(size_t i=1;i<size();i++){
		if(val!=cam_interface[i]->get(prop_id)) return 0.0;
	}
	return val;
}

/*
 * @brief this is used to set/get parameters for cameras independently, when VideoCapture manage several cameras.
 * @param device derect camera device.
 * @return an instance which has the same get/set functions, but for camera[device].
 * */
_VideoCaptureInterface& VideoCapture::operator[](int device){
	assert(static_cast<unsigned int>(device) < size());
	return *(cam_interface[device]);
}

bool VideoCapture::push_back(const std::string& filename){
	std::string extname = ExtName(filename);
	cv::Ptr<_VideoCaptureInterface> capture;
	if(extname==".lst" || extname==".lst0"){
		capture = new VideoCaptureImageList();
	}
	else if(extname==".lstf" || extname==".lstflow"){
		capture = new VideoCaptureOptFlowImageList();
	}
	else{
		capture = new VideoCaptureDefault();
	}
	if(!capture->open(filename)){
		return false;
	}
	cam_interface.push_back(capture);

	return true;
}
bool VideoCapture::push_back(const std::string& filename, cv::Ptr<_VideoCaptureInterface> cam){
	if(!cam->open(filename)){
		return false;
	}
	cam_interface.push_back(cam);
	return true;
}

bool VideoCapture::push_back(int device){
	cv::Ptr<_VideoCaptureInterface> video_capture = new VideoCaptureDefault();
	if(!video_capture->open(device)){
		return false;
	}
	cam_interface.push_back(video_capture);
	return true;
}
bool VideoCapture::push_back(int device, cv::Ptr<_VideoCaptureInterface> cam){
	if(!cam->open(device)){
		return false;
	}
	cam_interface.push_back(cam);
	return true;
}


class VideoCapture_parallel_retrieve{
	public:
		VideoCapture_parallel_retrieve(
				std::vector<cv::Ptr<_VideoCaptureInterface> >& cam_interface,
				std::vector<cv::Mat>& mat_vec):cam_interface(cam_interface),mat_vec(mat_vec){}
		~VideoCapture_parallel_retrieve(){}
		void operator()(const cv::BlockedRange& range)const{
			for(int i = range.begin(); i != range.end(); i++){
				cam_interface[i]->retrieve(mat_vec[i],0);
			}
		}
	protected:
		std::vector<cv::Ptr<_VideoCaptureInterface> >& cam_interface;
		std::vector<cv::Mat>& mat_vec;
};

VideoCapture& VideoCapture::operator>>(std::vector<cv::Mat>& mat_vec){
	if(size()!=mat_vec.size()){
		mat_vec.resize(size());
	}
	if(!grab()){
		mat_vec.clear();
	}
	else{
		cv::parallel_for(
				cv::BlockedRange(0,(int)size()),
				VideoCapture_parallel_retrieve(cam_interface,mat_vec)
				);
	}
	return *this;
}
