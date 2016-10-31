#ifndef __SKL_CVUTILS_BLENDING_H__
#define __SKL_CVUTILS_BLENDING_H__

namespace skl{
	template<class T> void setWeight(const T& mask,double* w1, double* w2){
		*w1 = mask;
		*w2 = 1.0 - mask;
	}

	template<> void setWeight<unsigned char>(const unsigned char& mask, double* w1, double* w2);

	template<class T> T blend(const T& pix1, const T& pix2, double w1,double w2){
		return static_cast<T>(w1 * pix1 + w2 * pix2);
	}
	template<> cv::Vec3b blend(const cv::Vec3b& pix1,const cv::Vec3b& pix2, double w1, double w2);


	template <class ElemType,class WeightType> class ParallelBlending{
		public:
			ParallelBlending(
					const cv::Mat& src1,
					const cv::Mat& src2,
					const cv::Mat& mask,
					cv::Mat& dest):
				src1(src1),src2(src2),mask(mask),dest(dest){}
			~ParallelBlending(){}
			void operator()(const cv::BlockedRange& range)const{
				for(int i=range.begin();i!=range.end();i++){
					int y = i / mask.cols;
					int x = i % mask.cols;
					double weight1, weight2;
					setWeight<WeightType>(mask.at<WeightType>(y,x), &weight1,&weight2);
					dest.at<ElemType>(y,x) = blend<ElemType>(
							src1.at<ElemType>(y,x),
							src2.at<ElemType>(y,x),
							weight1,weight2);
				}
			}
		protected:
			const cv::Mat& src1;
			const cv::Mat& src2;
			const cv::Mat& mask;
			cv::Mat& dest;

	};
} // namespace skl


#endif // __SKL_CVUTILS_BLENDING_H__