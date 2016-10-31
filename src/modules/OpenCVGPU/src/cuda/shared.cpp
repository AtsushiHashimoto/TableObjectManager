/*!
 * @file shared.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/25
 * @date Last Change:2012/Feb/20.
 */

#include <cmath>
#include <cassert>
#include "shared.h"

#ifdef _WIN32
#define log2f(x) (logf(x)*1.4426950408889634f)
#endif
using namespace skl;
using namespace skl::gpu;

dim3 skl::gpu::maxBlockSize(int* shared_mem_size, float byte_per_thread,float byte_per_row,float byte_per_col, int byte_const, bool near_square,int dev){
	cudaDeviceProp deviceProp;
	cudaGetDeviceProperties(&deviceProp, dev);

	int block[2];
	if(near_square){
		float max_block_area_log2 = log2f(deviceProp.maxThreadsPerBlock);
		assert(ceil(max_block_area_log2)==floor(max_block_area_log2));
		block[0] = std::pow(2.f,static_cast<int>(max_block_area_log2+1)/2);
		block[1] = std::pow(2.f,static_cast<int>(max_block_area_log2)/2);
	}
	else{
		block[0] = deviceProp.maxThreadsPerBlock;
		block[1] = 1;
	}

	int byte_per_block = deviceProp.sharedMemPerBlock;
	// which_dim = 1 if block size is square, otherwise 0.
	// The case block[0]!=block[1], block[which_dim(=0)] is always longer than block[1];
	bool which_dim = block[0]==block[1];
	while(1){
		*shared_mem_size = ceil(byte_per_thread * (block[0]*block[1])) + ceil(byte_per_row * block[1]) + ceil(byte_per_col * block[0]) + byte_const;
		if(*shared_mem_size < byte_per_block) break;
		block[which_dim] /= 2;
		if(near_square) which_dim = !which_dim;
		assert(block[0]>0 && block[1]>0);
	}
#ifdef _DEBUG
	std::cerr << "Block Size: " << block[0] << ", " << block[1] << std::endl;
#endif
	return dim3(block[0],block[1]);
}

