/*!
 * @file SampleSetReader.h
 * @author yamamoto, a_hasimoto
 * @date Date Created: 2012/May/30
 * @date Last Change:2012/Jun/29.
 */
#ifndef __SKL_SAMPLE_SET_READER_H__
#define __SKL_SAMPLE_SET_READER_H__

#include <fstream>
#include "cv.h"
#include "skl.h"

namespace skl{

/*!
 * @class SampleSetReader
 * @brief 学習用/評価用の特徴量サンプルセットの読み込みを行う
 */
class SampleSetReader{

	public:
		SampleSetReader();
		virtual ~SampleSetReader();
		static bool read(const std::string& filename, cv::Mat* samples=NULL, cv::Mat* responces=NULL, cv::Mat* likelihoods=NULL,std::vector<skl::Time>* timestamps=NULL, std::vector<cv::KeyPoint>* keypoints=NULL);
		static bool read(std::istream& in, cv::Mat* samples=NULL, cv::Mat* responces=NULL, cv::Mat* likelihoods=NULL,std::vector<skl::Time>* timestamps=NULL,std::vector<cv::KeyPoint>* keypoints=NULL);
	protected:
		static bool skip(std::istream& in,size_t num);
		template <class Type,size_t CV_DEPTH> static bool _readMatrix(std::istream& in, size_t cols, size_t rows, cv::Mat& dist);

		static bool _readHeader(std::istream& in, size_t& sample_num,size_t& sample_dim,bool& has_responce, size_t& class_num, bool& has_timestamp, bool& has_keypoint);

		inline static bool _readSamples(std::istream& in, size_t sample_num, size_t sample_dim, cv::Mat& dist){
			return _readMatrix<float,CV_32F>(in,sample_dim,sample_num,dist);
		}

		inline static bool _readResponces(std::istream& in, size_t sample_num, cv::Mat& dist){
			return _readMatrix<int,CV_32S>(in,1,sample_num,dist);
		}

		inline static bool _readLikelihoods(std::istream& in, size_t sample_num, size_t class_num, cv::Mat& dist){
			return _readMatrix<float,CV_32F>(in,class_num,sample_num,dist);
		}
		static bool _readTimestamps(std::istream& in, size_t sample_num, std::vector<skl::Time>& dist);
		static bool _readKeyPoints(std::istream& in, size_t sample_num, std::vector<cv::KeyPoint>& dist);

	private:
};

template <class Type,size_t CV_DEPTH> bool SampleSetReader::_readMatrix(std::istream& in, size_t cols, size_t rows, cv::Mat& dist){
	if(dist.depth()!=CV_DEPTH || dist.channels()!=1 || dist.cols!=(int)cols || dist.rows!=(int)rows){
		dist = cv::Mat::zeros(cv::Size((int)cols,(int)rows),CV_DEPTH);
	}
	std::string str;
	std::vector<std::string> buf;
	for(size_t y=0;y<cols;y++){
		if(in.eof()) return false;
		std::getline(in,str);
		buf = skl::split_strip(str,",",(int)rows);
		if(buf.size()!=rows) return false;
		for(size_t x=0;x<rows;x++){
			Type val = static_cast<Type>(atof(buf[x].c_str()));
			dist.at<Type>((int)x,(int)y) = val;
		}
	}
	return true;
}


} // skl

#endif // __SKL_SAMPLE_SET_READER_H__

