#include "TableObjectManagerGpu.h"

#include <highgui.h>
#include "skl.h"


namespace skl{
	namespace gpu{

TableObjectManager::TableObjectManager(
		float __learning_rate,
		cv::Ptr<skl::gpu::TexCut> __bgs_algo,
		cv::Ptr<RegionLabelingAlgorithm> __rl_algo,
		cv::Ptr<HumanDetector> __hd_algo,
		cv::Ptr<RegionLabelingAlgorithm> __srd_algo,
		cv::Ptr<PatchModel> __patch_model):
	skl::TableObjectManager(
			__learning_rate,
			NULL,
			__rl_algo,
			__hd_algo,
			__srd_algo,
			__patch_model),
	_bgs_algo(__bgs_algo),doSetBackground(false){
}

TableObjectManager::~TableObjectManager(){
}

void TableObjectManager::compute(
		const cv::Mat& src,
		const cv::gpu::GpuMat& src_gpu,
		cv::Mat& human,
		std::vector<size_t>& put_objects,
		std::vector<size_t>& taken_objects){
#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	StopWatch timer;
#else
	cv::Mat bgs_image;
	cv::Mat labels;
	cv::Mat static_region_labels;
#endif

	bg_subtract(src_gpu,bgs_image);

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "BGS   :" << timer.lap() << std::endl;
	region_num = 
#endif
		_rl_algo->compute(src,bgs_image,labels);

//	__labels = visualizeRegionLabel(labels,region_num);

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "LABEL :" <<timer.lap() << std::endl;
#endif

	std::list<size_t> human_region;
	assert(_hd_algo!=NULL);
	cv::Mat human_small = cv::Mat(labels.size(),CV_8UC1);
	human_region = _hd_algo->compute(src,labels,human_small);
#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "HUMAN :" << timer.lap() << std::endl;
#endif

//	__human_region = human_small

	static_region_labels = cv::Mat(human_small.size(),CV_16SC1);
	size_t object_cand_num = _srd_algo->compute(labels,255-human_small, static_region_labels);

//	__static_region = visualizeRegionLabel(static_region_labels,object_cand_num);

#ifdef DEBUG_TABLE_OBJECT_MANAGER_WITHOUT_TOUCH_REASONING
	std::cerr << "STATIC:" << timer.lap() << std::endl;
#endif

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
//	_patch_model->updated_mask().convertTo(patch_model_update_mask,CV_8UC1,255,0);
#endif


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

void TableObjectManager::bg_subtract(const cv::gpu::GpuMat& src, cv::Mat& dest){
	if(doSetBackground){
		_bgs_algo->setBackground(bg_for_texcut);
		doSetBackground = false;
	}
	_bgs_algo->compute(src,dest);
}

void TableObjectManager::bg_update(const cv::Mat& src, const cv::Mat& non_update_mask){
	if(_bg.size()!=src.size()){
		_page_locked_bg.create(src.size(),CV_8UC3,cv::gpu::CudaMem::ALLOC_PAGE_LOCKED);
		_bg = _page_locked_bg;
	}
	blending<cv::Vec3b,float>(_patch_model->latest_bg(),src,non_update_mask,_bg);

	_patch_model->latest_bg(_bg);
	doSetBackground = true;
	cv::gpu::ensureSizeIsEnough(_bg.size(),_bg.type(),bg_for_texcut);
	_bgs_algo->getStreamSetBackground().enqueueUpload(_bg,bg_for_texcut);
}

	}
}
