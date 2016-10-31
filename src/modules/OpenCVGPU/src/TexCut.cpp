/*!
* @file TexCut.cpp
* @author a_hasimoto
* @date Date Created: 2012/Jan/25
* @date Last Change: 2014/Mar/04.
*/
#include "TexCut.h"
#include "skl.h"
#include "sklcvgpu_utils.h"
#include "shared.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/gpu/stream_accessor.hpp>
#include <fstream>
#define GRADIENT_HETEROGENUITY_ITER_TIME 1000

//#define DEBUG_GPU_TEXCUT_STOPWATCH


using namespace skl;

/* Parallel Process by intel tbb */
class ParallelGHNoiseEstimate{
public:
	ParallelGHNoiseEstimate(
		const std::vector<float>& noise_std_dev,
		std::vector<float>* gh_expectation=NULL,
		std::vector<float>* gh_std_dev=NULL
		):noise_std_dev(noise_std_dev),gh_expectation(gh_expectation),gh_std_dev(gh_std_dev){}

	void operator()(const cv::BlockedRange& range)const{
		for(int c=range.begin();c<range.end();c++){
			float ghe,ghsd;
			ghNoiseEstimate(noise_std_dev[c],&ghe,&ghsd);
			gh_expectation->at(c) = ghe;
			gh_std_dev->at(c) = ghsd;
		}
	}
protected:
	const std::vector<float>& noise_std_dev;
	std::vector<float>* gh_expectation;
	std::vector<float>* gh_std_dev;

private:
	void ghNoiseEstimate(
		float noise_std_dev,
		float* gh_expectation,
		float* gh_std_dev)const{
			size_t iteration_time = GRADIENT_HETEROGENUITY_ITER_TIME;
			float moment1(0),moment2(0);
			int elem_num = TEXCUT_BLOCK_SIZE * TEXCUT_BLOCK_SIZE;
			std::vector<float> powers(elem_num,0.0);
			for(size_t i = 0; i < iteration_time; i++){
				for(int e = 0; e < elem_num; e++){
					powers[e] = static_cast<float>(rayleigh_rand(std::sqrt(6.0)*noise_std_dev));
				}
				//				std::sort(powers.begin(),powers.end(),std::greater<float>());
				//				float factor = powers[powers.size()/2];
				float factor = *std::min_element(powers.begin(),powers.end());
				if(factor==0){
					i--;
					continue;
				}
				float val = *std::max_element(powers.begin(),powers.end())/factor;
				moment2 += val;
				moment1 += sqrt(val);
			}
			moment1 /= iteration_time;
			moment2 /= iteration_time;
			*gh_expectation = moment1;
			*gh_std_dev = 2 * sqrt(moment2 - std::pow(moment1,2));
	}
};
/* end of parallel process declaration & definition */

// internal functions calling cuda kernels for TexCut.
namespace skl{
	namespace gpu{
		void calcBGDataTerm_gpu(
			const cv::gpu::DevMem2Di sobel_x,
			const cv::gpu::DevMem2Di sobel_y,
			cv::gpu::DevMem2Df tex_intencity,
			cv::gpu::DevMem2Df gradient_heterogenuity,
			float gh_expectation,
			float gh_std_dev,
			cudaStream_t stream=cudaStream_t(),
			int dev = cv::gpu::getDevice());
		void calcTexturalIntencity_gpu(
			const cv::gpu::DevMem2Di sobel_x,
			const cv::gpu::DevMem2Di sobel_y,
			cv::gpu::DevMem2Df tex_intencity,
			cudaStream_t stream=cudaStream_t(),
			int dev = cv::gpu::getDevice());
		void calcDataTerm_gpu(
			const cv::gpu::DevMem2Di sobel_x,
			const cv::gpu::DevMem2Di sobel_y,
			const cv::gpu::DevMem2Di bg_sobel_x,
			const cv::gpu::DevMem2Di bg_sobel_y,
			cv::gpu::DevMem2Df fg_tex_intencity,
			cv::gpu::DevMem2Df textural_correlation,
			cv::gpu::DevMem2Df gradient_heterogenuity,
			float gh_expectation,
			float gh_std_dev,
			cudaStream_t stream,
			int dev = cv::gpu::getDevice());
		void checkOverExposure_gpu(
			const cv::gpu::PtrStepSzb img,
			cv::gpu::PtrStepSzb is_over_exposure,
			unsigned char thresh,
			cudaStream_t stream=cudaStream_t(),
			int dev = cv::gpu::getDevice());
		void checkUnderExposure_gpu(
			const cv::gpu::PtrStepSzb img,
			cv::gpu::PtrStepSzb is_under_exposure,
			unsigned char thresh,
			cudaStream_t stream=cudaStream_t(),
			int dev = cv::gpu::getDevice());

		void calcSmoothingTermX_gpu(
			const cv::gpu::DevMem2D src,
			const cv::gpu::DevMem2D bg,
			cv::gpu::DevMem2Df sterm,
			float noise_std_dev,
			cudaStream_t stream=cudaStream_t(),
			int dev = cv::gpu::getDevice());
		void calcSmoothingTermY_gpu(
			const cv::gpu::DevMem2D src,
			const cv::gpu::DevMem2D bg,
			cv::gpu::DevMem2Df sterm,
			float noise_std_dev,
			cudaStream_t stream=cudaStream_t(),
			int dev = cv::gpu::getDevice());
		void bindUpDataTerm_gpu(
			cv::gpu::DevMem2Df max_intencity,
			cv::gpu::DevMem2Df max_gradient_heterogenuity,
			cv::gpu::DevMem2Di terminals,
			const cv::gpu::DevMem2Df fg_tex_intencity,
			const cv::gpu::DevMem2Df bg_tex_intencity,
			const cv::gpu::DevMem2Df fg_gradient_heterogenuity,
			const cv::gpu::DevMem2Df bg_gradient_heterogenuity,
			cv::gpu::DevMem2Df textural_correlation,
			float noise_std_dev,
			float thresh_tex_diff,
			const cv::gpu::DevMem2D is_over_under_exposure,
			cudaStream_t stream=cudaStream_t(),
			int dev = cv::gpu::getDevice());
		void bindUpSmoothingTerms_gpu(
			const cv::gpu::DevMem2Di terminals,
			const cv::gpu::DevMem2Df gradient_heterogenuity,
			const cv::gpu::DevMem2Df max_sterm_x,
			const cv::gpu::DevMem2Df max_sterm_y,
			cv::gpu::DevMem2Di rightTransp,
			cv::gpu::DevMem2Di leftTransp,
			cv::gpu::DevMem2Di bottom,
			cv::gpu::DevMem2Di top,
			cudaStream_t stream=cudaStream_t(),
			int dev = cv::gpu::getDevice());
	}
}

// local utility func
inline cv::Size getGraphSize(const cv::Size& img_size){
	return cv::Size(
		divUp(img_size.width,TEXCUT_BLOCK_SIZE),
		divUp(img_size.height,TEXCUT_BLOCK_SIZE));
}

/*!
* @brief default constructor
*/
gpu::TexCut::TexCut(float alpha, float smoothing_term_weight, float thresh_tex_diff,unsigned char over_exposure_thresh,unsigned char under_exposure_thresh,bool doSmoothing):
	_doSmoothing(doSmoothing),
	noise_std_dev(3,3.5f),
	gh_expectation(3,2.3f),
	gh_std_dev(3,1.12f)
{
	setParams(alpha,smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh);
	t_compute = new double[9];
	memset(t_compute,0,9*sizeof(t_compute[0]));
	t_bg[0] = t_bg[1] = 0.0f;
}

/*!
* @brief �ǥ��ȥ饯��
*/
gpu::TexCut::~TexCut(){
	delete[] t_compute;
}

gpu::TexCut::TexCut(const cv::gpu::GpuMat& bg1, const cv::gpu::GpuMat& bg2, float alpha, float smoothing_term_weight, float thresh_tex_diff,unsigned char over_exposure_thresh,unsigned char under_exposure_thresh,bool doSmoothing):_doSmoothing(doSmoothing){
	setParams(alpha,smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh);
	setBackground(bg1);
	learnImageNoiseModel(bg2);
	t_compute = new double[9];
	memset(t_compute,0,9*sizeof(t_compute[0]));
	t_bg[0] = t_bg[1] = 0.0f;
}

void gpu::TexCut::setParams(float alpha,float smoothing_term_weight,float thresh_tex_diff,unsigned char over_exposure_thresh,unsigned char under_exposure_thresh){
	this->alpha = alpha;
	this->smoothing_term_weight = smoothing_term_weight;
	this->thresh_tex_diff = thresh_tex_diff;
	this->over_exposure_thresh = over_exposure_thresh;
	this->under_exposure_thresh = under_exposure_thresh;

	setAlpha(alpha);
	setSmoothingTermWeight(smoothing_term_weight);
	//	cudaSafeCall(cudaMemcpyToSymbol("skl::gpu::alpha",&alpha,sizeof(float)));
	//	cudaSafeCall(cudaMemcpyToSymbol("skl::gpu::smoothing_term_weight",&smoothing_term_weight,sizeof(float)));
}

void gpu::TexCut::setBackground(const cv::gpu::GpuMat& bg, bool noSmoothing){
	assert(CV_8U==bg.depth());

	cv::gpu::split(bg,_bg_blur_temp,stream_setBackground);

	size_t channels = bg.channels();

	_bg_sobel_x.resize(channels);
	_bg_sobel_y.resize(channels);
	_background.resize(channels);

	double st, ed;
	st = cv::getTickCount();
	for(size_t c = 0; c < channels; c++){
		if(_doSmoothing && !noSmoothing){
			cv::gpu::blur(_bg_blur_temp[c],_background[c],cv::Size(3,3), cv::Point(-1,-1),stream_setBackground);
		}
		else{
			_background[c] = _bg_blur_temp[c];
		}
		cv::gpu::Sobel(_background[c], _bg_sobel_x[c], CV_32S, 1, 0, _bg_buf_sobel_x, 3, 1, cv::BORDER_DEFAULT, -1, stream_setBackground);
		cv::gpu::Sobel(_background[c], _bg_sobel_y[c], CV_32S, 0, 1, _bg_buf_sobel_y, 3, 1, cv::BORDER_DEFAULT, -1, stream_setBackground);
	}

	ed =  cv::getTickCount();
	this->t_bg[0] += (ed - st)/cv::getTickFrequency();

	alloc_gpu(bg.size(),bg.channels());
	stream_setBackground.enqueueMemSet(bg_is_over_exposure, cv::Scalar(255));
	stream_setBackground.enqueueMemSet(bg_is_under_exposure, cv::Scalar(255));

	st = cv::getTickCount();
	for(size_t c = 0; c < channels; c++){
		checkOverExposure_gpu(
			_background[c],
			bg_is_over_exposure,
			over_exposure_thresh,
			cv::gpu::StreamAccessor::getStream(stream_setBackground));
		checkUnderExposure_gpu(
			_background[c],
			bg_is_under_exposure,
			under_exposure_thresh,
			cv::gpu::StreamAccessor::getStream(stream_setBackground));
	}

	ed =  cv::getTickCount();
	this->t_bg[1] += (ed - st)/cv::getTickFrequency();
	if(gh_expectation.size()!=channels){
		return;
	}

	for(size_t c = 0; c < channels; c++){
		calcBGDataTerm_gpu(
			_bg_sobel_x[c],_bg_sobel_y[c],
			_bg_tex_intencity[c],
			_bg_gradient_heterogenuity[c],
			gh_expectation[c],
			gh_std_dev[c],
			cv::gpu::StreamAccessor::getStream(stream_setBackground));
	}

}

void gpu::TexCut::alloc_gpu(
	const cv::Size& img_size,
	size_t channels){
		cv::Size graph_size(img_size.width/TEXCUT_BLOCK_SIZE,img_size.height/TEXCUT_BLOCK_SIZE);

		cv::gpu::ensureSizeIsEnough(graph_size,CV_8UC1,bg_is_over_exposure);
		cv::gpu::ensureSizeIsEnough(graph_size,CV_8UC1,bg_is_under_exposure);
		cv::gpu::ensureSizeIsEnough(graph_size,CV_8UC1,fg_is_over_exposure);
		cv::gpu::ensureSizeIsEnough(graph_size,CV_8UC1,fg_is_under_exposure);

		cv::gpu::ensureSizeIsEnough(graph_size,CV_32FC1,max_gradient_heterogenuity);
		cv::gpu::ensureSizeIsEnough(graph_size,CV_32FC1,max_intencity);

		// graph edge capacities
		cv::gpu::ensureSizeIsEnough(graph_size,CV_32SC1,terminals);
		cv::gpu::ensureSizeIsEnough(graph_size.width, graph_size.height, CV_32SC1,rightTransp);
		cv::gpu::ensureSizeIsEnough(graph_size.width, graph_size.height,CV_32SC1,leftTransp);
		cv::gpu::ensureSizeIsEnough(graph_size,CV_32SC1,bottom);
		cv::gpu::ensureSizeIsEnough(graph_size,CV_32SC1,top);

		// resize vector<GpuMat>
		_bg_tex_intencity.resize(channels);
		_bg_gradient_heterogenuity.resize(_background.size());
		fg_gradient_heterogenuity.resize(channels);
		fg_tex_intencity.resize(channels);
		textural_correlation.resize(channels);
		sterm_x.resize(channels);
		sterm_y.resize(channels);
		sobel_x.resize(channels);
		sobel_y.resize(channels);
		for(size_t c=0;c<channels;c++){
			cv::gpu::ensureSizeIsEnough(graph_size,CV_32FC1,_bg_tex_intencity[c]);
			cv::gpu::ensureSizeIsEnough(graph_size,CV_32FC1,_bg_gradient_heterogenuity[c]);
			cv::gpu::ensureSizeIsEnough(graph_size,CV_32FC1,textural_correlation[c]);

			cv::gpu::ensureSizeIsEnough(graph_size.width,graph_size.height,CV_32FC1,sterm_x[c]);
			cv::gpu::ensureSizeIsEnough(graph_size,CV_32FC1,sterm_y[c]);
			cv::gpu::ensureSizeIsEnough(graph_size,CV_32FC1,fg_gradient_heterogenuity[c]);
			cv::gpu::ensureSizeIsEnough(graph_size,CV_32FC1,fg_tex_intencity[c]);

		}


}

void _imwrite(const std::string& filename, const cv::Mat& mat){
	std::ofstream fout;
	fout.open(filename.c_str());
	if(!fout) return;
	fout << mat;
	fout.close();
}

bool gpu::TexCut::compute(const cv::gpu::GpuMat& _src, cv::gpu::GpuMat& dest,cv::gpu::Stream& stream_external){
	double st, ed;
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	skl::StopWatch swatch;
#endif
	st = cv::getTickCount();
	cv::gpu::split(_src,splitmats,stream_external);
	ed = cv::getTickCount();
	t_compute[0] += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "split: " << swatch.lap() << std::endl;
#endif

	size_t channels = _src.channels();
	assert(_background.size()==channels);
	assert(_background[0].size()==_src.size());

	if (blur_temp.size() < channels) {
		blur_temp.resize(channels);
	}

	// start calculations which do not use sobel edges
	st = cv::getTickCount();
	for(size_t c = 0; c < channels; c++){
		if(_doSmoothing){
			cv::gpu::blur(splitmats[c],blur_temp[c],cv::Size(3,3),cv::Point(-1,-1),stream_external);
		}
		else{
			blur_temp[c] = splitmats[c];
		}
//#ifdef __linux__ 
		cv::gpu::Sobel(blur_temp[c], sobel_x[c], CV_32S, 1, 0, buf_sobel_x, 3, 1.0, cv::BORDER_DEFAULT,-1,stream_external);
		cv::gpu::Sobel(blur_temp[c], sobel_y[c], CV_32S, 0, 1, buf_sobel_y, 3, 1.0, cv::BORDER_DEFAULT,-1,stream_external);
//#else
//		cv::gpu::Sobel(blur_temp[c], sobel_x[c], CV_32S, 1, 0, 3,1.0,cv::BORDER_DEFAULT,-1,stream_data_terms);
//		cv::gpu::Sobel(blur_temp[c], sobel_y[c], CV_32S, 0, 1, 3,1.0,cv::BORDER_DEFAULT,-1,stream_data_terms);
//#endif
	}	
	stream_external.enqueueMemSet(fg_is_over_exposure,cv::Scalar(255));
	stream_external.enqueueMemSet(fg_is_under_exposure,cv::Scalar(255));

	for(size_t c = 0; c < channels; c++){
		stream_external.enqueueMemSet(sterm_x[c],cv::Scalar(1.f));
		stream_external.enqueueMemSet(sterm_y[c],cv::Scalar(1.f));
	}

	ed = cv::getTickCount();
	t_compute[1] += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "do sobel: " << swatch.lap() << std::endl;
#endif

	st = cv::getTickCount();

	ed = cv::getTickCount();
	t_compute[2] += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "wait for upload exposures: " << swatch.lap() << std::endl;
#endif
	st = cv::getTickCount();
	for(size_t c = 0; c < channels; c++){
		checkOverExposure_gpu(
				blur_temp[c],
				fg_is_over_exposure,
				over_exposure_thresh,
				cv::gpu::StreamAccessor::getStream(stream_external));
		checkUnderExposure_gpu(
				blur_temp[c],
				fg_is_under_exposure,
				under_exposure_thresh,
				cv::gpu::StreamAccessor::getStream(stream_external));
	}

	stream_external.enqueueMemSet(max_intencity,cv::Scalar(0));
	stream_external.enqueueMemSet(max_gradient_heterogenuity,cv::Scalar(0));
	stream_setBackground.waitForCompletion();
	for(size_t c = 0; c < channels; c++){
		calcDataTerm_gpu(
				sobel_x[c], sobel_y[c],
				_bg_sobel_x[c], _bg_sobel_y[c],
				fg_tex_intencity[c],
				textural_correlation[c],
				fg_gradient_heterogenuity[c],
				gh_expectation[c], gh_std_dev[c],
				cv::gpu::StreamAccessor::getStream(stream_external));
	}

	// bindup exposure status
	cv::gpu::bitwise_or(fg_is_over_exposure,bg_is_over_exposure,fg_is_over_exposure,cv::gpu::GpuMat(),stream_external);
	cv::gpu::bitwise_or(fg_is_under_exposure,bg_is_under_exposure,fg_is_under_exposure,cv::gpu::GpuMat(),stream_external);
	cv::gpu::bitwise_or(fg_is_over_exposure,fg_is_under_exposure,fg_is_over_exposure,cv::gpu::GpuMat(),stream_external);

	ed = cv::getTickCount();
	t_compute[3] += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "calc DataTerms: " << swatch.lap() << std::endl;
#endif
/*	cv::namedWindow("hoge",0);
	cv::imshow("hoge",cv::Mat(fg_gradient_heterogenuity[0]));
	cv::waitKey(-1);
*/
	// start smoothing term calculation
	st = cv::getTickCount();
	for(size_t c = 0; c < channels; c++){
		calcSmoothingTermX_gpu(
				blur_temp[c], _background[c],
				sterm_x[c],
				noise_std_dev[c],
				cv::gpu::StreamAccessor::getStream(stream_external));
		calcSmoothingTermY_gpu(
				blur_temp[c], _background[c],
				sterm_y[c],
				noise_std_dev[c],
				cv::gpu::StreamAccessor::getStream(stream_external));
	}
	ed = cv::getTickCount();
	t_compute[4] += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "bindup exposures, start sterm calc: " << swatch.lap() << std::endl;
#endif


	st = cv::getTickCount();
	for(size_t c = 0; c < channels; c++){
		bindUpDataTerm_gpu(
				max_intencity,
				max_gradient_heterogenuity,
				terminals,
				fg_tex_intencity[c],
				_bg_tex_intencity[c],
				fg_gradient_heterogenuity[c],
				_bg_gradient_heterogenuity[c],
				textural_correlation[c],
				noise_std_dev[c],
				thresh_tex_diff,
				fg_is_over_exposure,
				cv::gpu::StreamAccessor::getStream(stream_external));
	}
	ed = cv::getTickCount();
	t_compute[5] += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "bindup data terms: " << swatch.lap() << std::endl;
#endif
#ifdef DEBUG_GPU_TEXCUT
	cv::Mat __terminals(terminals);
	cv::namedWindow("terminals",0);
	__terminals += GRAPHCUT_QUANTIZATION_LEVEL/2;
	cv::imshow("terminals",__terminals);
	cv::Mat __max_intencity(max_intencity);
	cv::namedWindow("tex_intencity",0);
	cv::imshow("tex_intencity",__max_intencity);
	cv::Mat __gradient_heterogenuity(max_gradient_heterogenuity);
	cv::namedWindow("max grad_hetero",0);
	cv::imshow("max grad_hetero",__gradient_heterogenuity);
#endif

	st = cv::getTickCount();
	for(size_t c=1;c<channels;c++){
		cv::gpu::min(sterm_x[0],sterm_x[c],sterm_x[0],
			stream_external);
		cv::gpu::min(sterm_y[0],sterm_y[c],sterm_y[0],
			stream_external);
	}
	ed = cv::getTickCount();
	t_compute[6] += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "bindup smoothing_terms: " << swatch.lap() << std::endl;
#endif

#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "wait for bindup the last channel data term: " << swatch.lap() << std::endl;
#endif
	st =  cv::getTickCount();
	bindUpSmoothingTerms_gpu(
			terminals,
			max_gradient_heterogenuity,
			sterm_x[0],
			sterm_y[0],
			rightTransp,
			leftTransp,
			bottom,
			top,
			cv::gpu::StreamAccessor::getStream(stream_external));
#ifdef DEBUG_GPU_TEXCUT
	cv::Mat __hCue(rightTransp);
	cv::namedWindow("hCue",0);
	cv::imshow("hCue",__hCue.t());
	cv::Mat __vCue(bottom);
	cv::namedWindow("vCue",0);
	cv::imshow("vCue",__vCue);
#endif

	ed = cv::getTickCount();
	t_compute[7] += (ed - st)/cv::getTickFrequency();
#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "enhance smoothing_term: " << swatch.lap() << std::endl;
#endif

	// do graphcut
	st = cv::getTickCount();
#if 1
#if 0
	_imwrite("/data/terminals.csv",cv::Mat(terminals));
	_imwrite("/data/leftTransp.csv",cv::Mat(leftTransp));
	_imwrite("/data/rightTransp.csv",cv::Mat(rightTransp));
	_imwrite("/data/top.csv",cv::Mat(top));
	_imwrite("/data/bottom.csv",cv::Mat(bottom));
#endif
	cv::gpu::graphcut(terminals,leftTransp,rightTransp,top,bottom,dest,buf_graphcut,stream_external);
#else
	cv::Mat _dest;
	gc_algo.compute(cv::Mat(terminals),
			cv::Mat(leftTransp),cv::Mat(rightTransp),
			cv::Mat(top),cv::Mat(bottom),_dest);
	dest.upload(_dest);
#endif
	ed = cv::getTickCount();
	t_compute[8] += (ed - st)/cv::getTickFrequency();

#ifdef DEBUG_GPU_TEXCUT_STOPWATCH
	std::cerr << "graphcut               " << swatch.lap() << std::endl;
	std::cerr << "All elasped time: " << swatch.elapsedTime() << std::endl;
#endif
	return true;
}

void gpu::TexCut::learnImageNoiseModel(const cv::gpu::GpuMat& bg2){
	std::vector<cv::gpu::GpuMat> background2;
	cv::gpu::split(bg2,background2);

	size_t channels = _background.size();
	assert(channels==background2.size());
	if (blur_temp.size() < channels) {
		blur_temp.resize(channels);
	}

	assert(CV_8U == bg2.depth());

	noise_std_dev.assign(channels,0);
	for(size_t c=0;c<channels;c++){
		cv::gpu::GpuMat diff,__bg2;
		if(_doSmoothing){
			cv::gpu::blur(background2[c],blur_temp[c],cv::Size(3,3));
		}
		else{
			blur_temp[c] = background2[c];
		}
		_background[c].convertTo(diff,CV_32FC1);
		blur_temp[c].convertTo(__bg2,CV_32FC1);
		cv::gpu::subtract(diff,__bg2,diff);

		cv::Scalar mean,stddev;
		skl::gpu::meanStdDev(diff,mean,stddev);

		noise_std_dev[c] = (float)stddev[0];
		if(stddev[0]==0.f){
			noise_std_dev[c] = 5.f;
		}
		assert(noise_std_dev[c]>0);
	}

	gh_expectation.assign(channels,0);
	gh_std_dev.assign(channels,0);
	cv::parallel_for(
		cv::BlockedRange(0,(int)channels),
		ParallelGHNoiseEstimate(
		noise_std_dev,
		&gh_expectation,
		&gh_std_dev
		)
		);
#ifdef DEBUG_GPU_TEXCUT
	std::cerr << "Noise SD: ";
	for(size_t i=0;i<channels;i++){
		if(i!=0) std::cerr << ",";
		std::cerr << noise_std_dev[i];
	}
	std::cerr << std::endl;

	std::cerr << "GH Expec: ";
	for(size_t i=0;i<channels;i++){
		if(i!=0) std::cerr << ",";
		std::cerr << gh_expectation[i];
	}
	std::cerr << std::endl;

	std::cerr << "GH SD   : ";
	for(size_t i=0;i<channels;i++){
		if(i!=0) std::cerr << ",";
		std::cerr << gh_std_dev[i];
	}
	std::cerr << std::endl;
#endif //DEBUG
	for(size_t c = 0; c < _background.size(); c++){
		calcBGDataTerm_gpu(
			_bg_sobel_x[c],_bg_sobel_y[c],
			_bg_tex_intencity[c],
			_bg_gradient_heterogenuity[c],
			gh_expectation[c],
			gh_std_dev[c]);
	}
}

void gpu::TexCut::setNoiseModel(
	const std::vector<float>& noise_std_dev,
	const std::vector<float>& gh_expectation,
	const std::vector<float>& gh_std_dev){
		this->noise_std_dev = noise_std_dev;
		this->gh_expectation = gh_expectation;
		this->gh_std_dev = gh_std_dev;

		if(_background.empty()){
			return;
		}

		for(size_t c = 0; c < _background.size(); c++){
			calcBGDataTerm_gpu(
				_bg_sobel_x[c],_bg_sobel_y[c],
				_bg_tex_intencity[c],
				_bg_gradient_heterogenuity[c],
				gh_expectation[c],
				gh_std_dev[c]);
		}
}
