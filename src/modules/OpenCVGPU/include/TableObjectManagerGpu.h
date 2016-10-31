#ifndef __SKL_GPU_TABLE_OBJECT_MANAGER_H__
#define __SKL_GPU_TABLE_OBJECT_MANAGER_H__

#include <list>
#include "TexCut.h"
#include "sklcv.h"

#ifdef _DEBUG
#define DEBUG_TABLE_OBJECT_MANAGER
#endif

namespace skl{
	namespace gpu{
		class TableObjectManager: public skl::TableObjectManager{
			public:
				typedef FilterMat2Mat<size_t> RegionLabelingAlgorithm;
				typedef FilterMat2Mat<std::list<size_t> > HumanDetector;
				TableObjectManager(
						float __learning_rate = 0.05,
						cv::Ptr<skl::gpu::TexCut> bgs_algo = new skl::gpu::TexCut(),
						cv::Ptr<RegionLabelingAlgorithm> region_labeling_algo = new RegionLabelingSimple(),
						cv::Ptr<HumanDetector> human_detect_algo = new HumanDetectorWorkspaceEnd(),
						cv::Ptr<RegionLabelingAlgorithm> static_region_detect_algo = new StaticRegionDetector(),
						cv::Ptr<PatchModel> patch_model = new PatchModel());
				virtual ~TableObjectManager();

				virtual void compute(const cv::Mat& src,const cv::gpu::GpuMat& src_gpu, cv::Mat& human, std::vector<size_t>& put_objects, std::vector<size_t>& taken_objects);

/*				cv::Mat __labels;
				cv::Mat __human_region;
				cv::Mat __static_region;
				cv::Mat __no_touch_fg;
				cv::Mat __object_cand;
				cv::Mat __patch_model_update;
				cv::Mat __non_update_mask;*/
				inline const cv::Ptr<skl::gpu::TexCut>& bgs_algo()const{return _bgs_algo;}
				inline void bgs_algo(const cv::Ptr<skl::gpu::TexCut>& __bgs_algo){_bgs_algo = __bgs_algo;}
			protected:
				cv::Ptr<skl::gpu::TexCut> _bgs_algo;
				virtual void bg_subtract(const cv::gpu::GpuMat& src, cv::Mat& dest);
				virtual void bg_update(const cv::Mat& src, const cv::Mat& non_update_mask);
				cv::gpu::GpuMat bg_for_texcut;
				bool doSetBackground;
				cv::gpu::CudaMem _page_locked_bg;
		};

	} // namespace gpu
} // namespace skl
#endif // __SKL_GPU_TABLE_OBJECT_MANAGER_H__
