/*!
 * @file FilterGpuMat2GpuMat.h
 * @author a_hasimoto
 * @date Date Created: 2012/Feb/10
 * @date Last Change:2012/Jul/31.
 */
#ifndef __SKL_GPU_FILTER_GPU_MAT2_GPU_MAT_H__
#define __SKL_GPU_FILTER_GPU_MAT2_GPU_MAT_H__
#include "skl.h"
#include <opencv2/gpu/gpu.hpp>

namespace skl{
	namespace gpu{

		/*!
		 * @brief cv::gpu::GpuMatを引数としてとるFilter
		 */
		template<class RET=bool> class FilterGpuMat2GpuMat: public _Filter<RET,cv::gpu::GpuMat,cv::gpu::GpuMat>{
			public:
				FilterGpuMat2GpuMat();
				virtual RET compute(const cv::gpu::GpuMat& src,cv::gpu::GpuMat& dest);
				virtual RET compute(const cv::gpu::GpuMat& src,cv::gpu::GpuMat& dest,cv::gpu::Stream& s)=0;
				virtual RET compute(const cv::Mat& src,cv::Mat& dest,cv::gpu::Stream& s = cv::gpu::Stream::Null());
				virtual RET compute(const cv::Mat& src,cv::gpu::GpuMat& dest,cv::gpu::Stream& s = cv::gpu::Stream::Null());
				virtual RET compute(const cv::gpu::GpuMat& src,cv::Mat& dest,cv::gpu::Stream& s = cv::gpu::Stream::Null());
				virtual ~FilterGpuMat2GpuMat();
			protected:
				cv::gpu::GpuMat src_buf;
				cv::gpu::GpuMat dest_buf;
		};
		template<class RET> FilterGpuMat2GpuMat<RET>::FilterGpuMat2GpuMat(){}
		template<class RET> FilterGpuMat2GpuMat<RET>::~FilterGpuMat2GpuMat(){}

		template<class RET> RET FilterGpuMat2GpuMat<RET>::compute(const cv::gpu::GpuMat& src,cv::gpu::GpuMat& dest){
			return compute(src,dest,cv::gpu::Stream::Null());
		}

		template<class RET> RET FilterGpuMat2GpuMat<RET>::compute(const cv::Mat& src,cv::Mat& dest,cv::gpu::Stream& s){
			cv::gpu::ensureSizeIsEnough(src.size(),src.type(),src_buf);
			src_buf.upload(src);
			RET val = compute(src_buf,dest_buf,s);
			dest_buf.download(dest);
			return val;
		}

		template<class RET> RET FilterGpuMat2GpuMat<RET>::compute(const cv::Mat& src,cv::gpu::GpuMat& dest,cv::gpu::Stream& s){
			cv::gpu::ensureSizeIsEnough(src.size(),src.type(),src_buf);
			src_buf.upload(src);
			return compute(src_buf,dest,s);
		}
		template<class RET> RET FilterGpuMat2GpuMat<RET>::compute(const cv::gpu::GpuMat& src,cv::Mat& dest,cv::gpu::Stream& s){
			RET val = compute(src,dest_buf,s);
			dest_buf.download(dest);
			return val;
		}

	} // skl
} // gpu
#endif // __SKL_GPU_FILTER_GPU_MAT2_GPU_MAT_H__

