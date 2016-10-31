/*!
 * @file TexCut.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/25
 * @date Last Change:2012/Aug/02.
 */
#ifndef __SKL_GPU_TEX_CUT_H__
#define __SKL_GPU_TEX_CUT_H__
#ifdef _DEBUG
//#define DEBUG_GPU_TEXCUT
#endif

#include <cv.h>
#include <highgui.h>
#include <opencv2/gpu/gpu.hpp>
#include "sklcv.h"
#include "FilterGpuMat2GpuMat.h"

namespace skl{
	namespace gpu{
		/*!
		 * @brief GPU上でTexCutによる背景差分を行うプログラム
		 */
		class TexCut: public FilterGpuMat2GpuMat<bool>{
			public:
				using FilterGpuMat2GpuMat<bool>::compute;
				TexCut(float alpha=1.5, float smoothing_term_weight=1.0, float thresh_tex_diff = 0.4,unsigned char over_exposure_thresh = 248,unsigned char under_exposure_thresh = 8, bool doSmoothing = true);
				~TexCut();
				void setParams(float alpha=1.5,float smoothing_term_weight=1.0, float thresh_tex_diff = 0.4, unsigned char over_exposure_thresh = 248,unsigned char under_exposure_thresh = 8);

				void setNoiseModel(
						const std::vector<float>& noise_std_dev,
						const std::vector<float>& gh_expectation,
						const std::vector<float>& gh_std_dev);

				virtual bool compute(const cv::gpu::GpuMat& src, cv::gpu::GpuMat& dest, cv::gpu::Stream& stream_external=cv::gpu::Stream::Null());

				/* INTERFACES by cv::gpu::GpuMat */
				TexCut(const cv::gpu::GpuMat& bg1, const cv::gpu::GpuMat& bg2, float alpha=1.0, float smoothing_term_weight=1.0, float thresh_tex_diff = 0.4,unsigned char over_exposure_thresh = 248,unsigned char under_exposure_thresh = 8, bool doSmoothing=true);
				void setBackground(const cv::gpu::GpuMat& bg,bool noSmoothing=false);
				void learnImageNoiseModel(const cv::gpu::GpuMat& bg2);
				inline void updateBackgroundModel(const cv::gpu::GpuMat& img){setBackground(img,true);}

				/* Interfaces with cv::Mat */
//				TexCut(const cv::Mat& bg1, const cv::Mat& bg2, float alpha=1.0, float smoothing_term_weight=1.0, float thresh_tex_diff = 0.4,unsigned char over_exposure_thresh = 248,unsigned char under_exposure_thresh = 8, doSmoothing=true);

				inline void setBackground(const cv::Mat& bg){setBackground(cv::gpu::GpuMat(bg));}
				inline void learnImageNoiseModel(const cv::Mat& bg2){learnImageNoiseModel(cv::gpu::GpuMat(bg2));}

				inline void updateBackgroundModel(const cv::Mat& img){updateBackgroundModel(cv::gpu::GpuMat(img));}
				inline cv::Mat background()const{
					std::vector<cv::Mat> temp(_background.size());
					for(size_t c=0;c<temp.size();c++){
						temp[c] = cv::Mat(_background[c]);
					}
					cv::Mat __background;
					cv::merge(temp,__background);
					return __background;
				}
				inline bool doSmoothing()const{return _doSmoothing;}
				inline void doSmoothing(bool __doSmoothing){_doSmoothing = __doSmoothing;}

				cv::gpu::Stream& getStreamSetBackground(void) { return stream_setBackground; }

				// For count time
				double * t_compute;
				double t_bg[2];
			protected:
				bool _doSmoothing;
				std::vector<cv::gpu::GpuMat> _background;
				std::vector<cv::gpu::GpuMat> _bg_blur_temp;
				std::vector<cv::gpu::GpuMat> _bg_sobel_x;
				std::vector<cv::gpu::GpuMat> _bg_sobel_y;
				cv::gpu::GpuMat _bg_buf_sobel_x;
				cv::gpu::GpuMat _bg_buf_sobel_y;
				std::vector<cv::gpu::GpuMat> _bg_tex_intencity;
				std::vector<cv::gpu::GpuMat> _bg_gradient_heterogenuity;
				std::vector<float> noise_std_dev;
				std::vector<float> gh_expectation;
				std::vector<float> gh_std_dev;

				float alpha;
				float smoothing_term_weight;
				float thresh_tex_diff;
				unsigned char over_exposure_thresh;
				unsigned char under_exposure_thresh;

				cv::Size graph_size;
				cv::gpu::GpuMat terminals;
				cv::gpu::GpuMat rightTransp;
				cv::gpu::GpuMat leftTransp;
				cv::gpu::GpuMat bottom;
				cv::gpu::GpuMat top;

				cv::gpu::GpuMat max_intencity;
				cv::gpu::GpuMat max_gradient_heterogenuity;
				cv::gpu::GpuMat fg_is_over_exposure;
				cv::gpu::GpuMat fg_is_under_exposure;
				cv::gpu::GpuMat bg_is_over_exposure;
				cv::gpu::GpuMat bg_is_under_exposure;

				cv::gpu::Stream stream_setBackground;

				void alloc_gpu(
						const cv::Size& img_size,
						size_t nChannels);

				skl::Graphcut gc_algo;
			private:
				std::vector<cv::Rect> img_rects;
				std::vector<cv::Rect> graph_rects;
				size_t stream_num;

				std::vector<cv::gpu::GpuMat> splitmats;
				// buffer for calculation
				std::vector<cv::gpu::GpuMat> blur_temp;
				std::vector<cv::gpu::GpuMat> sobel_x;
				std::vector<cv::gpu::GpuMat> sobel_y;
				std::vector<cv::gpu::GpuMat> fg_tex_intencity;
				std::vector<cv::gpu::GpuMat> textural_correlation;
				std::vector<cv::gpu::GpuMat> fg_gradient_heterogenuity;
				std::vector<cv::gpu::GpuMat> sterm_x;
				std::vector<cv::gpu::GpuMat> sterm_y;

				cv::gpu::GpuMat buf_graphcut;
				cv::gpu::GpuMat buf_sobel_x;
				cv::gpu::GpuMat buf_sobel_y;
				cv::gpu::GpuMat buf_intencity_sobel_x;
				cv::gpu::GpuMat buf_intencity_sobel_y;
		};

	} // skl
} // gpu
#endif // __SKL_GPU_TEX_CUT_H__

