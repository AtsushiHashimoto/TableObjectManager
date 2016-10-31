/*!
 * @file shared_funcs.cu
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/25
 * @date Last Change: 2012/Aug/02.
 */
#ifndef __SKL_GPU_KERNEL_SHARED_FUNCS_CU__
#define __SKL_GPU_KERNEL_SHARED_FUNCS_CU__

#define _GIdx(elem) ( ( blockIdx.elem * blockDim.elem ) + threadIdx.elem )
#define _SeqIdx(idxx,idxy,width) ( ( (idxy) * (width) ) + (idxx) )

#define GlobalIdx dim3(\
		_GIdx(x) ,\
		_GIdx(y) )
#define inThreadSeqIdx _SeqIdx(threadIdx.x,threadIdx.y,blockDim.x)
#define MEM_WIDTH ( gridDim.x * blockDim.x )
#define MEM_HEIGHT ( gridDim.y * blockDim.y )

#define ThreadSize ( blockDim.x * blockDim.y )


template<class T> __device__ void sumUp_(T* array, int length,const size_t idx){
	for(size_t s=length/2;s>0;s>>=1){
		if(idx<s){
			array[idx] += array[idx+s];
		}
		__syncthreads();
	}
}

inline __device__ void sumUpf(float* array, int length, const size_t idx){sumUp_(array,length,idx);}
inline __device__ void sumUpi(int* array, int length, const size_t idx){sumUp_(array,length,idx);}


template<class T> __device__ void sort_(T* array, int length, const size_t idx, int* order){
	order[idx] = 0;
	int i;
	for(i=0;i<idx;i++){
		// more smaller order for the same-value elements which lays before idx.
		if(array[i]<=array[idx]) order[idx]++;
	}
	for(i=idx+1;i<length;i++){
		// more bigger order for the same-value elements which lays after idx;
		if(array[i]<array[idx]) order[idx]++;
	};
}
inline __device__ void sortf(float* array, int length, const size_t idx, int* order){
	sort_(array,length,idx,order);
}
inline __device__ void sorti(int* array, int length, const size_t idx, int* order){
	sort_(array,length,idx,order);
}
inline __device__ void sortb(char* array, int length, const size_t idx, int* order){
	sort_(array,length,idx,order);
}

template<class T> inline __device__ void swap_(T& a, T& b){
	{
		T temp(a);
		a = b;
		b = temp;
	}
}

inline __device__ void swapf(float& a, float& b){ return swap_(a,b); }
inline __device__ void swapi(int& a, int& b){ return swap_(a,b); }
inline __device__ void swapb(char& a, char& b){ return swap_(a,b); }

#endif // __SKL_GPU_KERNEL_SHARED_FUNCS_CU__
