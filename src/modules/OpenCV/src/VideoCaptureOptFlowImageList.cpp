/*!
 * @file VideoCaptureOptFlowImageList.cpp
 * @author yamamoto
 * @date Date Created: 2012/May/25
 * @date Last Change: 2013/Nov/23.
 */
#include "VideoCaptureOptFlowImageList.h"
#include <fstream>
#include <string>

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
VideoCaptureOptFlowImageList::VideoCaptureOptFlowImageList():hasFlow(false){

}

/*!
 * @brief デストラクタ
 */
VideoCaptureOptFlowImageList::~VideoCaptureOptFlowImageList(){

}

bool VideoCaptureOptFlowImageList::open(const std::string& filename){
	release();
	std::ifstream fin;
	fin.open(filename.c_str());
	if(!fin){
		std::cerr << "ERROR: failed to open file '" << filename << "'." << std::endl;
		return false;
	}
	std::string str;
	while(fin && std::getline(fin,str)){
		img_list_.push_back(str);
	}
	fin.close();
	generateIndex(img_list_,img_list);
	return params.set(FRAME_COUNT,(double)img_list.size());
}

bool VideoCaptureOptFlowImageList::grab(){
	if(index >= static_cast<int>(img_list.size())){
		std::cerr << index << std::endl;
		return false;
	}

	checked_frame_pos = index;
	hasFlow = flow.read(img_list_[img_list[checked_frame_pos].first]);
#ifdef _DEBUG
	if(!hasFlow){
		std::cerr << "hasFlow fail to read'" << img_list_[img_list[checked_frame_pos].first] << "'." << std::endl;
	}
#endif
/*#ifdef _DEBUG
	std::cerr << checked_frame_pos << ": " << img_list[checked_frame_pos] << ": " << hasFlow << std::endl;
#endif
*/
	index++;
	if(!hasFlow) return false;
	return true;
}

bool VideoCaptureOptFlowImageList::retrieve(cv::Mat& image, int channel){
	if(static_cast<size_t>(checked_frame_pos) >= img_list.size() || checked_frame_pos < 0 || !hasFlow){
		image.release();
		return false;
	}
	else{
		if(Flow::X==channel){
			image = flow.u.clone();
			return true;
		}
		else if(Flow::Y==channel){
			image = flow.v.clone();
			return true;
		}
		else{
			return false;
		}
	}
	return true;
}

