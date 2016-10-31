/*!
 * @file SampleSetReader.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/May/30
 * @date Last Change: 2012/Jun/29.
 */
#include "SampleSetReader.h"

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
SampleSetReader::SampleSetReader(){

}

/*!
 * @brief デストラクタ
 */
SampleSetReader::~SampleSetReader(){

}

bool SampleSetReader::read(const std::string& filename, cv::Mat* samples, cv::Mat* responces, cv::Mat* likelihoods,std::vector<skl::Time>* timestamps,std::vector<cv::KeyPoint>* keypoints){
	std::ifstream fin;
	fin.open(filename.c_str());
	if(!fin) return false;
	bool isSuccess = read(fin,samples,responces,likelihoods,timestamps,keypoints);
	fin.close();
	return isSuccess;
}

bool SampleSetReader::read(std::istream& in, cv::Mat* samples, cv::Mat* responces, cv::Mat* likelihoods,std::vector<skl::Time>* timestamps, std::vector<cv::KeyPoint>* keypoints){

	size_t sample_num;
	size_t sample_dim;
	size_t class_num;
	bool has_responce;
	bool has_timestamp;
	bool has_keypoint;

	if(!_readHeader(in,sample_num,sample_dim,has_responce,class_num,has_timestamp,has_keypoint)) return false;

	// read samplels
	if(samples!=NULL){
		if(!_readSamples(in, sample_num, sample_dim, *samples)) return false;
	}
	else{
		skip(in,sample_dim);
	}

	// read responces
	if(has_responce){
		if(responces!=NULL){
			if(!_readResponces(in, sample_num, *responces)) return false;
		}
		else{
			skip(in,1);
		}
	}

	// read likelihoods
	if(class_num>0){
		if(likelihoods!=NULL){
			if(!_readLikelihoods(in,sample_num,class_num, *likelihoods)) return false;
		}
		else{
			skip(in,class_num);
		}
	}

	// read timestamps
	if(has_timestamp){
		if(timestamps!=NULL){
			if(!_readTimestamps(in,sample_num,*timestamps)) return false;
		}
		else{
			skip(in,1);
		}
	}

	// read keypoints
	if(has_keypoint){
		if(keypoints!=NULL){
			if(!_readKeyPoints(in,sample_num,*keypoints)) return false;
		}
		else{
			skip(in,7);
		}
	}
	return true;
}


bool SampleSetReader::skip(std::istream& in,size_t num){
	std::string buf;
	for(size_t i=0;i<num;i++){
		if(in.eof()) return false;
		std::getline(in,buf);
	}
	return true;
}

bool SampleSetReader::_readHeader(std::istream& in, size_t& sample_num, size_t& sample_dim, bool& has_responce, size_t& class_num, bool& has_timestamp,bool& has_keypoint){
	std::string str;
	if(in.eof()) return false;
	std::getline(in,str);
	std::vector<std::string> buf;
	buf = skl::split_strip(str,",",6);
	if(buf.size()!=6) return false;
	sample_num = atoi(buf[0].c_str());
	sample_dim = atoi(buf[1].c_str());
	has_responce = atoi(buf[2].c_str())>0;
	class_num = atoi(buf[3].c_str());
	has_timestamp = atoi(buf[4].c_str())>0;
	has_keypoint = atoi(buf[5].c_str())>0;
	return true;
}

bool SampleSetReader::_readTimestamps(std::istream& in, size_t sample_num, std::vector<skl::Time>& dist){
	std::string str;
	if(in.eof()) return false;
	std::getline(in,str);
	std::vector<std::string> buf = skl::split_strip(str,",",(int)sample_num);
	if(buf.size()!=sample_num) return false;

	dist.resize(sample_num);
	for(size_t i=0;i<sample_num;i++){
		dist[i].parseString(buf[i]);
	}
	return true;
}

bool SampleSetReader::_readKeyPoints(std::istream& in, size_t sample_num, std::vector<cv::KeyPoint>& keypoints){
	std::string str;
	std::vector<std::string> buf[7];
	for(int i=0;i<7;i++){
		if(in.eof()) return false;
		std::getline(in,str);
		buf[i] = skl::split_strip(str,",",(int)sample_num);
		if(buf[i].size()!=sample_num) return false;
	}
	
	keypoints.resize(sample_num);
	for(size_t i=0;i<sample_num;i++){
		keypoints[i] = cv::KeyPoint(
				(float)atof(buf[0][i].c_str()),//x
				(float)atof(buf[1][i].c_str()),//y
				(float)atof(buf[2][i].c_str()),//size
				(float)atof(buf[3][i].c_str()),//angle
				(float)atof(buf[4][i].c_str()),//response
				atoi(buf[5][i].c_str()),//octave
				atoi(buf[6][i].c_str()) //class_id
				);
	}
	return true;
}
