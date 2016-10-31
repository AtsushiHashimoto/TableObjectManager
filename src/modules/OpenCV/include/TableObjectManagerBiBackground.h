/*!
 * @file TableObjectManagerBiBackground.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/10
 * @date Last Change:2012/Sep/28.
 */
#ifndef __SKL_TABLE_OBJECT_MANAGER_BI_BACKGROUND_H__
#define __SKL_TABLE_OBJECT_MANAGER_BI_BACKGROUND_H__


#include "TableObjectManagerWithTouchReasoning.h"
#include "PatchModelBiBackground.h"

namespace skl{

	/*!
	 * @brief 2つの背景を利用して背景差分を行いながら机上物体の出入りを管理するクラス
	 */
	class TableObjectManagerBiBackground: public TableObjectManagerWithTouchReasoning{

		public:
			typedef FilterMat2Mat<size_t> RegionLabelingAlgorithm;
			typedef FilterMat2Mat<std::list<size_t> > HumanDetector;
			TableObjectManagerBiBackground(
					float __learning_rate=0.05,
					float __learning_rate2=0.05,
					cv::Ptr<BackgroundSubtractAlgorithm> bgs_algo = new TexCut(),
					cv::Ptr<RegionLabelingAlgorithm> region_labeling_algo = new RegionLabelingSimple(),
					cv::Ptr<HumanDetector> human_detect_algo = new HumanDetectorWorkspaceEnd(),
					cv::Ptr<RegionLabelingAlgorithm> static_region_detect_algo = new StaticRegionDetector(),
					cv::Ptr<RegionLabelingAlgorithm> touched_region_detect_algo = new TouchedRegionDetector(),
					cv::Ptr<BackgroundSubtractAlgorithm> bgs_algo2 = new TexCut(),
					cv::Ptr<PatchModelBiBackground> patch_model = new PatchModelBiBackground());
			virtual ~TableObjectManagerBiBackground();

			virtual void compute(const cv::Mat& src, cv::Mat& human, std::vector<size_t>& put_objects, std::vector<size_t>& taken_objects);

#ifdef _DEBUG
			cv::Mat update_mask;
#endif
			inline const cv::Mat& bg2()const{return _bg2;}
			float _learning_rate2;//>get,set
		protected:
			cv::Mat _bg2;
			cv::Ptr<BackgroundSubtractAlgorithm> _bgs_algo2;
			cv::Ptr<PatchModelBiBackground> _patch_model_ptr2;
			void bg_subtract(const cv::Mat& src, cv::Mat& dest);
			void bg_update(const cv::Mat& new_bg, const cv::Mat& update_mask, const cv::Mat& no_touch_fg);
	};

} // skl

#endif // __SKL_TABLE_OBJECT_MANAGER_BI_BACKGROUND_H__
