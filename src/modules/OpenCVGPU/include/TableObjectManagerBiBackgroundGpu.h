/*!
 * @file TableObjectManagerBiBackground.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/10
 * @date Last Change:2012/Sep/28.
 */
#ifndef __SKL_GPU_TABLE_OBJECT_MANAGER_BI_BACKGROUND_H__
#define __SKL_GPU_TABLE_OBJECT_MANAGER_BI_BACKGROUND_H__

#include "TableObjectManagerWithTouchReasoningGpu.h"
#include "TexCut.h"
#include "sklcv.h"

namespace skl{
	namespace gpu{
	/*!
	 * @brief 2つの背景を利用して背景差分を行いながら机上物体の出入りを管理するクラス
	 */
	class TableObjectManagerBiBackground: public skl::gpu::TableObjectManagerWithTouchReasoning{

		public:
			typedef FilterMat2Mat<size_t> RegionLabelingAlgorithm;
			typedef FilterMat2Mat<std::list<size_t> > HumanDetector;
			TableObjectManagerBiBackground(
					float __learning_rate=0.05,
					float __learning_rate2=0.1,
					cv::Ptr<skl::gpu::TexCut> bgs_algo = new skl::gpu::TexCut(),
					cv::Ptr<RegionLabelingAlgorithm> region_labeling_algo = new RegionLabelingSimple(),
					cv::Ptr<HumanDetector> human_detect_algo = new HumanDetectorWorkspaceEnd(),
					cv::Ptr<RegionLabelingAlgorithm> static_region_detect_algo = new StaticRegionDetector(),
					cv::Ptr<RegionLabelingAlgorithm> touched_region_detect_algo = new TouchedRegionDetector(),
					cv::Ptr<skl::gpu::TexCut> bgs_algo2 = new skl::gpu::TexCut(),
					cv::Ptr<PatchModelBiBackground> patch_model = new PatchModelBiBackground());
			virtual ~TableObjectManagerBiBackground();
			void compute(const cv::Mat& src, const cv::gpu::GpuMat& src_gpu,cv::Mat& human, std::vector<size_t>& put_objects, std::vector<size_t>& taken_objects);

			inline float learning_rate2(){return _learning_rate2;}
			inline void learning_rate2(float val){_learning_rate2 = val;}
			inline const cv::Ptr<skl::gpu::TexCut>& bgs_algo2()const{return _bgs_algo2;}
			inline void bgs_algo2(const cv::Ptr<skl::gpu::TexCut>& __bgs_algo2){_bgs_algo2 = __bgs_algo2;}
			inline const cv::Mat& bg2()const{return _bg2;}
			inline void bg2(const cv::Mat& __bg2){_bg2 = __bg2;}

			// WatchStop
			double t_bg_subtract;
			double t_bg_subtract_compute;
			double t_patch;
			double t_mask;
			double t_bg_update;
			double t_bg_update_parfor;
			double * time_compute() {
				double * a = (double *)malloc(9 * sizeof(double));
				for(int i = 0; i < 9; i++) {
					a[i] = _bgs_algo->t_compute[i] + _bgs_algo2->t_compute[i];
				}
				return a;
			}

			double * time_bg() {
				double * a = (double *)malloc(2 * sizeof(double));
				for(int i = 0; i < 2; i++) {
					a[i] = _bgs_algo->t_bg[i] + _bgs_algo2->t_bg[i];
				}
				return a;
			}
		protected:
			cv::Mat _bg2;
			float _learning_rate2;
			cv::Ptr<skl::gpu::TexCut> _bgs_algo2;
			cv::Ptr<PatchModelBiBackground> _patch_model_ptr2;
			void bg_subtract(const cv::gpu::GpuMat& src, cv::Mat& dest);
			void bg_update(const cv::Mat& new_bg, const cv::Mat& non_update_mask,const cv::Mat& no_touch_fg);
			cv::gpu::GpuMat bg_for_texcut2;
			cv::gpu::GpuMat _bgs_algo_compute_result;
			cv::gpu::GpuMat _bgs_algo2_compute_result;
			cv::gpu::GpuMat _bitwise_and_result;
			cv::gpu::Stream _bgs_algo_compute_stream;
			cv::gpu::Stream _bgs_algo2_compute_stream;
			cv::gpu::Stream _bitwise_and_stream;
			cv::gpu::CudaMem _page_locked_bg2;
	};
	} // namespace gpu
} // namespace skl

#endif // __SKL_TABLE_OBJECT_MANAGER_BI_BACKGROUND_H__
