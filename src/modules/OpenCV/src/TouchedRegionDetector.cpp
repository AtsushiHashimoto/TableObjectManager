/*!
 * @file TouchedRegionDetector.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/06
 * @date Last Change: 2012/Jan/11.
 */
#include "TouchedRegionDetector.h"

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
TouchedRegionDetector::TouchedRegionDetector(int len):motion_history_algo(len){
}

/*!
 * @brief デストラクタ
 */
TouchedRegionDetector::~TouchedRegionDetector(){
}

size_t TouchedRegionDetector::compute(const cv::Mat& object_labels, const cv::Mat& human_mask, cv::Mat& dest){
	assert(object_labels.size()==dest.size());
	assert(object_labels.size()==human_mask.size());
	assert(object_labels.type()==CV_16SC1);
	assert(object_labels.type()==dest.type());
	assert(human_mask.type() == CV_8UC1);

	motion_history_algo.compute(human_mask);

	std::vector<bool> is_touched(1,false);
	for(int y=0;y<object_labels.rows;y++){
		const short* label = object_labels.ptr<const short>(y);
		const unsigned char* phuman = motion_history_algo.motion_history_image().ptr<const unsigned char>(y);
		for(int x=0;x<object_labels.cols;x++,label++,phuman++){
			if(*label==0)continue;
			if(static_cast<int>(is_touched.size())<=*label){
				is_touched.resize(*label+1,false);
			}
			if(*phuman==0)continue;
			is_touched[*label] = true;
		}
	}

	size_t tid(1);
	std::vector<short> id_map(is_touched.size(),-1);
	for(size_t i=1;i<is_touched.size();i++){
		if(!is_touched[i]) continue;
		id_map[i] = tid;
		tid++;
	}

	if(tid==is_touched.size()){
		object_labels.copyTo(dest);
		return tid-1;
	}

	dest = cv::Scalar(0);

	for(int y=0;y<object_labels.rows;y++){
		const short* label = object_labels.ptr<const short>(y);
		short* pdest = dest.ptr<short>(y);
		for(int x=0;x<object_labels.cols;x++,label++,pdest++){
			if(!is_touched[*label]) continue;
			*pdest = id_map[*label];
		}
	}

	return tid-1;
}
