/*!
 * @file TableObjectManagerWithTouchReasoning.h
 * @author a_hasimoto
 * @date Date Created: 2012/Sep/28
 * @date Last Change:2012/Sep/28.
 */
#ifndef __SKL_GPU_TABLE_OBJECT_MANAGER_WITH_TOUCH_REASONING_H__
#define __SKL_GPU_TABLE_OBJECT_MANAGER_WITH_TOUCH_REASONING_H__


#include "TableObjectManagerGpu.h"


namespace skl{
	namespace gpu{

/*!
 * @class 接触理由付けを利用した机上物体検出
 */
 class TableObjectManagerWithTouchReasoning: public skl::gpu::TableObjectManager{

	public:
		typedef FilterMat2Mat<size_t> RegionLabelingAlgorithm;
		typedef FilterMat2Mat<std::list<size_t> > HumanDetector;
		TableObjectManagerWithTouchReasoning(
				float __learning_rate = 0.05,
				cv::Ptr<skl::gpu::TexCut> bgs_algo = new TexCut(),
				cv::Ptr<RegionLabelingAlgorithm> rl_algo = new RegionLabelingSimple(),
				cv::Ptr<HumanDetector> human_detect_algo = new HumanDetectorWorkspaceEnd(),
				cv::Ptr<RegionLabelingAlgorithm> static_region_detect_algo = new StaticRegionDetector(),
				cv::Ptr<RegionLabelingAlgorithm> touched_region_detect_algo = new TouchedRegionDetector(),
				cv::Ptr<PatchModel> patch_model = new PatchModel());
		virtual ~TableObjectManagerWithTouchReasoning();
		void compute(
				const cv::Mat& src,
				const cv::gpu::GpuMat& src_gpu,
				cv::Mat& human,
				std::vector<size_t>& put_objects,
				std::vector<size_t>& taken_objects);
		inline const cv::Ptr<RegionLabelingAlgorithm>& trd_algo()const{return _trd_algo;}
		inline void trd_algo(const cv::Ptr<RegionLabelingAlgorithm>& __trd_algo){_trd_algo = __trd_algo;}
	protected:
		cv::Ptr<RegionLabelingAlgorithm> _trd_algo;


	private:
		
};

	} // skl
} // gpu

#endif // __SKL_GPU_TABLE_OBJECT_MANAGER_WITH_TOUCH_REASONING_H__

