/*!
 * @file TableObjectManagerBiBackground.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/10
 * @date Last Change: 2012/Sep/28.
 */
#include "TableObjectManagerBiBackground.h"
#include "skl.h"

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
TableObjectManagerBiBackground::TableObjectManagerBiBackground(
	float __learning_rate,
	float __learning_rate2,
	cv::Ptr<BackgroundSubtractAlgorithm> __bgs_algo,
	cv::Ptr<RegionLabelingAlgorithm> __rl_algo,
	cv::Ptr<HumanDetector> __hd_algo,
	cv::Ptr<RegionLabelingAlgorithm> __srd_algo,
	cv::Ptr<RegionLabelingAlgorithm> __trd_algo,
	cv::Ptr<BackgroundSubtractAlgorithm> __bgs_algo2,
	cv::Ptr<PatchModelBiBackground> __patch_model):TableObjectManagerWithTouchReasoning(__learning_rate,__bgs_algo,__rl_algo,__hd_algo,__srd_algo,__trd_algo,
		static_cast<cv::Ptr<PatchModel> >(__patch_model)),
	_learning_rate2(__learning_rate2),
	_bgs_algo2(__bgs_algo2), _patch_model_ptr2(__patch_model)
{
}

/*!
 * @brief デストラクタ
 */
TableObjectManagerBiBackground::~TableObjectManagerBiBackground(){
}

void TableObjectManagerBiBackground::compute(const cv::Mat& src, cv::Mat& human, std::vector<size_t>& put_objects, std::vector<size_t>& taken_objects){
#ifdef DEBUG_TABLE_OBJECT_MANAGER
	StopWatch timer;
#else
	cv::Mat bgs_image;
	cv::Mat labels;
	cv::Mat static_region_labels;
	cv::Mat no_touched_fg;
#endif

	bg_subtract(src,bgs_image);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "BGS   :" << timer.lap() << std::endl;
	region_num = 
#endif
		_rl_algo->compute(src,bgs_image,labels);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "LABEL :" <<timer.lap() << std::endl;
#endif

	std::list<size_t> human_region;
	assert(_hd_algo!=NULL);
	cv::Mat human_small = cv::Mat(labels.size(),CV_8UC1);
	human_region = _hd_algo->compute(src,labels,human_small);
#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "HUMAN :" << timer.lap() << std::endl;
#endif

	static_region_labels = cv::Mat(human_small.size(),CV_16SC1);
	size_t object_cand_num = _srd_algo->compute(labels,255-human_small, static_region_labels);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "STATIC:" << timer.lap() << std::endl;
#endif

	cv::Mat object_cand_small = cv::Mat(human_small.size(),CV_16SC1);
	object_cand_num = _trd_algo->compute(static_region_labels,human_small,object_cand_small);

	// mat type = CV_32FC1
	cv::Mat non_update_mask_small = getLabelDiff(labels,object_cand_small);
	// mat type = CV_32FC1
	cv::Mat no_touched_fg_small = getLabelDiff(static_region_labels,object_cand_small);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "TOUCH:" << timer.lap() << std::endl;
#endif

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
		no_touched_fg = skl::resize_label<float>(no_touched_fg_small,scale);
	}
	else{
		human = human_small;
		object_cand = object_cand_small;
		non_update_mask = non_update_mask_small;
		no_touched_fg = no_touched_fg_small;
	}

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "RESIZE:" << timer.lap() << std::endl;
#endif

	_patch_model_ptr2->setObjectLabels(
			src,
			human,
			object_cand,
			object_cand_num,
			&put_objects,
			&taken_objects);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "PATCH :" << timer.lap() << std::endl;
#endif

	cv::Mat mask = _patch_model->updated_mask();

	no_touched_fg = Patch::blur_mask(no_touched_fg,PATCH_DILATE);

	non_update_mask = Patch::blur_mask(non_update_mask,PATCH_DILATE);
	non_update_mask = cv::max(_patch_model->updated_mask(),non_update_mask);
	non_update_mask = cv::max(no_touched_fg,non_update_mask);
	non_update_mask *= _learning_rate;
	non_update_mask += 1.0 - _learning_rate;

#ifdef _DEBUG
	update_mask = cv::Mat(non_update_mask.size(),CV_8UC1);
	non_update_mask.convertTo(update_mask,CV_8UC1,255);
	update_mask = cv::Scalar(255.0) - update_mask;
#endif

	if(_bg2.size()!=src.size()){
		_bg2 = cv::Mat(src.size(),CV_8UC3);
	}
	blending<cv::Vec3b,float>(src,_patch_model_ptr2->latest_bg2(),no_touched_fg,_bg2);
	_bg = _patch_model->latest_bg();

	assert(non_update_mask.type()==no_touched_fg.type());
	assert(non_update_mask.size()==no_touched_fg.size());

#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "MASK  :" << timer.lap() << std::endl;
#endif

	bg_update(src,non_update_mask,no_touched_fg);
#ifdef DEBUG_TABLE_OBJECT_MANAGER
	std::cerr << "UPDATE :" << timer.stop() << std::endl;
#endif
}

void TableObjectManagerBiBackground::bg_subtract(const cv::Mat& src, cv::Mat& dest){
	cv::Mat result;
	_bgs_algo->compute(src,result);
	_bgs_algo2->compute(src,dest);
	dest &= result;
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
		_bg = cv::Mat(src.size(),CV_8UC3);
	}
	if(_bg2.size()!=src.size()){
		_bg2 = cv::Mat(src.size(),CV_8UC3);
	}
	int size = src.rows * src.cols;

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

	_bgs_algo->updateBackgroundModel(_bg);
	_patch_model->latest_bg(_bg);

	_bgs_algo2->updateBackgroundModel(_bg2);
	_patch_model_ptr2->latest_bg2(_bg2);

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
			bval2 = blend<cv::Vec3b>(bval2,sval,1.0-weight,weight);
			bg2.at<cv::Vec3b>(y,x) = bval2;
		}
		else if(ssum > bsum1){
			if(weight==0){
				weight = (1.0 - non_update_mask.at<float>(y,x)) * _learning_rate;
			}
//			std::cout << "weight for light:" << weight << std::endl;
			bval1 = blend<cv::Vec3b>(bval1,sval,1.0-weight,weight);
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
			bval1 = blend<cv::Vec3b>(sval,bval1,_learning_rate2,1.f - _learning_rate2);
			bg1.at<cv::Vec3b>(y,x) = bval1;
			bval2 = blend<cv::Vec3b>(sval,bval2,_learning_rate2,1.f - _learning_rate2);
			bg2.at<cv::Vec3b>(y,x) = bval2;
		}
	}
}
