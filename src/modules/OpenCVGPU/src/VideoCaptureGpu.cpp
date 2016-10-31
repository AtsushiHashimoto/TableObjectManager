/*!
 * @file VideoCaptureGpu.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Feb/10
 * @date Last Change: 2012/Jul/31.
 */
#include "VideoCaptureGpu.h"
#include "sklcv.h"
#include "sklcvgpu_utils.h"
using namespace skl;
using namespace skl::gpu;

/*!
 * @brief デフォルトコンストラクタ
*/
VideoCaptureGpu::VideoCaptureGpu(cv::Ptr<_VideoCaptureInterface> video_capture_cpu):video_capture_cpu(video_capture_cpu),isNextFrameUploaded(false),_switch(false){
	if(this->video_capture_cpu.empty()){
		this->video_capture_cpu = cv::Ptr<_VideoCaptureInterface>(new skl::VideoCapture());
	}
}


/*!
 * @brief デストラクタ
 */
VideoCaptureGpu::~VideoCaptureGpu(){
	release();
}

bool VideoCaptureGpu::grab(){
	if(!isNextFrameUploaded){
		if(!grabNextFrame()){
			return false;
		}
	}

	// get NextFrame;
	s.waitForCompletion();
	_switch = !_switch;

	if(!grabNextFrame()){
		return false;
	}
	isNextFrameUploaded = true;
	return true;
}

bool VideoCaptureGpu::grabNextFrame(){
	bool next_frame = !_switch;
	if(!video_capture_cpu->grab()){
		isNextFrameUploaded = false;
		return false;
	}

	cv::Mat& mat = switching_mat_cpu[next_frame];
	if(!video_capture_cpu->retrieve(mat)){
		isNextFrameUploaded = false;
		return false;
	}

	// copy retrieved data into page-locked memory in order to use in cuda stream.
	cv::Mat pl_mat;
	copyMatToPageLockedCudaMem(mat, page_locked_mat, pl_mat);

//	std::cerr << switching_mat_cpu[next_frame].cols << " x " << switching_mat_cpu[next_frame].rows << std::endl;
	cv::gpu::ensureSizeIsEnough(pl_mat.size(),pl_mat.type(),switching_mat[next_frame]);
	s.enqueueUpload(pl_mat,switching_mat[next_frame]);
	return true;
}


