/*!
 * @file TableObjectManagerBiBackground.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/10
 * @date Last Change: 2012/Sep/28.
 */
#include "TableObjectManagerBiBackgroundGpu.h"
#include "skl.h"
#include "sklcv.h"
#include "sklcvgpu_utils.h"

using namespace skl::gpu;

/*!
 * @brief デフォルトコンストラクタ
 */
TableObjectManagerBiBackground::TableObjectManagerBiBackground(
		float __learning_rate,
		float __learning_rate2,
		cv::Ptr<skl::gpu::TexCut> __bgs_algo,
		cv::Ptr<RegionLabelingAlgorithm> __rl_algo,
		cv::Ptr<HumanDetector> __hd_algo,
		cv::Ptr<RegionLabelingAlgorithm> __srd_algo,
		cv::Ptr<RegionLabelingAlgorithm> __trd_algo,
		cv::Ptr<skl::gpu::TexCut> __bgs_algo2,
		cv::Ptr<skl::PatchModelBiBackground> __patch_model):
	TableObjectManagerWithTouchReasoning(__learning_rate,__bgs_algo,__rl_algo,__hd_algo,__srd_algo,__trd_algo,static_cast<cv::Ptr<PatchModel> >(__patch_model)),
	_learning_rate2(__learning_rate2),
	_bgs_algo2(__bgs_algo2), 
	_patch_model_ptr2(__patch_model)
{
	t_bg_subtract = 0.0f;
	t_bg_subtract_compute = 0.0f;
	t_patch = 0.0f;
	t_mask = 0.0f;
	t_bg_update = 0.0f;
	t_bg_update_parfor = 0.0f;
}


/*!
 * @brief デストラクタ
 */
TableObjectManagerBiBackground::~TableObjectManagerBiBackground(){
}

void TableObjectManagerBiBackground::compute(const cv::Mat& src, const cv::gpu::GpuMat& src_gpu, cv::Mat& human, std::vector<size_t>& put_objects, std::vector<size_t>& taken_objects){
#ifdef DEBUG_TABLE_OBJECT_MANAGER
	StopWatch timer;
#else
	cv::Mat bgs_image;
	cv::Mat labels;
	cv::Mat static_region_labels;
#endif
	double st, ed;
	st = cv::getTickCount();
	bg_subtract(src_gpu,bgs_image);

	ed = cv::getTickCount();
	t_bg_subtract += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "BGS   :" << timer.lap() << std::endl;
	region_num = 
#endif
		_rl_algo->compute(src,bgs_image,labels);

	//	__labels = visualizeRegionLabel(labels,region_num);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "LABEL :" <<timer.lap() << std::endl;
	cv::imshow("region_label",visualizeRegionLabel(labels,region_num));
#endif

	std::list<size_t> human_region;
	assert(_hd_algo!=NULL);
	cv::Mat human_small = cv::Mat(labels.size(),CV_8UC1);
	human_region = _hd_algo->compute(src,labels,human_small);
#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "HUMAN :" << timer.lap() << std::endl;
	cv::imshow("human",human_small);
#endif

	//	__human_region = human_small;

	static_region_labels = cv::Mat(human_small.size(),CV_16SC1);
	//size_t static_region_num = 
	_srd_algo->compute(labels,255-human_small, static_region_labels);

	//	__static_region = visualizeRegionLabel(static_region_labels,static_region_num);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "STATIC:" << timer.lap() << std::endl;
#endif

	cv::Mat object_cand_small = cv::Mat(human_small.size(),CV_16SC1);
	size_t object_cand_num = _trd_algo->compute(static_region_labels,human_small,object_cand_small);

	// mat type = CV_32FC1
	cv::Mat non_update_mask_small = getLabelDiff(labels,static_region_labels);
	// mat type = CV_32FC1
	cv::Mat no_touch_fg_small = getLabelDiff(static_region_labels,object_cand_small);

	//	__no_touch_fg = visualizeRegionLabel(no_touch_fg_small,static_region_num - object_cand_num);
	//	__object_cand = visualizeRegionLabel(object_cand_small,object_cand_num);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "TOUCH:" << timer.lap() << std::endl;
	cv::imshow("no_touch_fg",no_touch_fg_small);
#endif

	// set scale of label resolution to src image resolution;
	assert(0 == src.rows % labels.rows);
	int scale = src.rows / labels.rows;

	cv::Mat object_cand,non_update_mask,no_touch_fg;
	if(scale > 1){
		if(human.size()!=src.size()){
			human = cv::Mat::zeros(src.size(),human_small.type());
		}
		skl::resize_label<unsigned char>(human_small,scale,human);
		object_cand = skl::resize_label<short>(object_cand_small,scale);
		non_update_mask = skl::resize_label<float>(non_update_mask_small,scale);
		no_touch_fg = skl::resize_label<float>(no_touch_fg_small,scale);
	}
	else{
		human = human_small;
		object_cand = object_cand_small;
		non_update_mask = non_update_mask_small;
		no_touch_fg = no_touch_fg_small;
	}

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "RESIZE:" << timer.lap() << std::endl;
#endif

	st = cv::getTickCount();
	_patch_model_ptr2->setObjectLabels(
			src,
			human,
			object_cand,
			object_cand_num,
			&put_objects,
			&taken_objects);
	ed = cv::getTickCount();
	t_patch += (ed - st)/cv::getTickFrequency();

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "PATCH :" << timer.lap() << std::endl;
#endif


	st = cv::getTickCount();
	no_touch_fg = Patch::blur_mask(no_touch_fg,PATCH_DILATE);
	non_update_mask = Patch::blur_mask(non_update_mask,PATCH_DILATE);

//	blending<cv::Vec3b,float>(src,patch_model_ptr2->latest_bg2(),no_touch_fg,_bg2);
	copyMatToPageLockedCudaMem(_patch_model_ptr2->latest_bg2(), _page_locked_bg2, _bg2);
	copyMatToPageLockedCudaMem(_patch_model->latest_bg(), _page_locked_bg, _bg);
	ed = cv::getTickCount();
	t_mask += (ed - st)/cv::getTickFrequency();

	assert(non_update_mask.type()==no_touch_fg.type());
	assert(non_update_mask.size()==no_touch_fg.size());

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "MASK  :" << timer.lap() << std::endl;
#endif

	st = cv::getTickCount();
	bg_update(src,non_update_mask,no_touch_fg);
	ed = cv::getTickCount();
	t_bg_update += ( ed - st)/cv::getTickFrequency();

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "UPDATE :" << timer.stop() << std::endl;
	cv::imshow("background",_bg);
	cv::imshow("background2",_bg2);
#endif
}

void TableObjectManagerBiBackground::bg_subtract(const cv::gpu::GpuMat& src, cv::Mat& dest){
	if(doSetBackground){
		_bgs_algo->setBackground(bg_for_texcut);
		_bgs_algo2->setBackground(bg_for_texcut2);
		doSetBackground = false;
	}

	// Do not call compute(GpuMat,Mat,Stream) because it calls synchronous cudaMemcpy.
	double st = cv::getTickCount();
	_bgs_algo->compute(src,_bgs_algo_compute_result,_bgs_algo_compute_stream);
	_bgs_algo2->compute(src,_bgs_algo2_compute_result,_bgs_algo2_compute_stream);
	_bgs_algo_compute_stream.waitForCompletion();
	_bgs_algo2_compute_stream.waitForCompletion();
	double ed = cv::getTickCount();
	t_bg_subtract_compute += (ed - st)/cv::getTickFrequency();

	cv::gpu::bitwise_and(_bgs_algo_compute_result, _bgs_algo2_compute_result, _bitwise_and_result, cv::gpu::GpuMat(), _bitwise_and_stream);
	_bitwise_and_stream.waitForCompletion();
	_bitwise_and_result.download(dest);
}


class bg_update_parallel{
	public:
		bg_update_parallel(
				const cv::Mat& src,
				cv::Mat& bg1,
				cv::Mat& bg2,
				const cv::Mat& non_update_mask,
				const cv::Mat& no_touch_fg,
				float _learning_rate,
				float _learning_rate2
				):src(src),bg1(bg1),bg2(bg2),non_update_mask(non_update_mask),no_touch_fg(no_touch_fg),_learning_rate(_learning_rate),_learning_rate2(_learning_rate2){}
		~bg_update_parallel(){}
		void operator()(const cv::BlockedRange& range)const;
	protected:
		const cv::Mat& src;
		cv::Mat& bg1;
		cv::Mat& bg2;
		const cv::Mat& non_update_mask;
		const cv::Mat& no_touch_fg;
		float _learning_rate;
		float _learning_rate2;
};

void TableObjectManagerBiBackground::bg_update(const cv::Mat& src, const cv::Mat& non_update_mask, const cv::Mat& no_touch_fg){
	if(_bg.size()!=src.size()){
		_page_locked_bg.create(src.size(),CV_8UC3,cv::gpu::CudaMem::ALLOC_PAGE_LOCKED);
		_bg = _page_locked_bg;
	}
	if(_bg2.size()!=src.size()){
		_page_locked_bg2.create(src.size(),CV_8UC3,cv::gpu::CudaMem::ALLOC_PAGE_LOCKED);
		_bg2 = _page_locked_bg2;
	}
	int size = src.rows * src.cols;

	double st = cv::getTickCount();
	cv::parallel_for(
			cv::BlockedRange(0,size),
			bg_update_parallel(
				src,
				_bg,
				_bg2,
				non_update_mask,
				no_touch_fg,
				_learning_rate,
				_learning_rate2));
	double ed= cv::getTickCount();
	t_bg_update_parfor += (ed - st)/cv::getTickFrequency();
	
	_patch_model->latest_bg(_bg);
	_patch_model_ptr2->latest_bg2(_bg2);
	doSetBackground = true;
	cv::gpu::ensureSizeIsEnough(_bg.size(),_bg.type(),bg_for_texcut);
	cv::gpu::ensureSizeIsEnough(_bg2.size(),_bg2.type(),bg_for_texcut2);
	_bgs_algo->getStreamSetBackground().enqueueUpload(_bg,bg_for_texcut);
	_bgs_algo2->getStreamSetBackground().enqueueUpload(_bg2,bg_for_texcut2);
}

void bg_update_parallel::operator()(const cv::BlockedRange& range)const{
	for(int i=range.begin();i<range.end();i++){
		int y = i / src.cols;
		int x = i % src.cols;

		cv::Vec3b sval,bval1,bval2;
		sval = src.at<cv::Vec3b>(y,x);
		bval1 = bg1.at<cv::Vec3b>(y,x);
		bval2 = bg2.at<cv::Vec3b>(y,x);
		int ssum,bsum1,bsum2;
		ssum = sval[0] + sval[1] +sval[2];
		bsum1 = bval1[0] + bval1[1] + bval1[2];
		bsum2 = bval2[0] + bval2[1] + bval2[2];
		double weight = no_touch_fg.at<float>(y,x);
		if(ssum < bsum2){
			if(weight==0){
				weight = (1.0 - non_update_mask.at<float>(y,x)) * _learning_rate;
			}
//			std::cout << "weight for dark :" << weight << std::endl;
			bval2 = skl::blend<cv::Vec3b>(bval2,sval,1.0-weight,weight);
			bg2.at<cv::Vec3b>(y,x) = bval2;
		}
		else if(ssum > bsum1){
			if(weight==0){
				weight = (1.0 - non_update_mask.at<float>(y,x)) * _learning_rate;
			}
//			std::cout << "weight for light:" << weight << std::endl;
			bval1 = skl::blend<cv::Vec3b>(bval1,sval,1.0-weight,weight);
			bg1.at<cv::Vec3b>(y,x) = bval1;
		}
		else{
			if(weight==0){
				weight = (1.0 - non_update_mask.at<float>(y,x)) * _learning_rate * _learning_rate2;
			}
			else{
				weight *= _learning_rate2;
			}
//			std::cout << "weight for both :" << weight << std::endl;
			bval1 = skl::blend<cv::Vec3b>(sval,bval1,_learning_rate2,1.f - _learning_rate2);
			bg1.at<cv::Vec3b>(y,x) = bval1;
			bval2 = skl::blend<cv::Vec3b>(sval,bval2,_learning_rate2,1.f - _learning_rate2);
			bg2.at<cv::Vec3b>(y,x) = bval2;
		}
	}
}