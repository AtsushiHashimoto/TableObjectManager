#include "StaticRegionDetector.h"
#include <vector>

#ifdef DEBUG_STATIC_REGION_DETECTOR
#include <highgui.h>
#include "sklcvutils.h"
#endif

using namespace skl;

StaticRegionDetector::StaticRegionDetector(double thresh,size_t life_time){
	setParam(thresh,life_time);
#ifdef DEBUG_STATIC_REGION_DETECTOR
	cv::namedWindow("static_region",0);
	cv::namedWindow("dynamic_region",0);
#endif
}

StaticRegionDetector::~StaticRegionDetector(){
#ifdef DEBUG_STATIC_REGION_DETECTOR
	cv::destroyWindow("static_region");
	cv::destroyWindow("dynamic_region");
#endif
}

void StaticRegionDetector::clear(){
	prev_labels.release();
	prev_object_areas.clear();
	object_life_map.clear();
}

void StaticRegionDetector::setParam(double thresh, size_t life_time){
	this->life_time = life_time;
	this->thresh = thresh;
}

size_t StaticRegionDetector::compute(
		const cv::Mat& region_labels,
		const cv::Mat& mask,
		cv::Mat& object_labels){
	assert(region_labels.size()==mask.size());
	if(prev_labels.size() != region_labels.size()){
		prev_labels = cv::Mat::zeros(region_labels.size(),CV_16SC1);
	}

	assert(object_labels.size()==region_labels.size());
	object_labels = cv::Scalar(0);

	std::vector<std::vector<size_t> > cross_area_mat;
	std::vector<size_t> current_object_areas;
	bool has_prev_object = !object_life_map.empty();
	for(int y=0;y<region_labels.rows;y++){
		for(int x=0;x<region_labels.cols;x++){
			if(mask.at<unsigned char>(y,x)==0) continue;
			short id = region_labels.at<short>(y,x);

			if( id == 0 ) continue;
			if( current_object_areas.size() <= static_cast<size_t>(id) ){
				current_object_areas.resize(id+1,0);
				if(has_prev_object){
					cross_area_mat.resize(id+1,
							std::vector<size_t>(prev_object_areas.size(),0));
				}
			}
			current_object_areas[id]++;
			if(!has_prev_object)continue;
			short prev_id = prev_labels.at<short>(y,x);
			if(prev_id == 0) continue;
			cross_area_mat[id][prev_id]++;
		}
	}
	// reset prev_labels
	prev_labels = cv::Scalar(0);
	size_t region_num = current_object_areas.size();

	std::vector<bool> is_static_object = update_object_life_map(current_object_areas,prev_object_areas,cross_area_mat);

	std::vector<short> id2static_object_id(region_num,0),id2dynamic_object_id(region_num,0);
	short sid(1),did(1);

	std::map<size_t,size_t> temp_life_map(object_life_map);
	object_life_map.clear();
	prev_object_areas.clear();

	for(size_t i=1;i<is_static_object.size();i++){
		if(is_static_object[i]){
			id2static_object_id[i] = sid;
			sid++;
		}
		else{
			id2dynamic_object_id[i] = did;
			object_life_map[did] = temp_life_map[i];
			prev_object_areas.resize(did+1,0);
			prev_object_areas[did] = current_object_areas[i];
			did++;
		}
	}
	for(int y=0;y<region_labels.rows;y++){
		for(int x=0;x<region_labels.cols;x++){
			short id = region_labels.at<short>(y,x);
			if(id==0) continue;
			if(0==mask.at<unsigned char>(y,x)) continue;
			if(is_static_object[id]){
				object_labels.at<short>(y,x) = static_cast<short>(id2static_object_id[id]);
			}
			else{
				prev_labels.at<short>(y,x) = static_cast<short>(id2dynamic_object_id[id]);
			}
		}
	}
#ifdef DEBUG_STATIC_REGION_DETECTOR
	cv::imshow("static_region",visualizeRegionLabel(object_labels,sid-1));
	cv::imshow("dynamic_region",visualizeRegionLabel(prev_labels,did-1));
#endif

	return sid-1;
}

std::vector<bool> StaticRegionDetector::update_object_life_map(
		const std::vector<size_t>& current_object_areas,
		const std::vector<size_t>& prev_object_areas,
		std::vector<std::vector<size_t> >& cross_area_mat){
	std::vector<bool> is_static_object(current_object_areas.size(),false);
	std::map<size_t,size_t> temp_life_map(object_life_map);
	object_life_map.clear();

	for(size_t cid = 1; cid < current_object_areas.size();cid++){
		size_t best_match = UINT_MAX;
		double best_score = -DBL_MAX;
		for(size_t pid = 1; pid < prev_object_areas.size(); pid++){
			double score = calcScore(
					current_object_areas[cid],
					prev_object_areas[pid],
					static_cast<double>(cross_area_mat[cid][pid]));
			if(best_score < score){
				best_score = score;
				best_match = pid;
			}
		}
		if(best_score<thresh){
			object_life_map[cid] = life_time;
			continue;
		}
		std::map<size_t,size_t>::iterator pp = temp_life_map.find(best_match);
		assert(temp_life_map.end() != pp);
		if(pp->second <= 1){
			is_static_object[cid] = true;
			continue;
		}
		object_life_map[cid] = pp->second -1;
	}
	return is_static_object;
}

double StaticRegionDetector::calcScore(double area1, double area2, double cross_area)const{
	return 2.0*cross_area/(area1+area2);
}
