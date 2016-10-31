#include "TableObjectManager.h"
#include <highgui.h>
#include "skl.h"


using namespace skl;

TableObjectManager::TableObjectManager(
		float __learning_rate,
		cv::Ptr<BackgroundSubtractAlgorithm> __bgs_algo,
		cv::Ptr<RegionLabelingAlgorithm> __rl_algo,
		cv::Ptr<HumanDetector> __hd_algo,
		cv::Ptr<RegionLabelingAlgorithm> __srd_algo,
		cv::Ptr<PatchModel> __patch_model):
	_bgs_algo(__bgs_algo),
	_rl_algo(__rl_algo),
	_hd_algo(__hd_algo),
	_srd_algo(__srd_algo),
	_patch_model(__patch_model),
	_learning_rate(__learning_rate){
}

TableObjectManager::~TableObjectManager(){
}

void TableObjectManager::compute(
		const cv::Mat& src,
		cv::Mat& human,
		std::vector<size_t>& put_objects,
		std::vector<size_t>& taken_objects){
#ifdef DEBUG_TABLE_OBJECT_MANAGER
	StopWatch timer;
#else
	cv::Mat bgs_image;
	cv::Mat labels;
	cv::Mat static_region_labels;
#endif
	bg_subtract(src,bgs_image);

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "BGS   :" << timer.lap() << std::endl;
	region_num = 
#endif
		_rl_algo->compute(src,bgs_image,labels);

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "LABEL :" <<timer.lap() << std::endl;
#endif

	std::list<size_t> human_region;
	assert(_hd_algo!=NULL);
	cv::Mat human_small = cv::Mat(labels.size(),CV_8UC1);
	human_region = _hd_algo->compute(src,labels,human_small);
#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "HUMAN :" << timer.lap() << std::endl;
	cv::imshow("human",human_small);
#endif

	static_region_labels = cv::Mat(human_small.size(),CV_16SC1);
	size_t object_cand_num = _srd_algo->compute(labels,255-human_small, static_region_labels);

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "STATIC:" << timer.lap() << std::endl;
#endif

	//	cv::Mat object_cand_small = cv::Mat(human_small.size(),CV_16SC1);
	cv::Mat object_cand_small = static_region_labels;

	// mat type = CV_32FC1
	cv::Mat non_update_mask_small = getLabelDiff(labels,object_cand_small);

	// set scale of label resolution to src image resolution;
	assert(0 == src.rows % labels.rows);
	int scale = src.rows / labels.rows;

	cv::Mat object_cand,non_update_mask;
	if(scale > 1){
		if(human.size()!=src.size()){
			human = cv::Mat::zeros(src.size(),human_small.type());
		}
		skl::resize_label<unsigned char>(human_small,scale,human);
		object_cand = skl::resize_label<short>(object_cand_small,scale);
		non_update_mask = skl::resize_label<float>(non_update_mask_small,scale);
	}
	else{
		human = human_small;
		object_cand = object_cand_small;
		non_update_mask = non_update_mask_small;
	}

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "RESIZE:" << timer.lap() << std::endl;
#endif

	_patch_model->setObjectLabels(
			src,
			human,
			object_cand,
			object_cand_num,
			&put_objects,
			&taken_objects);

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "PATCH :" << timer.lap() << std::endl;
#endif

	cv::Mat mask = _patch_model->updated_mask();

	non_update_mask = Patch::blur_mask(non_update_mask,PATCH_DILATE);
	non_update_mask = cv::max(_patch_model->updated_mask(),non_update_mask);
	non_update_mask *= _learning_rate;
	non_update_mask += 1.0 - _learning_rate;


#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "MASK  :" << timer.lap() << std::endl;
#endif

	bg_update(src,non_update_mask);

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "SETBG :" << timer.stop() << std::endl;
#endif
}

cv::Mat TableObjectManager::getLabelDiff(const cv::Mat& label1, const cv::Mat& label2){
	assert(label1.size()==label2.size());
	assert(label1.type()==CV_16SC1);
	assert(label1.type()==label2.type());

	// CV_32FC1
	cv::Mat diff_mask = cv::Mat::zeros(label1.size(),CV_32FC1);

	for(int y=0;y<label1.rows;y++){
		for(int x=0;x<label1.cols;x++){
			if(label1.at<short>(y,x)==0) continue;
			if(label2.at<short>(y,x)!=0) continue;
			diff_mask.at<float>(y,x) = 1.0 ;
		}
	}
	return diff_mask;
}

void TableObjectManager::bg_subtract(const cv::Mat& src, cv::Mat& dest){
	_bgs_algo->compute(src,dest);
}

void TableObjectManager::bg_update(const cv::Mat& src, const cv::Mat& non_update_mask){
	if(_bg.size()!=src.size()){
		_bg = cv::Mat(src.size(),CV_8UC3);
	}
	blending<cv::Vec3b,float>(_patch_model->latest_bg(),src,non_update_mask,_bg);

	_patch_model->latest_bg(_bg);
	_bgs_algo->updateBackgroundModel(_bg);
}
