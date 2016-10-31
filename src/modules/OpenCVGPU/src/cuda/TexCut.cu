/*!
 * @file TexCut.cu
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/25
 * @date Last Change: 2014/Mar/04.
 */

#include <cassert>
#include <cfloat>
#include "opencv2/gpu/devmem2d.hpp"
#include "cuda_runtime.h"
#include "npp.h"
#include "shared.h"
#include "shared_funcs.cu"

#define ENOUGH_LARGE_NUM 1000

namespace cv{
	namespace gpu{
#if CV_MAJOR_VERSION > 1
#if CV_MINOR_VERSION < 4
	typedef PtrStep PtrStepb;
#endif
#endif
/*#elif WIN32
	typedef PtrStep PtrStepb;
#endif
	*/
	}
}

// functions using kernel functions
namespace skl{
	namespace gpu{

#define SquareNum (blockDim.x * blockDim.y) / TEXCUT_SQUARE_AREA;
#define SquareIdx dim3(\
		( _GIdx(x) ) / TEXCUT_BLOCK_SIZE,\
		( _GIdx(y) ) / TEXCUT_BLOCK_SIZE)

#define SquareSeqIdxInBlock _SeqIdx(\
		(threadIdx.x / TEXCUT_BLOCK_SIZE), (threadIdx.y / TEXCUT_BLOCK_SIZE), (blockDim.x / TEXCUT_BLOCK_SIZE) )

#define inSquareSeqIdx _SeqIdx( (threadIdx.x%TEXCUT_BLOCK_SIZE), (threadIdx.y%TEXCUT_BLOCK_SIZE), TEXCUT_BLOCK_SIZE)

		__device__ __constant__ float alpha;
		__device__ __constant__ float smoothing_term_weight;
		extern __shared__ unsigned char smem[];
		

		__host__ void setAlpha(float _alpha){
			cudaSafeCall(cudaMemcpyToSymbol(alpha,&_alpha,sizeof(float)));
		}
		__host__ void setSmoothingTermWeight(float _weight){
			cudaSafeCall(cudaMemcpyToSymbol(smoothing_term_weight,&_weight,sizeof(float)));
		}

		inline __device__ float normalize(float val, float std_dev, float mean){
			val = (val - mean - std_dev)/(2.f*alpha*std_dev);
			if(isnan(val)) return 0.f;
			return min(max(val,0.f),1.f);
		}

		__global__ void calcBGDataTerm_kernel(
				const cv::gpu::PtrStepi sobel_x,
				const cv::gpu::PtrStepi sobel_y,
				cv::gpu::PtrStepf tex_intencity,
				cv::gpu::PtrStepf gradient_heterogenuity,
				float gh_expectation,
				float gh_std_dev,
				int cols,
				int rows){
			dim3 gidx = GlobalIdx;
			if( gidx.x >= cols || gidx.y >= rows) return;

			int smem_step = (TEXCUT_SQUARE_AREA+1) * SquareNum;

			// declare shared_memory
			int* array = (int*)smem;
			int* order = array + smem_step;
			dim3 graph_node_idx = SquareIdx;

			// odd-even transposition sort ( x and y direction elements at once
			int in_square_seq_idx = inSquareSeqIdx;
			int square_seq_idx_in_block = SquareSeqIdxInBlock;
			// offset to direct shared memory position
			// for the belonging square
			int offset = (TEXCUT_SQUARE_AREA+1) * square_seq_idx_in_block;

			// copy data to shared_memory
			int dx = abs(sobel_x.ptr(gidx.y)[gidx.x]);
			int dy = abs(sobel_y.ptr(gidx.y)[gidx.x]);
			array[offset + in_square_seq_idx] = dx*dx + dy*dy;
			__syncthreads();

			sorti(array+offset,TEXCUT_SQUARE_AREA,in_square_seq_idx,order+offset);
			float* max = (float*)(array + offset + TEXCUT_SQUARE_AREA);
			float* min = (float*)(order + offset + TEXCUT_SQUARE_AREA);
			if(order[offset+in_square_seq_idx]==0){
				*min = (float)array[offset+in_square_seq_idx];
			}
			__syncthreads();
			if(order[offset+in_square_seq_idx]==TEXCUT_SQUARE_AREA-1){
				*max = (float)array[offset+in_square_seq_idx];
				*max = (*min == 0.f) ? (*max==0.f?0.f:ENOUGH_LARGE_NUM) : sqrt(*max / *min);
				gradient_heterogenuity.ptr(graph_node_idx.y)[graph_node_idx.x] = normalize(*max,gh_std_dev*2.f,gh_expectation);
			}

			// calc intencity
			sumUpi(array+offset,
					TEXCUT_SQUARE_AREA,
					in_square_seq_idx);
			if(in_square_seq_idx==0){
				tex_intencity.ptr(graph_node_idx.y)[graph_node_idx.x] = (float)array[offset];
			}
		

		}

		void calcBGDataTerm_gpu(
				const cv::gpu::DevMem2Di sobel_x,
				const cv::gpu::DevMem2Di sobel_y,
				cv::gpu::DevMem2Df tex_intencity,
				cv::gpu::DevMem2Df gradient_heterogenuity,
				float gh_expectation,
				float gh_std_dev,
				cudaStream_t stream,
				int dev){
			int sharedMemSize;
			dim3 block = maxBlockSize(&sharedMemSize,(2.f+2.f/TEXCUT_SQUARE_AREA) * sizeof(int),0,0,1,true,dev);
			dim3 grid(divUp(sobel_x.cols,block.x), divUp(sobel_x.rows,block.y));
//			std::cerr << "GradHetero: " << sharedMemSize << std::endl;
//			std::cerr << block.x << ", " << block.y << ", " << block.z << std::endl;

			calcBGDataTerm_kernel<<<grid,block,sharedMemSize,stream>>>(
					sobel_x,sobel_y,
					tex_intencity,
					gradient_heterogenuity,
					gh_expectation,gh_std_dev,
					sobel_x.cols,
					sobel_y.rows);
			cudaSafeCall( cudaGetLastError() );
		}

		__global__ void calcDataTerm_kernel(
				const cv::gpu::PtrStepi sobel_x,
				const cv::gpu::PtrStepi sobel_y,
				const cv::gpu::PtrStepi bg_sobel_x,
				const cv::gpu::PtrStepi bg_sobel_y,
				cv::gpu::PtrStepf fg_tex_intencity,
				cv::gpu::PtrStepf textural_correlation,
				cv::gpu::PtrStepf gradient_heterogenuity,
				float gh_expectation,
				float gh_std_dev,
				int cols,
				int rows){
			int in_square_seq_idx = inSquareSeqIdx;
			dim3 gidx = GlobalIdx;
			if(gidx.x >= cols || gidx.y >= rows) return;

			// copy data to shared_memory
			int dx = abs(sobel_x.ptr(gidx.y)[gidx.x]);
			int dy = abs(sobel_y.ptr(gidx.y)[gidx.x]);

			// declare shared_memory
			int* buf = (int*)smem;

			int square_seq_idx_in_block = SquareSeqIdxInBlock;
			// offset to direct shared memory position
			// for the belonging square
			int offset = (TEXCUT_SQUARE_AREA + 1) * square_seq_idx_in_block;

			// calc correlation
			buf[offset + in_square_seq_idx] = 
				  dx * abs(bg_sobel_x.ptr(gidx.y)[gidx.x])
				+ dy * abs(bg_sobel_y.ptr(gidx.y)[gidx.x]);

			__syncthreads();

			// reuse dx space for sumup function
			sumUpi(buf+offset,
					TEXCUT_SQUARE_AREA,
					in_square_seq_idx);

			dim3 graph_node_idx = SquareIdx;
			if(in_square_seq_idx==0){
				textural_correlation.ptr(graph_node_idx.y)[graph_node_idx.x] = (float)buf[offset];
			}

			buf[offset + in_square_seq_idx] = dx * dx + dy * dy;
			__syncthreads();

			// calc grad. hetero.
			int* order = buf + (TEXCUT_SQUARE_AREA + 1) * SquareNum;

			sorti(buf + offset, TEXCUT_SQUARE_AREA,
					in_square_seq_idx, order + offset);
			float* _max = (float*)(buf + offset + TEXCUT_SQUARE_AREA);
			float* _min = (float*)(order + offset + TEXCUT_SQUARE_AREA);
			if(order[offset + in_square_seq_idx]==0){
				*_min = (float)buf[offset + in_square_seq_idx];
			}
			__syncthreads();

			if(order[offset + in_square_seq_idx]==TEXCUT_SQUARE_AREA-1){
				*_max = (float)buf[offset + in_square_seq_idx];
				*_max = (*_min == 0.f) ? (*_max==0.f?0.f:ENOUGH_LARGE_NUM) : sqrt(*_max / *_min);
				gradient_heterogenuity.ptr(graph_node_idx.y)[graph_node_idx.x] = normalize(*_max,gh_std_dev*2.f,gh_expectation);
			}

			// calc intencity
			sumUpi(buf+offset,
					TEXCUT_SQUARE_AREA,
					in_square_seq_idx);
			if(in_square_seq_idx==0){
				fg_tex_intencity.ptr(graph_node_idx.y)[graph_node_idx.x] = (float)buf[offset];
			}
		}

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
				int dev){
			int sharedMemSize;
			dim3 block = maxBlockSize(&sharedMemSize,(1.f+1.f/TEXCUT_SQUARE_AREA) * sizeof(int) * 2, 0,0,0,true,dev);
			dim3 grid(divUp(sobel_x.cols,block.x), divUp(sobel_x.rows,block.y));
//			std::cerr << "calcTexture: " << sharedMemSize << std::endl;
//			std::cerr << block.x << ", " << block.y << ", " << block.z << std::endl;

			calcDataTerm_kernel<<<grid,block,sharedMemSize,stream>>>(
					sobel_x, sobel_y,
					bg_sobel_x, bg_sobel_y,
					fg_tex_intencity,
					textural_correlation,
					gradient_heterogenuity,
					gh_expectation,
					gh_std_dev,
					sobel_x.cols,
					sobel_y.rows);
			cudaSafeCall( cudaGetLastError() );
		}

		__global__ void calcSmoothingTermX_kernel(
					const cv::gpu::PtrStepb src,
					const cv::gpu::PtrStepb bg,
					cv::gpu::PtrStepf sterm,
					float noise_std_dev,
					int offset,
					int cols,
					int rows){
			dim3 gidx = GlobalIdx;

			dim3 imgIdx(
					min((gidx.x+1),MEM_WIDTH-1) * TEXCUT_BLOCK_SIZE,
					gidx.y * TEXCUT_BLOCK_SIZE);

			int smem_step = (ThreadSize + blockDim.y);

			// calc top
			int* lsrc = (int*)smem;
			int* lbg = lsrc + smem_step * TEXCUT_BLOCK_SIZE;

			int array_idx = _SeqIdx(threadIdx.x+1,threadIdx.y,blockDim.x+1);
			float ldiff=0.f;

			for(int i = 0, si = array_idx; i < TEXCUT_BLOCK_SIZE; i++,si += smem_step){
				lsrc[si] = src.ptr(imgIdx.y+i)[imgIdx.x + offset];
				lbg[si]  =  max(1,bg.ptr(imgIdx.y+i)[imgIdx.x + offset]);
				lsrc[si-1] = src.ptr(imgIdx.y+i)[imgIdx.x-TEXCUT_BLOCK_SIZE + offset];
				lbg[si-1]  =  max(1,bg.ptr(imgIdx.y+i)[imgIdx.x-TEXCUT_BLOCK_SIZE + offset]);
				ldiff += (float)lsrc[si] 
					  -  (float)lbg[si] 
							  * (float)lsrc[si-1]
								  / lbg[si-1];
			}

			if(gidx.x < cols && gidx.y < rows){
				sterm.ptr(gidx.x)[gidx.y] = min(sterm.ptr(gidx.x)[gidx.y],1.f - exp(
						normalize(
							abs(ldiff) / TEXCUT_BLOCK_SIZE,
							noise_std_dev / TEXCUT_SQRT_BLOCK_SIZE,
							0.f
						)
						- 1.f
					));
			}
		}


		void calcSmoothingTermX_gpu(
				const cv::gpu::DevMem2D src,
				const cv::gpu::DevMem2D bg,
				cv::gpu::DevMem2Df sterm,
				float noise_std_dev,
				cudaStream_t stream,
				int dev){
			int sharedMemSize;
			dim3 block = maxBlockSize(&sharedMemSize,2 * sizeof(int)* TEXCUT_BLOCK_SIZE,2 * sizeof(int)* TEXCUT_BLOCK_SIZE,0,0,true,dev);
			// sterm is transposed
			while(sterm.rows%block.x!=0){
				block.x/=2;
			}
			assert(block.x>0);
			while(sterm.cols%block.y!=0){
				block.y/=2;
			}
			assert(block.y>0);
//			std::cerr << "block_size: " << block.x << ", " << block.y << std::endl;
			dim3 grid(divUp(sterm.rows,block.x),divUp(sterm.cols,block.y) );
//			std::cerr << "calcSmoothingTermX: " << sharedMemSize << std::endl;
//			std::cerr << block.x << ", " << block.y << ", " << block.z << std::endl;

			calcSmoothingTermX_kernel<<<grid,block,sharedMemSize,stream>>>(
					src,
					bg,
					sterm,
					noise_std_dev,0,
					sterm.rows, sterm.cols);
			calcSmoothingTermX_kernel<<<grid,block,sharedMemSize,stream>>>(
					src,
					bg,
					sterm,
					noise_std_dev,TEXCUT_BLOCK_SIZE-1,
					sterm.rows, sterm.cols);
			cudaSafeCall( cudaGetLastError() );
		}

		__global__ void calcSmoothingTermY_kernel(
				const cv::gpu::PtrStepb src,
				const cv::gpu::PtrStepb bg,
				cv::gpu::PtrStepf sterm,
				float noise_std_dev,
				int offset,
				int cols,
				int rows){
			dim3 gidx = GlobalIdx;
			dim3 imgIdx(
					gidx.x * TEXCUT_BLOCK_SIZE,
					min((gidx.y+1),MEM_HEIGHT-1) * TEXCUT_BLOCK_SIZE);

			int smem_step = ThreadSize + blockDim.x;

			// calc top
			int* lsrc = (int*)smem;
			int* lbg = lsrc + smem_step * TEXCUT_BLOCK_SIZE;

			int array_idx = _SeqIdx(threadIdx.y+1,threadIdx.x,blockDim.y+1);

			float ldiff=0.f;
			for(int i = 0, si = array_idx; i < TEXCUT_BLOCK_SIZE; i++,si+=smem_step){
				lsrc[si] = src.ptr(imgIdx.y+offset)[imgIdx.x+i];
				lbg[si]  =  max(1,bg.ptr(imgIdx.y+offset)[imgIdx.x+i]);
				lsrc[si-1] = src.ptr(imgIdx.y-TEXCUT_BLOCK_SIZE+offset)[imgIdx.x+i];
				lbg[si-1]  =  max(1,bg.ptr(imgIdx.y-TEXCUT_BLOCK_SIZE+offset)[imgIdx.x+i]);
				ldiff += (float)lsrc[si] 
					  -  (float)(
							  lbg[si] 
							  * ((float)lsrc[si-1]
								  / lbg[si-1]));
			}


			// sterm is NOT transposed
			if(gidx.x < cols && gidx.y < rows){
				sterm.ptr(gidx.y)[gidx.x] = min(sterm.ptr(gidx.y)[gidx.x],1.f - exp(
						normalize(
							abs(ldiff) / TEXCUT_BLOCK_SIZE,
							noise_std_dev / TEXCUT_SQRT_BLOCK_SIZE,
							0.f
						)
						- 1.f
					));
			}
		}

		void calcSmoothingTermY_gpu(
				const cv::gpu::DevMem2D src,
				const cv::gpu::DevMem2D bg,
				cv::gpu::DevMem2Df sterm,
				float noise_std_dev,
				cudaStream_t stream,
				int dev){
			int sharedMemSize;
			dim3 block = maxBlockSize(&sharedMemSize,2 * sizeof(int)* TEXCUT_BLOCK_SIZE,0,2 * sizeof(int)* TEXCUT_BLOCK_SIZE,0,true,dev);
			// sterm is transposed
			while(sterm.rows%block.y!=0){
				block.y/=2;
			}
			assert(block.y>0);
			while(sterm.cols%block.x!=0){
				block.x/=2;
			}
			assert(block.x>0);
//			std::cerr << "block_size: " << block.x << ", " << block.y << std::endl;

			dim3 grid(divUp(sterm.cols,block.x),divUp(sterm.rows,block.y) );

			calcSmoothingTermY_kernel<<<grid,block,sharedMemSize,stream>>>(
					src,
					bg,
					sterm,
					noise_std_dev,0,
					sterm.cols,sterm.rows);
			calcSmoothingTermY_kernel<<<grid,block,sharedMemSize,stream>>>(
					src,
					bg,
					sterm,
					noise_std_dev,TEXCUT_BLOCK_SIZE-1,
					sterm.cols,sterm.rows);
			cudaSafeCall( cudaGetLastError() );
		}

		__global__ void calcTexturalIntencity_kernel(
				const cv::gpu::PtrStepi sobel_x,
				const cv::gpu::PtrStepi sobel_y,
				cv::gpu::PtrStepf tex_intencity,
				int cols,
				int rows){
			int in_square_seq_idx = inSquareSeqIdx;
			dim3 gidx = GlobalIdx;

			// declare shared_memory
			int* auto_correlation = (int*)smem;

			int square_seq_idx_in_block = SquareSeqIdxInBlock;
			// offset to direct shared memory position
			// for the belonging square
			int offset = (TEXCUT_SQUARE_AREA+1) * square_seq_idx_in_block;
			int dx = sobel_x.ptr(gidx.y)[gidx.x];
			int dy = sobel_y.ptr(gidx.y)[gidx.x];
			auto_correlation[offset + in_square_seq_idx] = dx*dx + dy*dy;

			__syncthreads();
			sumUpi(auto_correlation + offset,
					TEXCUT_SQUARE_AREA,
					in_square_seq_idx);

			if(in_square_seq_idx!=0) return;
			dim3 graph_node_idx = SquareIdx;
			if(gidx.x < cols && gidx.y < rows){
				tex_intencity.ptr(graph_node_idx.y)[graph_node_idx.x] = (float)auto_correlation[offset];
			}
		}

		void calcTexturalIntencity_gpu(
				const cv::gpu::DevMem2Di sobel_x,
				const cv::gpu::DevMem2Di sobel_y,
				cv::gpu::DevMem2Df tex_intencity,
				cudaStream_t stream,
				int dev){
			int sharedMemSize;
			dim3 block = maxBlockSize(&sharedMemSize,(1.f+1.f/TEXCUT_SQUARE_AREA) * sizeof(int),0,0,0,true,dev);
			dim3 grid(divUp(sobel_x.cols,block.x), divUp(sobel_x.rows,block.y));

			calcTexturalIntencity_kernel<<<grid,block,sharedMemSize,stream>>>(
					sobel_x, sobel_y,
					tex_intencity,
					sobel_x.cols,
					sobel_x.rows);
			cudaSafeCall( cudaGetLastError() );
		}

		__global__ void bindUpDataTerm_kernel(
				cv::gpu::PtrStepf max_intencity,
				cv::gpu::PtrStepf max_gradient_heterogenuity,
				cv::gpu::PtrStepi terminals,
				const cv::gpu::PtrStepf fg_tex_intencity,
				const cv::gpu::PtrStepf bg_tex_intencity,
				const cv::gpu::PtrStepf fg_gradient_heterogenuity,
				const cv::gpu::PtrStepf bg_gradient_heterogenuity,
				cv::gpu::PtrStepf textural_correlation,
				float noise_std_dev,
				float thresh_tex_diff,
				const cv::gpu::PtrStepb is_over_under_exposure,
				int cols,
				int rows){

			dim3 gidx = GlobalIdx;
			float fg_ti = fg_tex_intencity.ptr(gidx.y)[gidx.x];
			float bg_ti = bg_tex_intencity.ptr(gidx.y)[gidx.x];


			float tex_int = normalize(
					sqrt(max(fg_ti,bg_ti))/TEXCUT_BLOCK_SIZE,2 * SQRT3 * noise_std_dev,0.f);

			float gh = max(fg_gradient_heterogenuity.ptr(gidx.y)[gidx.x],bg_gradient_heterogenuity.ptr(gidx.y)[gidx.x]);
			tex_int = exp((gh * tex_int)-1.f);

			// ignore over/under exposure and image boundary terminals.
			if(0 != is_over_under_exposure.ptr(gidx.y)[gidx.x]){
				tex_int = 0.f;
				gh = 0.f;
			}

			if(max_intencity.ptr(gidx.y)[gidx.x] > tex_int){
				return;
			}

			if(gidx.x<cols && gidx.y < rows){
				max_gradient_heterogenuity.ptr(gidx.y)[gidx.x] = gh;
				max_intencity.ptr(gidx.y)[gidx.x] = tex_int;

				if(fg_ti + bg_ti==0){
					terminals.ptr(gidx.y)[gidx.x] = 0.f;
					return;
				}
				terminals.ptr(gidx.y)[gidx.x] = 
					2 * (int)(
							GRAPHCUT_QUANTIZATION_LEVEL * 
							( tex_int * 
								 (1.f - (2.f * textural_correlation.ptr(gidx.y)[gidx.x])/(fg_ti + bg_ti)  - thresh_tex_diff) / (2.f * thresh_tex_diff) ));
			}
		}

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
				cudaStream_t stream,
				int dev){
			int sharedMemSize;
			dim3 block = maxBlockSize(&sharedMemSize,0,0,0,0,true,dev);
			dim3 grid(
					divUp(fg_tex_intencity.cols,block.x),
					divUp(fg_tex_intencity.rows,block.y));
			bindUpDataTerm_kernel<<<grid,block,sharedMemSize,stream>>>(
					max_intencity,
					max_gradient_heterogenuity,
					terminals,
					fg_tex_intencity,
					bg_tex_intencity,
					fg_gradient_heterogenuity,
					bg_gradient_heterogenuity,
					textural_correlation,
					noise_std_dev,
					thresh_tex_diff,
					is_over_under_exposure,
					max_intencity.cols,
					max_intencity.rows);
			cudaSafeCall( cudaGetLastError() );
		}

		__global__ void bindUpSmoothingTerms_kernel(
				const cv::gpu::PtrStepi terminals,
				const cv::gpu::PtrStepf gradient_heterogenuity,
				const cv::gpu::PtrStepf max_sterm_x,
				const cv::gpu::PtrStepf max_sterm_y,
				cv::gpu::PtrStepi rightTransp,
				cv::gpu::PtrStepi leftTransp,
				cv::gpu::PtrStepi bottom,
				cv::gpu::PtrStepi top,
				int cols,
				int rows){
			dim3 gidx = GlobalIdx;
			int array_width = blockDim.x+1;
			int array_height = blockDim.y+1;
			int array_idx = _SeqIdx(threadIdx.x,threadIdx.y,array_width);
			int* dt = (int*)smem;
			float* gh = (float*)(dt + array_width * array_height);

			dt[array_idx] = terminals.ptr(gidx.y)[gidx.x];
			gh[array_idx] = gradient_heterogenuity.ptr(gidx.y)[gidx.x];

			int mem_width = MEM_WIDTH;
			int mem_height = MEM_HEIGHT;
			int idx_plus = min(gidx.x+1,mem_width -1);

			if(threadIdx.x==blockDim.x-1){
				dt[array_idx+1] = terminals.ptr(gidx.y)[idx_plus];
				gh[array_idx+1] = gradient_heterogenuity.ptr(gidx.y)[idx_plus];
			}

			idx_plus = min(gidx.y+1,mem_height -1);
			if(threadIdx.y==blockDim.y-1){
				dt[array_idx+array_width] = terminals.ptr(idx_plus)[gidx.x];
				gh[array_idx+array_width] = abs(gradient_heterogenuity.ptr(idx_plus)[gidx.x]);
			}
			__syncthreads();

			int dt_bonus = -min(0,min(dt[array_idx],dt[array_idx+1]));
			float gh_penalty = exp( - max(gh[array_idx],gh[array_idx+1]));
			// hCue (sterm_x) is transposed
			int temp = smoothing_term_weight * max(0,(int)((max_sterm_x.ptr(gidx.x)[gidx.y] + gh_penalty) * GRAPHCUT_QUANTIZATION_LEVEL) + dt_bonus);
			if(gidx.x < cols && gidx.y < rows){
				rightTransp.ptr(gidx.x)[gidx.y] = (gidx.x == cols-1) ? 0 :temp;
				leftTransp.ptr((gidx.x+1)%mem_width)[gidx.y] = (gidx.x == mem_width-1) ? 0 : temp;
			}


			dt_bonus = -min(0,min(dt[array_idx],dt[array_idx+array_width]));
			gh_penalty = exp( - max(gh[array_idx],gh[array_idx+array_width]) );
			temp = smoothing_term_weight * max(0,(int)((max_sterm_y.ptr(gidx.y)[gidx.x] + gh_penalty) * GRAPHCUT_QUANTIZATION_LEVEL) + dt_bonus);
			if(gidx.x < cols && gidx.y < rows){
				bottom.ptr(gidx.y)[gidx.x] = (gidx.y == rows-1) ? 0 : temp;
				top.ptr((gidx.y+1)%mem_height)[gidx.x] = (gidx.y==mem_height-1) ? 0:temp;
			}
		}

		void bindUpSmoothingTerms_gpu(
				const cv::gpu::DevMem2Di terminals,
				const cv::gpu::DevMem2Df gradient_heterogenuity,
				const cv::gpu::DevMem2Df max_sterm_x,
				const cv::gpu::DevMem2Df max_sterm_y,
				cv::gpu::DevMem2Di rightTransp,
				cv::gpu::DevMem2Di leftTransp,
				cv::gpu::DevMem2Di bottom,
				cv::gpu::DevMem2Di top,
				cudaStream_t stream,
				int dev){
			int sharedMemSize;
			int byte_per_thread = sizeof(int)+sizeof(float);
			dim3 block = maxBlockSize(&sharedMemSize,byte_per_thread,byte_per_thread,byte_per_thread,byte_per_thread,true,dev);
			dim3 grid(
					divUp(terminals.cols,block.x),
					divUp(terminals.rows,block.y));
			bindUpSmoothingTerms_kernel<<<grid,block,sharedMemSize,stream>>>(
					terminals,
					gradient_heterogenuity,
					max_sterm_x,
					max_sterm_y,
					rightTransp,
					leftTransp,
					bottom,
					top,
					terminals.cols,
					terminals.rows);
			cudaSafeCall( cudaGetLastError() );
		}

		__global__ void checkOverExposure_kernel(
				const cv::gpu::PtrStepb img,
				cv::gpu::PtrStepb is_over_exposure,
				unsigned char over_exposure_thresh,
				int cols,
				int rows){
			dim3 gidx = GlobalIdx;
			dim3 graph_node_idx = SquareIdx;
			if(gidx.x < cols && gidx.y < rows &&
					img.ptr(gidx.y)[gidx.x] <= over_exposure_thresh){
				is_over_exposure.ptr(graph_node_idx.y)[graph_node_idx.x] = 0;
			}
		}
		__global__ void checkUnderExposure_kernel(
				const cv::gpu::PtrStepb img,
				cv::gpu::PtrStepb is_under_exposure,
				unsigned char under_exposure_thresh,
				int cols,
				int rows){
			dim3 gidx = GlobalIdx;
			dim3 graph_node_idx = SquareIdx;
			if(gidx.x < cols && gidx.y < rows &&
					under_exposure_thresh <= img.ptr(gidx.y)[gidx.x]){
				is_under_exposure.ptr(graph_node_idx.y)[graph_node_idx.x] = 0;
			}
		}


		void checkOverExposure_gpu(
				const cv::gpu::PtrStepSzb img,
				cv::gpu::PtrStepSzb is_over_exposure,
				unsigned char over_exposure_thresh,
				cudaStream_t stream,
				int dev){
			int sharedMemSize;
			dim3 block = maxBlockSize(&sharedMemSize,0,0,0,0,true,dev);
			dim3 grid(
					divUp(img.cols,block.x),
					divUp(img.rows,block.y));
			checkOverExposure_kernel<<<grid,block,sharedMemSize,stream>>>(
					img,
					is_over_exposure,
					over_exposure_thresh,
					img.cols,
					img.rows);
			cudaSafeCall( cudaGetLastError() );
		}
		void checkUnderExposure_gpu(
				const cv::gpu::PtrStepSzb img,
				cv::gpu::PtrStepSzb is_under_exposure,
				unsigned char under_exposure_thresh,
				cudaStream_t stream,
				int dev){
			int sharedMemSize;
			dim3 block = maxBlockSize(&sharedMemSize,0,0,0,0,true,dev);
			dim3 grid(
					divUp(img.cols,block.x),
					divUp(img.rows,block.y));
			checkUnderExposure_kernel<<<grid,block,sharedMemSize,stream>>>(
					img,
					is_under_exposure,
					under_exposure_thresh,
					img.cols,
					img.rows);
			cudaSafeCall( cudaGetLastError() );
		}


	}
}
