/*!
 * @file ShortMemoryMoments.h
 * @author a_hasimoto
 * @date Date Created: 2014/Jan/21
 * @date Last Change:2014/Feb/11.
 */
#ifndef __SKL_SHORT_MEMORY_MOMENTS_H__
#define __SKL_SHORT_MEMORY_MOMENTS_H__
#include <vector>
#include <list>
#include <climits>

namespace skl{

/*!
 * @class ShortMemoryMoments
 * @brief 一定区間の長さのn次モーメントを計算・保存するクラス
 */
template<
	typename Sample=double,
	unsigned int N=2>
class ShortMemoryMoments{

	public:
		ShortMemoryMoments(size_t memory_length=UINT_MAX);
		virtual ~ShortMemoryMoments();

		inline void clear(){
			_short_memory.clear();
			_moments.assign(N,0);
		}

		ShortMemoryMoments<Sample,N>& operator=(const ShortMemoryMoments<Sample,N>& other){
			_short_memory = other._short_memory;
			_moments = other._moments;
			_memory_length = other._memory_length;
			return *this;
		}

		void operator()(const Sample& sample);
		template<typename Iter> inline void operator()(Iter begin, Iter end){
			for(Iter it=begin;it!=end;it++)(*this)(*it);
		}
		inline void push(const Sample& sample){(*this)(sample);}

		inline Sample operator[](size_t i)const{
			if(i==0)return 1;
			assert(i<=N);
			return _moments[i-1]/_short_memory.size();
		}

		Sample sum()const{
			assert(N>=1);
			return _moments[0];
		}
		Sample mean()const{
			assert(N>=1);
			return (*this)[1];
		}

		Sample variance()const{
			assert(N>=2);
			Sample var = (*this)[2] - std::pow(mean(),2);
			// It sometimes becomes negative because of round-off error
			return var > 0 ? var : 0;
		}

		Sample stddev()const{
			return std::sqrt(variance());
		}

		/*** Accessor ***/
		inline size_t size()const{return _short_memory.size();}
		inline void memory_length(size_t __memory_length){
			_memory_length = __memory_length;
			while(_short_memory.size()>_memory_length) pop();
		}
		size_t memory_length()const{return _memory_length();}
		inline const std::list<Sample>& short_memory()const{
			return _short_memory;
		}
		inline std::list<Sample>& short_memory(){
			return _short_memory;
		}
	protected:
		std::list<Sample> _short_memory;
		std::vector<Sample> _moments;
		size_t _memory_length;

		void pop();
	private:
};

template<typename Sample, unsigned int N>
ShortMemoryMoments<Sample,N>::ShortMemoryMoments(size_t memory_length):_moments(N,0),_memory_length(memory_length){
	assert(_memory_length>0);
}

template<typename Sample, unsigned int N>
ShortMemoryMoments<Sample,N>::~ShortMemoryMoments(){
}

template<typename Sample, unsigned int N>
void ShortMemoryMoments<Sample,N>::operator()(const Sample& sample){
	_short_memory.push_back(sample);
	Sample temp = sample;
	_moments[0]+=temp;
	for(size_t n=1;n<N;n++){
		temp*=sample;
		_moments[n]+=temp;
	}
	if(_short_memory.size()>_memory_length){
		pop();
	}
}

template<typename Sample, unsigned int N>
void ShortMemoryMoments<Sample,N>::pop(){
	Sample temp = _short_memory.front();
	_moments[0]-=temp;
	for(size_t n=1;n<N;n++){
		temp*=_short_memory.front();
		_moments[n]-=temp;
	}
	_short_memory.pop_front();
}

} // skl

#endif // __SKL_SHORT_MEMORY_ACCUMULATOR_H__

