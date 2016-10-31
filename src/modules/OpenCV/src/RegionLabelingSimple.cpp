#include "RegionLabelingSimple.h"
using namespace skl;
/** 
 * @brief 奈良先端大、井村先生のプログラム（激速）を利用したラベリング
 * 背景色は(0,0,0)でないといけない
 * */
RegionLabelingSimple::RegionLabelingSimple(int threshold):threshold(threshold){}

RegionLabelingSimple::~RegionLabelingSimple(){}

size_t RegionLabelingSimple::compute(
		const cv::Mat& img,
		const cv::Mat& _mask,
		cv::Mat& labels){
	cv::Mat mask;
	assert(CV_8UC1 == _mask.type());
	if(_mask.isContinuous()){
		mask = _mask;
	}
	else{
		mask = _mask.clone();
	}
	unsigned char* pmask = mask.ptr<unsigned char>(0);

	labels = cv::Mat::zeros(mask.rows,mask.cols,CV_16SC1);
	assert(labels.isContinuous());
	short* plabel = labels.ptr<short>(0);
	labeling.Exec(pmask,plabel,mask.cols,mask.rows,true,threshold);
//	std::cerr << labeling.GetNumOfResultRegions() << std::endl;
	return labeling.GetNumOfResultRegions();

}

