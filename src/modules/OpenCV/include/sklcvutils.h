#ifndef __SKL_CV_UTILS_H__
#define __SKL_CV_UTILS_H__

#ifdef _WIN32
#pragma warning(disable:4996)
#endif
#include <cv.h>

#define SKL_GRAY 127

// &(get the "and" region, cap operation)
// |(get the "or" region, cup operation)
cv::Rect operator&(const cv::Rect& left, const cv::Rect& right);
cv::Rect operator|(const cv::Rect& left, const cv::Rect& right);

// && returns true if two rectanbles are overlapped.
bool operator&&(const cv::Rect& left, const cv::Rect& right);

// Bayer to BGR image interpolation
// Used in class skl::VideoCaptureFlyCapture
#include "sklcvutils_bayer_conversion.h"

// template subfunctions for skl::blending
#include "sklcvutils_blending.h"

// use with a template function like below
// int FuncName<MatType>(...);
// sample code are available in 
#define cvMatDepthTemplateCall(MatDepth,FuncName,ReturnVal,...)\
	switch(MatDepth){\
		case CV_8U:\
			ReturnVal = FuncName<unsigned char>(__VA_ARGS__);\
			break;\
		case CV_8S:\
			ReturnVal = FuncName<char>(__VA_ARGS__);\
			break;\
		case CV_16U:\
			ReturnVal = FuncName<unsigned short>(__VA_ARGS__);\
			break;\
		case CV_16S:\
			ReturnVal = FuncName<short>(__VA_ARGS__);\
			break;\
		case CV_32S:\
			ReturnVal = FuncName<int>(__VA_ARGS__);\
			break;\
		case CV_32F:\
			ReturnVal = FuncName<float>(__VA_ARGS__);\
			break;\
		case CV_64F:\
			ReturnVal = FuncName<double>(__VA_ARGS__);\
			break;\
		default:\
			std::cerr << "ERROR: unknown depth type '" << MatDepth << "'." << std::endl;\
			assert(0);\
	}

#define cvMatDepthTemplateCall4Vec(MatDepth,FuncName,ReturnVal,...)\
	switch(MatDepth){\
		case CV_8U:\
		case CV_8S:\
			ReturnVal = FuncName<cv::Vec3b>(__VA_ARGS__);\
			break;\
		case CV_16U:\
		case CV_16S:\
			ReturnVal = FuncName<cv::Vec3s>(__VA_ARGS__);\
			break;\
		case CV_32S:\
			ReturnVal = FuncName<cv::Vec3i>(__VA_ARGS__);\
			break;\
		case CV_32F:\
			ReturnVal = FuncName<cv::Vec3f>(__VA_ARGS__);\
			break;\
		case CV_64F:\
			ReturnVal = FuncName<cv::Vec3d>(__VA_ARGS__);\
			break;\
		default:\
			std::cerr << "ERROR: unknown depth type '" << MatDepth << "'." << std::endl;\
			assert(0);\
	}

namespace skl{

	// check cv::Mat type/size and return debug message.	
	bool checkMat(const cv::Mat& mat, int depth = -1,int channels = 0,cv::Size size = cv::Size(0,0) );
	// check cv::Mat type/size and allocate the appropriate matrix if needed.
	bool ensureMat(cv::Mat& mat, int depth, int channels, cv::Size);

	cv::Rect fitRect(const std::vector< cv::Point >& points);

	// generate gaussian mask.
	// ignore_rate: approximate mask value to 0 if the point is futher than ignore_rate * sigma, where sigma is standard variance. ignore_rate = 2 means 96% of vote are taken in consideration.
	cv::Mat generateGaussianMask(const cv::Mat& covariance,double ignore_rate=2.0);

	// a simple ransac estimation.
	// the error for model evaluation is calcurated by sum of L2 norm between "estimated model parameter" and sample parameters.
	// see http://en.wikipedia.org/wiki/RANSAC for algorithm details.
	cv::Mat ransac(const cv::Mat& samples, cv::TermCriteria termcrit, double thresh_outliar, double sampling_rate = 0.2, double minimum_inliar_rate = 0.2);

	//! fill color palette acording to palette.size(). Different colors are set to each elem in palette.
	//! the difference are decided in HLS color space.
	void fillColorPalette(
			std::vector<cv::Scalar>& palette,
			size_t hue_pattern_num=7,
			bool use_gray=true,
			int min_luminance_value=20);

	// for visualize the result returned by RegionLabelingAlgorith classes.
	cv::Mat visualizeRegionLabel(const cv::Mat& label,size_t region_num);
	cv::Mat visualizeRegionLabel_int(const cv::Mat& label,size_t region_num);
	enum ArrowType{
		NONE,
		ARROW,
		ARROW_FILL,
		INV_ARROW,
		INV_ARROW_FILL,
		CIRCLE,
		CIRCLE_FILL,
		SQUARE,
		SQUARE_FILL,
		ABS_SQUARE,
		ABS_SQUARE_FILL,
		DIAMOND,
		DIAMOND_FILL,
		ABS_DIAMOND,
		ABS_DIAMOND_FILL
	};

	// you can easily draw an arrow instead of cv::line!
	void arrow(
		cv::Mat& img,
		cv::Point pt1,
		cv::Point pt2,
		const cv::Scalar& color,
		int thickness = 1,
		int lineType = 8,
		ArrowType head_type = ARROW,
		ArrowType tail_type = NONE,
		int head_size = 6,
		int tail_size = 6,
		int head_degree = 30,
		int tail_degree = 30,
		int shift=0);


	template<class ElemType,class WeightType> void blending(const cv::Mat& src1,const cv::Mat& src2, const cv::Mat& weight_mask, cv::Mat& dest){
		assert(weight_mask.size()==src1.size());
		assert(weight_mask.size()==src2.size());
		assert(src1.type()==src2.type());
		assert(src1.size()==dest.size());
		assert(src1.type()==dest.type());
		cv::parallel_for(
				cv::BlockedRange(0,src1.rows*src1.cols),
				ParallelBlending<ElemType,WeightType>(src1,src2,weight_mask,dest)
				);
	}

	template<class ElemType,class WeightType> cv::Mat blending(const cv::Mat& src1,const cv::Mat& src2, const cv::Mat& weight_mask){
		cv::Mat dest = cv::Mat::zeros(src1.size(),src1.type());
		blending<ElemType,WeightType>(src1,src2,weight_mask,dest);
		return dest;
	}

	template<class T>
	bool isInRange(const T& val, const T& min, const T& max){
		return (min<val) && (val<max);
	}

#define MEDIAN_SD_RATIO 0.67449f
	template<class T> void estimate_noise(const cv::Mat& src, std::vector<T>& std_dev){
		assert(src.type()==CV_8UC3);
		size_t channels = src.channels();

		std::vector<std::vector<T> > abs_diff(channels),diff(channels);

		const cv::Vec3b* upper_column;
		const cv::Vec3b* lower_column;
		T temp;

		for(int y=0,idx=0;y<src.rows;y++){
			upper_column = src.ptr<cv::Vec3b>(y);
			if(y!=src.rows-1){
				lower_column = src.ptr<cv::Vec3b>(y+1);
			}
			else{
				lower_column = NULL;
			}

			for(int x=0;x<src.cols;x++,idx++){
				for(size_t c=0;c<channels;c++){
					if(!isInRange((int)upper_column[x][c],0,255)) continue;

					if(x<src.cols-1){
						if(!isInRange((int)upper_column[x+1][c],0,255)) continue;
						temp = (T)upper_column[x][c] - (T)upper_column[x+1][c];
//						if(temp==0) continue;
						diff[c].push_back(temp);
						abs_diff[c].push_back(std::abs(temp));
					}

					if(lower_column!=NULL){
						if(!isInRange((int)lower_column[x][c],0,255)) continue;
						temp = (T)upper_column[x][c]-(T)lower_column[x][c];
//						if(temp==0) continue;
						diff[c].push_back(temp);
						abs_diff[c].push_back(std::abs(temp));
					}
				}
			}
		}

		std_dev.resize(channels);
		for(size_t c=0;c<channels;c++){
			size_t valid_num = diff[c].size();
			size_t median_index = valid_num/2;
			assert(valid_num>0);
			std::nth_element(abs_diff[c].begin(),abs_diff[c].begin()+median_index,abs_diff[c].end());

			T threshold = std::max(abs_diff[c][median_index],1.f) * static_cast<T>(4.f/MEDIAN_SD_RATIO);
//			std::cerr << "std_dev by median [" << c << "] = " << (threshold/2) << std::endl;
			T mean(0), square_mean(0);
			abs_diff[c].clear();
			size_t count(0);
			for(size_t i=0;i<diff[c].size();i++){
				if(std::abs(diff[c][i]) > threshold) continue;
				mean += diff[c][i];
				square_mean += std::pow(diff[c][i],static_cast<T>(2));
				count++;
			}
			square_mean /= count;
			mean /= count;
			std_dev[c] = std::sqrt(square_mean - std::pow(mean,static_cast<T>(2)));
//			std::cerr << "std_dev[" << c << "] = " << std_dev[c] << std::endl;
		}


	}


	/********************************************************/
	/*                                                      */
	/*        "not very common" functions are below.        */
	/*                                                      */
	/********************************************************/


	// used in Patch.cpp
	cv::Mat blur_mask(const cv::Mat& mask, size_t blur_width);

	// used in Patch.cpp
	void edge_difference(const cv::Mat& src1, const cv::Mat& src2, cv::Mat& edge1, cv::Mat& edge2, double canny_thresh1=24, double canny_thresh2=24, int aperture_size=3, int dilate_size=3);

	// used in TableObjectManager.cpp
	template <class LabelType> void resize_label(const cv::Mat& label_small,size_t scale, cv::Mat& label){
		assert(label_small.channels()==1);
		assert(label_small.type()==label.type());
		cv::Size label_size(label_small.size());
		label_size.width *= (int)scale;
		label_size.height *= (int)scale;
		assert(label_size == label.size());

		label = cv::Scalar(0);
		if(label.isContinuous()){
			label.cols *= label.rows;
			label.rows = 1;
		}

		LabelType *plabel;
		const LabelType *plabel_small = 0;
		int y = -1, x = 0;
		int y_counter = 0;
		LabelType elem;
		for(int ly = 0; ly < label.rows; ly++){
			//			assert(0<=ly);
			//			assert(ly<label.rows);
			plabel = label.ptr<LabelType>(ly);

			for(int lx = 0; lx < label.cols; lx+=(int)scale,x++){
				if(lx % label_size.width == 0){
					if(y_counter%scale == 0){
						y++;
						//						assert(0<=y);
						//						assert(y < label_small.rows);
						plabel_small = label_small.ptr<const LabelType>(y);
					}
					y_counter++;
					x = 0;
				}
				//				assert(0 <= x);
				//				assert(x < label_small.cols);
				//				assert(0 <= lx);
				//				assert(lx < label.cols);
				if(0==(elem = plabel_small[x])) continue;
				for(size_t s=0;s<scale;s++){
					plabel[lx+s] = elem;
				}
			}
		}
		if(label.isContinuous()){
			label.cols = label_size.width;
			label.rows = label_size.height;
		}
	}

	// used in TableObjectManager.cpp
	template <class LabelType> cv::Mat resize_label(const cv::Mat& label_small,size_t scale){
		cv::Size label_size = label_small.size();
		label_size.width *= (int)scale;
		label_size.height *= (int)scale;
		cv::Mat label = cv::Mat(label_size,label_small.type());
		resize_label<LabelType>(label_small,scale,label);
		return label;
	}

	void binary2float(const cv::Mat& binary_mat, cv::Mat& float_mat);
}

#endif // __SKL_CV_UTILS_H__
