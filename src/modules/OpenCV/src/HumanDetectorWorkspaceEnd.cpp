#include "HumanDetectorWorkspaceEnd.h"

using namespace skl;

HumanDetectorWorkspaceEnd::HumanDetectorWorkspaceEnd(){}
HumanDetectorWorkspaceEnd::HumanDetectorWorkspaceEnd(const cv::Mat& workspace_end){
	setWorkspaceEnd(workspace_end);
}

HumanDetectorWorkspaceEnd::~HumanDetectorWorkspaceEnd(){}

void HumanDetectorWorkspaceEnd::setWorkspaceEnd(const cv::Mat& workspace_end){
	assert(CV_8UC1 == workspace_end.type());
	this->workspace_end = workspace_end;
}

std::list<size_t> HumanDetectorWorkspaceEnd::compute(
		const cv::Mat& src,
		const cv::Mat& mask,
		cv::Mat& human){
//	assert(mask.size()==workspace_end.size());
	human = cv::Mat::zeros(mask.size(),CV_8UC1);
	std::vector<bool> is_human(1,false);

	for(int y=0;y<mask.rows;y++){
		for(int x=0;x<mask.cols;x++){
			short label = mask.at<short>(y,x);
			if(label==0) continue;
			if(static_cast<short>(is_human.size()) <= label) is_human.resize(label+1,false);
			if(workspace_end.at<unsigned char>(y,x)==0) continue;
			is_human[label] = true;
		}
	}

	std::list<size_t> human_regions;

	for(size_t i=0;i<is_human.size();i++){
		if(!is_human[i])continue;
		human_regions.push_back(i);
	}
	if(human_regions.empty()) return human_regions;

	for(int y=0;y<mask.rows;y++){
		for(int x=0;x<mask.cols;x++){
			short label = mask.at<short>(y,x);
			if(!is_human[label]) continue;
			human.at<unsigned char>(y,x) = 255;
		}
	}

	return human_regions;
}
