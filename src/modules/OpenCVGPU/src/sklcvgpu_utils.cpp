/*!
 * @file sklcvgpu_utils.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/25
 * @date Last Change:2012/Feb/17.
 */
#include "sklcvgpu_utils.h"

namespace skl{
	namespace gpu{

		void meanStdDev(const cv::gpu::GpuMat& mtx,cv::Scalar& mean, cv::Scalar& stddev){
			assert(1==mtx.channels());

			size_t elem_num = mtx.rows * mtx.cols;
			cv::gpu::GpuMat buf;
			mean = cv::gpu::sum( mtx, buf);
			stddev = cv::gpu::sqrSum( mtx, buf);
			for(int i=0;i<mean.cols;i++){
				mean[i] /= elem_num;
			}
			stddev = cv::gpu::sqrSum( mtx, buf);
			for(int i=0;i<stddev.cols;i++){
				stddev[i] /= elem_num;
				stddev[i] = std::sqrt(stddev[i] - std::pow(mean[i],2));
			}
		}
	}
}
