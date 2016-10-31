#ifndef __REGION_LABELING_SIMPLE_H__
#define __REGION_LABELING_SIMPLE_H__

#include "FilterMat2Mat.h"
#include "Labeling.h"

namespace skl{

	class RegionLabelingSimple : public FilterMat2Mat<size_t>{
		public:
			RegionLabelingSimple(int threshold=30);
			~RegionLabelingSimple();
			inline void setThreshold(int threshold){this->threshold = threshold;}
			size_t compute(const cv::Mat& img, const cv::Mat& mask, cv::Mat& labels);
		protected:
			int threshold;
			LabelingBS labeling;
	};
}
#endif // __REGION_LABELING_SIMPLE_H__
