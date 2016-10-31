/*!
 * @file TableObjectManagerWithTouchReasoning.h
 * @author a_hasimoto
 * @date Date Created: 2012/Sep/27
 * @date Last Change:2012/Sep/27.
 */
#ifndef __SKL_TABLE_OBJECT_MANAGER_WITH_TOUCH_REASONING_H__
#define __SKL_TABLE_OBJECT_MANAGER_WITH_TOUCH_REASONING_H__

#include "TableObjectManager.h"
#include "TouchedRegionDetector.h"


namespace skl{

/*!
 * @class 接触理由付けを用いた(但し棄却領域を用いた背景モデルの学習を伴わない）机上物体管理アルゴリズム
 */
class TableObjectManagerWithTouchReasoning: public TableObjectManager{

	public:
		typedef FilterMat2Mat<size_t> RegionLabelingAlgorithm;
		typedef FilterMat2Mat<std::list<size_t> > HumanDetector;

		TableObjectManagerWithTouchReasoning(
				float learning_rate = 0.05,
				cv::Ptr<BackgroundSubtractAlgorithm> background_subtraction_algo = new TexCut(),
				cv::Ptr<RegionLabelingAlgorithm> region_labeling_algo = new RegionLabelingSimple(),
				cv::Ptr<HumanDetector> human_detect_algo = new HumanDetectorWorkspaceEnd(),
				cv::Ptr<RegionLabelingAlgorithm> static_region_detect_algo = new StaticRegionDetector(),
				cv::Ptr<RegionLabelingAlgorithm> touched_region_detect_algo = new TouchedRegionDetector(),
				cv::Ptr<PatchModel> patch_model = new PatchModel());
		virtual ~TableObjectManagerWithTouchReasoning();

		void compute(
				const cv::Mat& src,
				cv::Mat& human,
				std::vector<size_t>& put_objects,
				std::vector<size_t>& taken_objects);

#ifdef DEBUG_TABLE_OBJECT_MANAGER
		cv::Mat no_touched_fg;
#endif
		inline const cv::Ptr<RegionLabelingAlgorithm>& trd_algo()const{return _trd_algo;}
		inline void trd_algo(const cv::Ptr<RegionLabelingAlgorithm>& __trd_algo){_trd_algo = __trd_algo;}
	protected:
		cv::Ptr<RegionLabelingAlgorithm> _trd_algo;
		
	private:
		
};

} // skl

#endif // __SKL_TABLE_OBJECT_MANAGER_WITH_TOUCH_REASONING_H__

