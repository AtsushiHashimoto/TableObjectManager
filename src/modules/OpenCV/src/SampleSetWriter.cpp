/*!
 * @file SampleSetWriter.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/May/30
 * @date Last Change: 2012/Jun/29.
 */
#include "SampleSetWriter.h"

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
SampleSetWriter::SampleSetWriter(){

}

/*!
 * @brief デストラクタ
 */
SampleSetWriter::~SampleSetWriter(){

}

bool SampleSetWriter::write(const std::string& filename, const cv::Mat* samples, const cv::Mat* responces, const cv::Mat* likelihoods,const std::vector<skl::Time>* timestamps, const std::vector<cv::KeyPoint>* keypoints){
	std::ofstream fout;
	fout.open(filename.c_str());
	if(!fout) return false;
	bool isSuccess = write(fout,samples,responces,likelihoods,timestamps,keypoints);
	fout.close();
	return isSuccess;
}

bool SampleSetWriter::_writeHeader(std::ostream& out, size_t sample_num, size_t sample_dim, bool has_responce, size_t class_num, bool has_timestamp,bool has_keypoint){
	out << sample_num << "," << sample_dim << "," << has_responce << "," << class_num << "," << has_timestamp << "," << has_keypoint << std::endl;
	return true;
}

bool SampleSetWriter::write(std::ostream& out, const cv::Mat* samples, const cv::Mat* responces, const cv::Mat* likelihoods,const std::vector<skl::Time>* timestamps, const std::vector<cv::KeyPoint>* keypoints){
	// each column corresponds to a sample in samples
	// each row corresponds to a feature dimension in samples
	// responces must be a matrix with cols==1, rows==sample_num
	// likelihood must be a matrix with cols==class_num, rows==sample_num

	size_t sample_num = samples->rows;
	size_t sample_dim = samples->cols;
	bool has_responce = (responces != NULL);
	size_t class_num = 0;
	if(likelihoods!=NULL){
		class_num = likelihoods->cols;
	}
	bool has_timestamp = timestamps != NULL;
	if(timestamps->size()!=sample_num) return false;
	bool has_keypoint = keypoints != NULL;
	if(keypoints->size()!=sample_num) return false;

	// write header
	if(!_writeHeader(out,sample_num,sample_dim,has_responce,class_num,has_timestamp,has_keypoint)) return false;

	// write samples
	if(!_writeSamples(out,*samples)) return false;
	

	// write responce
	if(has_responce){
		if(responces->rows!=(int)sample_num) return false;
		if(responces->cols!=1) return false;
		if(!_writeResponces(out,*responces)) return false;
	}

	// write likelihood
	if(class_num > 0){
		if(likelihoods->rows!=(int)sample_num) return false;
		if(!_writeLikelihoods(out,*likelihoods)) return false;
	}

	// write timestamps
	if(has_timestamp){
		if(!_writeTimeStamps(out,*timestamps)) return false;
	}

	// write keypoints
	if(has_keypoint){
		if(!_writeKeyPoints(out,*keypoints)) return false;
	}
	return true;
}

bool SampleSetWriter::_writeTimeStamps(std::ostream& out,const std::vector<skl::Time>& timestamps){
	for(size_t i=0;i<timestamps.size();i++){
		if(i!=0) out << ",";
		out << timestamps[i];
	}
	out << std::endl;
	return true;
}

bool SampleSetWriter::_writeKeyPoints(std::ostream& out, const std::vector<cv::KeyPoint>& keypoints){
	for(size_t i=0;i<keypoints.size();i++){
		if(i!=0) out << ",";
		out << keypoints[i].pt.x;
	}
	out << std::endl;
	for(size_t i=0;i<keypoints.size();i++){
		if(i!=0) out << ",";
		out << keypoints[i].pt.y;
	}
	out << std::endl;
	for(size_t i=0;i<keypoints.size();i++){
		if(i!=0) out << ",";
		out << keypoints[i].size;
	}
	out << std::endl;
	for(size_t i=0;i<keypoints.size();i++){
		if(i!=0) out << ",";
		out << keypoints[i].angle;
	}
	out << std::endl;
	for(size_t i=0;i<keypoints.size();i++){
		if(i!=0) out << ",";
		out << keypoints[i].response;
	}
	out << std::endl;
	for(size_t i=0;i<keypoints.size();i++){
		if(i!=0) out << ",";
		out << keypoints[i].octave;
	}
	out << std::endl;
	for(size_t i=0;i<keypoints.size();i++){
		if(i!=0) out << ",";
		out << keypoints[i].class_id;
	}
	out << std::endl;
	return true;
}
