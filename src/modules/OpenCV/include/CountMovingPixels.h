/*!
 * @file CountMovingPixel.h
 * @author a_hasimoto
 * @date Date Created: 2013/Nov/21
 * @date Last Change:2014/Jan/10.
 */
#ifndef __SKL_COUNT_MOVING_PIXELS_H__
#define __SKL_COUNT_MOVING_PIXELS_H__

#include "skl.h"
#include "sklcv.h"

namespace skl{

/*!
 * @class 高速に2フレーム間で動きがあるかどうかをチェックする
 */
 class CountMovingPixels{

	public:
		CountMovingPixels(float thresh,int step, int life=2,bool shallow_copy=false);
		virtual ~CountMovingPixels();
		virtual float compute(const cv::Mat& img);

		/* accessor */
		inline float thresh()const{return _thresh;}
		inline void thresh(float __thresh){_thresh = __thresh;}
		inline int step()const{return _step;}
		inline void step(int __step){_step = __step;}
		inline bool shallow_copy()const{return _shallow_copy;}
		inline void shallow_copy(bool __shallow_copy){_shallow_copy = __shallow_copy;}
	protected:
		float _thresh;
		int _step;
		bool _shallow_copy;
		std::vector<cv::Mat> prevs;
		size_t phase;
	private:
};

} // skl

#endif // __SKL_COUNT_MOVING_PIXELS_H__

