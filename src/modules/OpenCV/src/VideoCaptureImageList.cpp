/*!
 * @file VideoCaptureImageList.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/18
 * @date Last Change: 2013/Dec/02.
 */
#include "VideoCaptureImageList.h"
#include <fstream>

#include <boost/regex.hpp>
#include <boost/tuple/tuple.hpp>
using namespace skl;

#define getImage(index) img_list_[img_list[index].first]
#define isFake(index) img_list[index].second

/*!
 * @brief デフォルトコンストラクタ
 */
VideoCaptureImageList::VideoCaptureImageList():index(0),bayer_change(-1),imread_option(-1),checked_frame_pos(-1),_forbidFake(false){
}

/*!
 * @brief デストラクタ
 */
VideoCaptureImageList::~VideoCaptureImageList(){

}

typedef std::pair<size_t,bool> PseudoIndex;
void generateIndexStraight(const std::vector<std::string>& list,std::vector<PseudoIndex>& index){
#ifdef _DEBUG
	std::cerr << "make an straght index." << std::endl;
#endif
	index.resize(list.size());
	for(size_t i=0;i<list.size();i++){
		index[i].first = i;
		index[i].second = true;
	}
}

/**
 * 文字列中から文字列を検索して別の文字列に置換する
 * @param str  : 置換対象の文字列。上書かれます。
 * @param from : 検索文字列
 * @param to   : 置換後の文字列
 */
void strReplace (std::string& str, const std::string& from, const std::string& to) {
    std::string::size_type pos = 0;
    while(pos = str.find(from, pos), pos != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

void VideoCaptureImageList::generateIndex(const std::vector<std::string>& list,std::vector<PseudoIndex>& index){
	std::string extname = skl::ExtName(list[0]);
	boost::regex e(std::string("(.*?)(\\d+)")+extname);

	boost::smatch match;

	if(!boost::regex_search(list[0], match, e)){
		generateIndexStraight(list,index);
		return;
	}
	
	std::string prefix = match[1];
	size_t serial_num_len = std::string(match[2]).size();

	// for '\\' in the path (mainly for windows)
	strReplace(prefix,"\\","\\\\");
	std::stringstream ss_e;
	ss_e << prefix << "(\\d{" << serial_num_len << "})" << extname;
	boost::regex e_strict(ss_e.str());

	size_t prev_index = 0;
	bool flag = true;
	for(size_t i=0;i<list.size();i++){
		if(!boost::regex_search(list[i],match,e_strict)){
			flag = false;
			break;
		}
		size_t frame_idx = atoi(std::string(match[1]).c_str());
		for(size_t k=prev_index+1;k<frame_idx;k++){
//			std::cerr << k << " : " << (prev_index+1) << ": " << frame_idx << std::endl;
			index.push_back(PseudoIndex(i-1,false));
		}
		index.push_back(PseudoIndex(i,true));
		prev_index = frame_idx;
	}
#ifdef _DEBUG
	if(flag){
		std::cerr << "make an pseudo index." << std::endl;
	}
#endif
	if(!flag){
		generateIndexStraight(list,index);
	}
	return;
}

bool VideoCaptureImageList::open(const std::string& filename){
	release();
	std::ifstream fin;
	fin.open(filename.c_str());
	if(!fin){
		std::cerr << "ERROR: failed to open file '" << filename << "'." << std::endl;
		return false;
	}
	std::string str;
	while(fin && std::getline(fin,str)){
		str = skl::strip(str);
		if(str.empty()) continue;
		if(str[0]=='#') continue;
		img_list_.push_back(str);
	}
	fin.close();
	generateIndex(img_list_,img_list);

	return params.set(FRAME_COUNT,(double)img_list.size());
}

void VideoCaptureImageList::release(){
	index = 0;
	img_list.clear();
	bayer_change = -1;
	imread_option = -1;
	checked_frame_pos = -1;
}

bool VideoCaptureImageList::grab(){
	if(static_cast<size_t>(index) >= img_list.size()){
		release();
		return false;
	}
	checked_frame_pos = index;
	index++;
//	std::cerr << checked_frame_pos << ": " << _forbidFake << " " << !(img_list[checked_frame_pos].second) << std::endl;
	if(_forbidFake && isPseudoCapture()){
		// recursive grab until non-fake index comes.
		if(!grab()){
			release();
			return false;
		}
	}
	return true;
}

bool VideoCaptureImageList::retrieve(cv::Mat& image, int channel){
	if(static_cast<size_t>(checked_frame_pos) >= img_list.size() || checked_frame_pos < 0){
		image.release();
		return false;
	}
	else if(bayer_change<=0){
		image = cv::imread(getImage(checked_frame_pos),imread_option);
	}
	else{
		cv::Mat bayer = cv::imread(getImage(checked_frame_pos),-1);
		if(bayer.channels()!=1){
			bayer.copyTo(image);
		}
		else{
			skl::cvtBayer2BGR(bayer,image,bayer_change,BAYER_EDGE_SENSE);
		}
	}
	return true;
}

bool VideoCaptureImageList::set(capture_property_t prop_id,double val){
	switch(prop_id){
		case SKIP_FAKE_CAPTURE:
			_forbidFake = val>0.0;
			return true;
		case POS_FRAMES:
			index = static_cast<int>(val);
			if(static_cast<size_t>(index) < img_list.size()){
				checked_frame_pos = index;
				return true;
			}
			else{
				return false;
			}
		case CONVERT_RGB:
			if(46 <= val && val < 50){
				bayer_change = static_cast<int>(val);
			}
			else if(val < 0){
				bayer_change = -1;
			}
			else{
				bayer_change = 0;
				imread_option = 1; // グレースケール画像でも3chで読み込む
			}
			return true;
		case MONOCROME:
			if(val>1){
				imread_option = 0;
				bayer_change = -1;
			}
			return true;
		default:
			return false;
	}
}

double VideoCaptureImageList::get(capture_property_t prop_id){
	switch(prop_id){
		case POS_FRAMES:
			return checked_frame_pos;
		case FRAME_COUNT:
			return (double)img_list.size();
		case MONOCROME:
			if(imread_option==0){
				return 1.0;
			}
			else{
				return -1.0;
			}
		case CONVERT_RGB:
			return bayer_change;
		default:
			return 0.0;
	}
}
