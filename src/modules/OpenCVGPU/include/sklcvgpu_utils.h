/*!
 * @file sklcvgpu_utils.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/25
 * @date Last Change:2012/Feb/20.
 */
#ifndef __SKL_CV_GPU_UTILS_H__
#define __SKL_CV_GPU_UTILS_H__

#ifdef _DEBUG
#define DEBUG_CV_GPU_UTILS
#endif

#include <cv.h>
#include <highgui.h>
#include <opencv2/gpu/gpu.hpp>
namespace skl{
	namespace gpu{

		void meanStdDev(const cv::gpu::GpuMat& mtx,cv::Scalar& mean, cv::Scalar& stddev);

		inline void copyMatToPageLockedCudaMem(const cv::Mat& mat, cv::gpu::CudaMem& cudaMem, cv::Mat& memMat) {
			if (mat.size() != cudaMem.size() ||
				mat.type() != cudaMem.type()) {
					cudaMem.create(mat.size(), mat.type(), cv::gpu::CudaMem::ALLOC_PAGE_LOCKED);
			}
			memMat = cudaMem;
			mat.copyTo(memMat);
		}
	} // namespace gpu
} // namespace skl

#endif // __SKL_CV_GPU_UTILS_H__
