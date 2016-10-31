/*!
 * @file ParticleFilter.h
 * @author a_hasimoto
 * @date Date Created: 2012/Nov/21
 * @date Last Change:2012/Nov/21.
 */
#ifndef __SKL_PARTICLE_FILTER_H__
#define __SKL_PARTICLE_FILTER_H__

#include "sklcv.h"
#include <opencv2/legacy/legacy.hpp>
namespace skl{

/*!
 * @class ParticleFilter
 * @brief Interface for Particle Filter with SIR (or ConDenSation) algorithm.
 */
template <class Observation, class LikelihoodCalcFunctor> class ParticleFilter{
	public:
		// 状態/観測結果が一次元のfloat*の列になるように工夫する必要がある
		class State{
			public:
				State(size_t dim):
					data(new float[dim]),
					_dim(dim){}
				State(size_t dim, float* data){
					_dim = dim;
					this->data = cv::Ptr<float>(data);
					(*this->data.refcount)++;
				}
				State(const State& other){
					_dim = other.dim();
					data = other.data;
				}
				inline State clone()const{
					State copy(dim());
					memcpy(copy.data,data,dim()*sizeof(float));
					return copy;
				}
				~State(){
				}
				cv::Ptr<float> data;
				inline size_t dim()const{return _dim;}
			protected:
				size_t _dim;
		};

	public:
		ParticleFilter(size_t sample_num=5000);
		ParticleFilter(size_t sample_num,cv::Ptr<LikelihoodCalcFunctor> functor);
		virtual ~ParticleFilter();
		void compute(const Observation& observation);
		inline const State state(size_t n)const{
			assert(n<_sample_num);
			return State(cond->DP,cond->flSamples[n]);
		}
		inline float confidence(size_t n)const{
			assert(n<_sample_num);
			return cond->flConfidence[n];
		}

		void initialize(
				const State& lowerBound,
				const State& upperBound,
				const cv::Mat& DynamMat=cv::Mat());

		void setRandStateParam(size_t d,float max,float min,long seed, int dist_type=CV_RAND_NORMAL);// (dist_type can be CV_RAND_UNI,too)
		inline size_t sample_num()const{return _sample_num;}
		inline const cv::Ptr<LikelihoodCalcFunctor>& functor()const{return _functor;}
		inline void functor(const cv::Ptr<LikelihoodCalcFunctor>& __functor){_functor = __functor;}
	protected:
		size_t _sample_num;
		cv::Ptr<LikelihoodCalcFunctor> _functor;

	private:
		cv::Ptr<CvConDensation> cond;
};



/*!
 * @brief デフォルトコンストラクタ
 */
template<class Observation,class LikelihoodCalcFunctor> 
ParticleFilter<Observation,LikelihoodCalcFunctor>::ParticleFilter(size_t __sample_num):
	_sample_num(__sample_num)
{
}

/*!
 * @brief コンストラクタ
 */
template<class Observation,class LikelihoodCalcFunctor> 
ParticleFilter<Observation,LikelihoodCalcFunctor>::ParticleFilter(size_t __sample_num,cv::Ptr<LikelihoodCalcFunctor> __functor):
	_sample_num(__sample_num),
	_functor(__functor)
{
}


/*!
 * @brief デストラクタ
 */
template<class Observation,class LikelihoodCalcFunctor> 
ParticleFilter<Observation,LikelihoodCalcFunctor>::~ParticleFilter()
{
}





template<class Observation,class LikelihoodCalcFunctor> 
void ParticleFilter<Observation,LikelihoodCalcFunctor>::initialize(
		const State& __lowerBound,
		const State& __upperBound,
		const cv::Mat& __DynamMat){
	size_t state_dim = __lowerBound.dim();
	assert(__upperBound.dim()==state_dim);

	cond = cvCreateConDensation(state_dim, 0,_sample_num);

	cv::Mat lowerBound(cv::Size(1,state_dim),CV_32FC1);
	memcpy(lowerBound.ptr<float>(0),__lowerBound.data,state_dim*sizeof(float));
	cv::Mat upperBound(cv::Size(1,state_dim),CV_32FC1);
	memcpy(upperBound.ptr<float>(0),__upperBound.data,state_dim*sizeof(float));
	

	cv::Mat DynamMat(__DynamMat);
	if(DynamMat.empty()){
		DynamMat = cv::Mat::eye(cv::Size(state_dim,state_dim),CV_32FC1);
	}
	else{
		assert(skl::checkMat(DynamMat,CV_32F,1,cv::Size(state_dim,state_dim)));
	}
	CvMat lowerBound_ = lowerBound;
	CvMat upperBound_ = upperBound;
	cvConDensInitSampleSet(cond,
			&lowerBound_,
			&upperBound_);
	
	// set DynamMat
	int step_width = DynamMat.cols;
	for(int y=0;y<DynamMat.rows;y++){
		memcpy(
				cond->DynamMatr + y * step_width,
				DynamMat.ptr<float>(y),
				sizeof(float) * step_width);
	}
}



template<class Observation,class LikelihoodCalcFunctor> 
void ParticleFilter<Observation,LikelihoodCalcFunctor>::setRandStateParam(
		size_t d,
		float max,
		float min,
		long seed,
		int dist_type){
	assert(d<(size_t)cond->DP);
	cond->RandS[d].disttype = dist_type;
	cvRandInit(&(cond->RandS[d]),min,max,seed);
}


template<class Observation,class LikelihoodCalcFunctor> 
void ParticleFilter<Observation,LikelihoodCalcFunctor>::compute(
		const Observation& observation){
	// P(X_t|X_{t-1}) calculation
	cvConDensUpdateByTime(cond);

	// confidence (P(Z|X)) calculation
	 _functor->set(observation,cond->flSamples,cond->flConfidence);
	cv::parallel_for(
			cv::BlockedRange(0,_sample_num),
			*_functor);
}


} // skl

#endif // __SKL_PARTICLE_FILTER_INTERFACE_H__

