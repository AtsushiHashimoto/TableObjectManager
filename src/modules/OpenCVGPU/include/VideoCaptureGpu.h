/*!
 * @file VideoCaptureGpu.h
 * @author a_hasimoto
 * @date Date Created: 2012/Feb/10
 * @date Last Change:2013/Dec/02.
 */
#ifndef __SKL_GPU_VIDEO_CAPTURE_GPU_H__
#define __SKL_GPU_VIDEO_CAPTURE_GPU_H__

#include <opencv2/gpu/gpu.hpp>
#include "sklcv.h"


namespace skl{
	typedef cv::Ptr<_VideoCaptureInterface> VideoCapturePtr;

	namespace gpu{
		/*!
		 * @brief cv::gpu::GpuMatを返すキャプチャ。処理中に次のフレームを非同期でdeviceに送る分、連続フレームに対する処理が高速化される。
		 */
		class VideoCaptureGpu: public VideoCaptureInterface<VideoCaptureGpu>{
			public:
				using _VideoCaptureInterface::set;
				using _VideoCaptureInterface::get;
				using VideoCaptureInterface<VideoCaptureGpu>::operator>>;

				VideoCaptureGpu(VideoCapturePtr video_capture_cpu=VideoCapturePtr());
				virtual ~VideoCaptureGpu();

				inline void setBaseCapture(VideoCapturePtr video_capture_cpu){
					assert(!video_capture_cpu.empty());
					this->video_capture_cpu = video_capture_cpu;
				}

				inline bool open(const std::string& filename){
					if(video_capture_cpu.empty()) return false;
					return video_capture_cpu->open(filename);
				}
				inline bool open(int device){
					if(video_capture_cpu.empty()) return false;
					return video_capture_cpu->open(device);
				}
				inline bool isOpened()const{
					if(video_capture_cpu.empty()) return false;
					return video_capture_cpu->isOpened();
				}
				inline void release(){
					s.waitForCompletion();
					if(!video_capture_cpu.empty()){
						video_capture_cpu.release();
					}
					isNextFrameUploaded = false;
				}

				bool grab();
				inline virtual bool retrieve(cv::Mat& image, int channel=0){
					if(channel!=0) return false;
					image = switching_mat_cpu[_switch];
					return true;
				}
				inline virtual bool retrieve(cv::gpu::GpuMat& image, int channel=0){
					if(channel!=0) return false;
					image = switching_mat[_switch];
					return true;
				}

				inline bool set(capture_property_t prop_id,double val){
					isNextFrameUploaded = false;
					return video_capture_cpu->set(prop_id,val);
				}
				inline double get(capture_property_t prop_id){
					if(prop_id==skl::POS_FRAMES){
						int pos_frame = static_cast<int>(video_capture_cpu->get(prop_id));
						if(isNextFrameUploaded){
							return pos_frame - 1;
						}
						else{
							return pos_frame;
						}
					}
					return video_capture_cpu->get(prop_id);
				}

				inline VideoCaptureGpu& operator>>(cv::gpu::GpuMat& gpu_mat){
					if(!grab()){
						gpu_mat.release();
					}
					else{
						retrieve(gpu_mat);
					}
					return *this;
				}


			protected:
				VideoCapturePtr video_capture_cpu;
				cv::gpu::GpuMat switching_mat[2];
				cv::Mat switching_mat_cpu[2];
				cv::gpu::CudaMem page_locked_mat;
				bool isNextFrameUploaded;
				bool _switch;
				cv::gpu::Stream s;

				bool grabNextFrame();
			private:
		};

	} // skl
} // gpu

#endif // __SKL_GPU_VIDEO_CAPTURE_G_P_U_H__

