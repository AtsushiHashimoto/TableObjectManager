#ifndef __STATIC_REGION_DETECTOR_H__
#define __STATIC_REGION_DETECTOR_H__

#include <map>
#include "FilterMat2Mat.h"

#ifdef _DEBUG
//#define DEBUG_STATIC_REGION_DETECTOR
#endif

namespace skl{

	class StaticRegionDetector: public FilterMat2Mat<size_t>{
		public:
			StaticRegionDetector(double thresh=0.95,size_t life_time=1);
			~StaticRegionDetector();
			void setParam(double thresh,size_t life_time=1);
			size_t compute(const cv::Mat& region_labels, const cv::Mat& mask, cv::Mat& object_labels);
			void clear();
		protected:
			double thresh;
			int life_time;

			cv::Mat prev_labels;
			std::vector<size_t> prev_object_areas;
			std::map<size_t,size_t> object_life_map;

			std::vector<bool> update_object_life_map(
					const std::vector<size_t>& current_object_areas,
					const std::vector<size_t>& prev_object_areas,
					std::vector<std::vector<size_t> >& cross_area_mat);
			double calcScore(double area1, double area2, double cross_area)const;
	};
}
#endif // __STATIC_REGION_DETECTOR_H__
