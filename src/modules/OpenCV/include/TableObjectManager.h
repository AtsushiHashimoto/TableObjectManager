#ifndef __SKL_TABLE_OBJECT_MANAGER_H__
#define __SKL_TABLE_OBJECT_MANAGER_H__

#include <list>
#include "TexCut.h"
#include "RegionLabelingSimple.h"
#include "HumanDetectorWorkspaceEnd.h"
#include "StaticRegionDetector.h"
#include "PatchModel.h"


#ifdef _DEBUG
#define DEBUG_TABLE_OBJECT_MANAGER
#endif

namespace skl{

	class TableObjectManager{
		public:
			typedef FilterMat2Mat<size_t> RegionLabelingAlgorithm;
			typedef FilterMat2Mat<std::list<size_t> > HumanDetector;
			TableObjectManager(
					float __learning_rate = 0.05,
					cv::Ptr<BackgroundSubtractAlgorithm> bgs_algo = new TexCut(),
					cv::Ptr<RegionLabelingAlgorithm> region_labeling_algo = new RegionLabelingSimple(),
					cv::Ptr<HumanDetector> human_detect_algo = new HumanDetectorWorkspaceEnd(),
					cv::Ptr<RegionLabelingAlgorithm> static_region_detect_algo = new StaticRegionDetector(),
					cv::Ptr<PatchModel> patch_model = new PatchModel());
			~TableObjectManager();

			void compute(const cv::Mat& src, cv::Mat& human, std::vector<size_t>& put_objects, std::vector<size_t>& taken_objects);


#ifdef DEBUG_TABLE_OBJECT_MANAGER
			cv::Mat bgs_image;
			cv::Mat labels;
			int region_num;
			cv::Mat static_region_labels;
#endif // DEBUG_TABLE_OBJECT_MANAGER
			inline const cv::Ptr<BackgroundSubtractAlgorithm>& bgs_algo()const{return _bgs_algo;}
			inline void bgs_algo(const cv::Ptr<BackgroundSubtractAlgorithm>& __bgs_algo){_bgs_algo = __bgs_algo;}
			inline const cv::Ptr<RegionLabelingAlgorithm>& rl_algo()const{return _rl_algo;}
			inline void rl_algo(const cv::Ptr<RegionLabelingAlgorithm>& __rl_algo){_rl_algo = __rl_algo;}
			inline const cv::Ptr<HumanDetector>& hd_algo()const{return _hd_algo;}
			inline void hd_algo(const cv::Ptr<HumanDetector>& __hd_algo){_hd_algo = __hd_algo;}
			inline const cv::Ptr<RegionLabelingAlgorithm>& srd_algo()const{return _srd_algo;}
			inline void srd_algo(const cv::Ptr<RegionLabelingAlgorithm>& __srd_algo){_srd_algo = __srd_algo;}
			inline const cv::Ptr<PatchModel>& patch_model()const{return _patch_model;}
			inline cv::Ptr<PatchModel>& mutable_patch_model(){return _patch_model;}
			inline void patch_model(const cv::Ptr<PatchModel>& __patch_model){_patch_model = __patch_model;}
			inline float learning_rate()const{return _learning_rate;}
			inline void learning_rate(float __learning_rate){_learning_rate = __learning_rate;}

			inline const cv::Mat& bg()const{return _bg;}
		protected:
			cv::Ptr<BackgroundSubtractAlgorithm> _bgs_algo;
			cv::Ptr<RegionLabelingAlgorithm> _rl_algo;
			cv::Ptr<HumanDetector> _hd_algo;
			cv::Ptr<RegionLabelingAlgorithm> _srd_algo;
			cv::Ptr<PatchModel> _patch_model;
			float _learning_rate;
			cv::Mat _bg;

			static cv::Mat getLabelDiff(const cv::Mat& label1,const cv::Mat& label2);
			void bg_subtract(const cv::Mat& src, cv::Mat& dest);
			void bg_update(const cv::Mat& src, const cv::Mat& non_update_mask);
};

}

#endif // __TABLE_OBJECT_MANAGER_H__
