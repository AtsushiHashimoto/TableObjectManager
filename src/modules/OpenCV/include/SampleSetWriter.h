/*!
 * @file SampleSetWriter.h
 * @author a_hasimoto
 * @date Date Created: 2012/May/30
 * @date Last Change:2012/Jul/09.
 */
#ifndef __SKL_SAMPLE_SET_WRITER_H__
#define __SKL_SAMPLE_SET_WRITER_H__

#include <fstream>
#include "skl.h"
#include "cv.h"

namespace skl{

/*!
 * @class SampleSetWriter
 * @brief 学習用/評価用の特徴量サンプルセットの書き込みを行う
 */
class SampleSetWriter{

	public:
		SampleSetWriter();
		virtual ~SampleSetWriter();
		static bool write(
				const std::string& filename, 
				const cv::Mat* samples, 
				const cv::Mat* responces=NULL, 
				const cv::Mat* likelihoods=NULL,
				const std::vector<skl::Time>* timestamps=NULL, 
				const std::vector<cv::KeyPoint>* keypoints=NULL);

		static bool write(
				std::ostream& out, 
				const cv::Mat* samples, 
				const cv::Mat* responces=NULL, 
				const cv::Mat* likelihoods=NULL,
				const std::vector<skl::Time>* timestamps=NULL, 
				const std::vector<cv::KeyPoint>* keypoints=NULL);




	protected:

		static bool _writeHeader(std::ostream& out, size_t sample_num, size_t sample_dim, bool has_responce, size_t class_num, bool has_timestamp,bool has_keypoint);
		template<class Type,int CV_DEPTH> static bool _writeMat(std::ostream& out,const cv::Mat& src);

		inline static bool _writeSamples(std::ostream& out,const cv::Mat& src){
			if(!_writeMat<float,CV_32F>(out,src)) return false;
			return true;
		}
		inline static bool _writeResponces(std::ostream& out,const cv::Mat& src){
			if(!_writeMat<int,CV_32S>(out,src)) return false;
			return true;
		}
		inline static bool _writeLikelihoods(std::ostream& out,const cv::Mat& src){
			if(!_writeMat<float,CV_32F>(out,src)) return false;
			return true;
		}
		static bool _writeTimeStamps(std::ostream& out,const std::vector<skl::Time>& timestamps);
		static bool _writeKeyPoints(std::ostream& out,const std::vector<cv::KeyPoint>& keypoints);
	private:
		
};

template<class Type,int CV_DEPTH> bool SampleSetWriter::_writeMat(std::ostream& out,const cv::Mat& src){
	// 書き込みは転置した状態で行う
	for(int y=0;y<src.cols;y++){
		for(int x=0;x<src.rows;x++){
			if(x!=0) out << ",";
			out << std::fixed << src.at<Type>(x,y);
		}
		out << std::endl;
	}
	return true;
}
} // skl

#endif // __SKL_SAMPLE_SET_WRITER_H__

