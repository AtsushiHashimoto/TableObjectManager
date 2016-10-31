#ifndef __TEXCUT_H__
#define __TEXCUT_H__
#include "skl.h"
#include <highgui.h>
#include "BackgroundSubtractAlgorithm.h"


#ifdef _DEBUG
#define DEBUG_TEXCUT
#endif

#define TEXCUT_BLOCK_SIZE 4
#define QUANTIZATION_LEVEL SHRT_MAX//32767.0f
#define QUANTIZATION_LEVEL_HARF QUANTIZATION_LEVEL/2
#define GRADIENT_HETEROGENUITY 1000

//#include "../Core/include/graph.h"

namespace skl{
	class TexCut: public BackgroundSubtractAlgorithm{
		public:
			using BackgroundSubtractAlgorithm::compute;
			typedef Graph<int,int,int> TexCutGraph;
			TexCut(float alpha=1.5, float smoothing_term_weight=1.0, float thresh_tex_diff = 0.4,unsigned char over_exposure_thresh = 248,unsigned char under_exposure_thresh = 8,bool doSmoothing = true);
			TexCut(const cv::Mat& bg1, const cv::Mat& bg2, float alpha=1.0, float smoothing_term_weight=1.0, float thresh_tex_diff = 0.4,unsigned char over_exposure_thresh = 248,unsigned char under_exposure_thresh = 8);
			~TexCut();
			virtual void setBackground(const cv::Mat& bg,bool noSmoothing=false);
			void setParams(float alpha=1.5, float smoothing_term_weight=1.5, float thresh_tex_diff = 0.4, unsigned char over_exposure_thresh = 248,unsigned char under_exposure_thresh = 8);
			void learnImageNoiseModel(const cv::Mat& bg2);

			void updateBackgroundModel(const cv::Mat& img);
			inline cv::Mat background()const{return _background;}
			void setNoiseModel(
					const std::vector<float>& noise_std_dev,
					const std::vector<float>& gh_expectation,
					const std::vector<float>& gh_std_dev);
			inline bool doSmoothing()const{return _doSmoothing;}
			inline void doSmoothing(bool __doSmoothing){_doSmoothing = __doSmoothing;}
		protected:
			bool _doSmoothing;
			virtual void compute(const cv::Mat& src,const cv::Mat& mask, cv::Mat& dest);
			void calcEdgeCapacity(
					const std::vector<cv::Mat>& src,
					const std::vector<cv::Mat>& sobel_x,
					const std::vector<cv::Mat>& sobel_y,
					const std::vector<cv::Mat>& bg_img,
					const std::vector<cv::Mat>& bg_sobel_x,
					const std::vector<cv::Mat>& bg_sobel_y,
					const std::vector<float>& noise_std_dev,
					const std::vector<float>& gh_expectation,
					const std::vector<float>& gh_std_dev,
					float alpha,
					float smoothing_term_weight,
					float thresh_tex_diff,
					cv::Mat& data_term,
					cv::Mat& smoothing_term_x,
					cv::Mat& smoothing_term_y);
			int calcGraphCut(const cv::Mat& data_term,const cv::Mat& smoothing_term_x, const cv::Mat& smoothing_y);
			void setResult(const cv::Mat& src,cv::Mat& dest)const;
			void getSobel(
					const std::vector<cv::Mat>& img,
					std::vector<cv::Mat>* sobel_x,
					std::vector<cv::Mat>* sobel_y);
			std::vector<cv::Mat> bg_img;
			std::vector<cv::Mat> bg_sobel_x;
			std::vector<cv::Mat> bg_sobel_y;
			std::vector<float> noise_std_dev;
			std::vector<float> gh_expectation;
			std::vector<float> gh_std_dev;
			float alpha;
			float smoothing_term_weight;
			float thresh_tex_diff;
			unsigned char over_exposure_thresh;
			unsigned char under_exposure_thresh;

			TexCutGraph* g;
			std::vector<std::vector<TexCutGraph::node_id> > nodes;
			cv::Mat _background;
		private:
			void setCapacity(
					TexCutGraph* g,std::vector<std::vector<TexCutGraph::node_id> >& nodes, size_t x,size_t y, const cv::Mat& data_term,const cv::Mat& smoothing_term_x, const cv::Mat& smoothing_term_y);
	};


	/******* Declarations for parallel processing *******/
	class ParallelNoiseEstimate{
		public:
			ParallelNoiseEstimate(
					std::vector<cv::Mat>* img1,
					std::vector<cv::Mat>* img2,
					std::vector<float>* noise_std_dev,
					std::vector<float>* gh_expectation,
					std::vector<float>* gh_std_dev
					);

			void operator()(const cv::BlockedRange& range)const;
		protected:
			std::vector<cv::Mat>* img1;
			std::vector<cv::Mat>* img2;
			std::vector<float>* noise_std_dev;
			std::vector<float>* gh_expectation;
			std::vector<float>* gh_std_dev;

		private:
			void ghNoiseEstimate(
					float noise_std_dev,
					float* gh_expectation,
					float* gh_std_dev)const;

			void noiseEstimate(
					const cv::Mat& img1,
					const cv::Mat& img2,
					float* noise_std_dev)const;
	};

	class ParallelAddGradientHeterogenuity{
		public:
			ParallelAddGradientHeterogenuity(
					const cv::Mat& data_term,
					const cv::Mat& gradient_heterogenuity,
					cv::Mat& smoothing_term_x,
					cv::Mat& smoothing_term_y,
					float smoothing_term_weight);
			void operator()(const cv::BlockedRange& range)const;
			inline int calcDataTermPenalty(int val)const{
				if(val > QUANTIZATION_LEVEL_HARF){
					return 0;
				}
				return QUANTIZATION_LEVEL_HARF - val;
			}
		protected:
			const cv::Mat& data_term;
			const cv::Mat& gradient_heterogenuity;
			cv::Mat& smoothing_term_x;
			cv::Mat& smoothing_term_y;
			float smoothing_term_weight;
		private:
	};
	class ParallelCalcEdgeCapacity{
		public:
			ParallelCalcEdgeCapacity(
					const std::vector<cv::Mat>& src,
					const std::vector<cv::Mat>& sobel_x,
					const std::vector<cv::Mat>& sobel_y,
					const std::vector<cv::Mat>& bg_img,
					const std::vector<cv::Mat>& bg_sobel_x,
					const std::vector<cv::Mat>& bg_sobel_y,
					const std::vector<float>& noise_std_dev,
					const std::vector<float>& gh_expectation,
					const std::vector<float>& gh_std_dev,
					float alpha,
					float thresh_tex_diff,
					unsigned char over_exposure_thresh,
					unsigned char under_exposure_thresh,
					cv::Mat& data_term,
#ifdef DEBUG_TEXCUT
					cv::Mat& tex_int,
#endif
					cv::Mat& gradient_heterogenuity,
					cv::Mat& smoothing_term_x,
					cv::Mat& smoothing_term_y);
			void operator()(const cv::BlockedRange& range)const;
			static float calcGradHetero(std::vector<float>& power);
		protected:
			const std::vector<cv::Mat>& src;
			const std::vector<cv::Mat>& sobel_x;
			const std::vector<cv::Mat>& sobel_y;
			const std::vector<cv::Mat>& bg_img;
			const std::vector<cv::Mat>& bg_sobel_x;
			const std::vector<cv::Mat>& bg_sobel_y;
			const std::vector<float>& noise_std_dev;
			const std::vector<float>& gh_expectation;
			const std::vector<float>& gh_std_dev;
			float alpha;
			float smoothing_term_weight;
			float thresh_tex_diff;
			unsigned char over_exposure_thresh;
			unsigned char under_exposure_thresh;
			cv::Mat& data_term;
#ifdef DEBUG_TEXCUT
			cv::Mat& tex_int;
#endif
			cv::Mat& gradient_heterogenuity;
			cv::Mat& smoothing_term_x;
			cv::Mat& smoothing_term_y;

			void calcDataTerm(
					const cv::Mat& sobel_x, const cv::Mat& sobel_y,
					const cv::Mat& bg_sobel_x, const cv::Mat& bg_sobel_y,
					float nsd,float gh_mean,float gh_sd,
					float* tex_int, float* gh, float* tex_diff)const;
			void calcSmoothingTerm(
					const cv::Mat& src_left, const cv::Mat& src_right,
					const cv::Mat& bg_left, const cv::Mat& bg_right,
					float* smoothing_term, float nsd)const;
			float normalize(float val, float sigma, float mean = 0)const;
			int isOverUnderExposure(const cv::Mat& block)const;
			bool isOverUnderExposure(const std::vector<cv::Mat>& img_planes,const cv::Rect& roi)const;
	};
} // namespace skl
#endif // __TEXCUT_H__

