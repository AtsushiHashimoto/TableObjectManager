/*!
 * @file shared.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/25
 * @date Last Change:2014/Mar/04.
 */
#ifndef __SKL_GPU_SHARED_H__
#define __SKL_GPU_SHARED_H__

#include <climits>

#define TEXCUT_BLOCK_SIZE 4
#define TEXCUT_SQUARE_AREA 16 // TEXCUT_BLOCK_SIZE * TEXCUT_BLOCK_SIZE
#define TEXCUT_SQUARE_AREA_HARF 8 // TEXCUT_BLOCK_SIZE * TEXCUT_BLOCK_SIZE / 2
#define TEXCUT_SQRT_BLOCK_SIZE 2 // sqrt(TEXCUT_BLOCK_SIZE)
#define GRAPHCUT_QUANTIZATION_LEVEL SHRT_MAX // 32767.0f
#define SQRT3 1.7320508075688772935275f // sqrt(3)

#include <iostream>
#include "cuda_runtime.h"
#define cudaSafeCall(expr) __cudaSafeCall(expr,__FILE__,__LINE__)

#define divUp(val,grain) ( (val)+(grain)-1 )/(grain)

namespace skl{
	namespace gpu{
		dim3 maxBlockSize(int* shared_mem_size, float byte_per_thread,float byte_per_row = 0.f,float byte_per_col = 0.f, int byte_const=0, bool near_square = true,int dev=0);

		static inline void __cudaSafeCall(cudaError_t err, const char* file, const int line){
			if(cudaSuccess != err){
				std::cerr << "Cuda error in file '" << file << "' in line " << line << std::endl;
				std::cerr << cudaGetErrorString(err) << std::endl;
			}
		}

		void setAlpha(float alpha);
		void setSmoothingTermWeight(float smoothing_term_weight);

	}
}

#endif // __SKL_GPU_SHARED_H__
