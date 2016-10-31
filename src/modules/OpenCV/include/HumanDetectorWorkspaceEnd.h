#ifndef __HUMAN_DETECTOR_WORKSPACE_END_H__
#define __HUMAN_DETECTOR_WORKSPACE_END_H__

#include <cv.h>
#include <list>
#include "FilterMat2Mat.h"

#ifdef _DEBUG
#define DEBUG_HUMAN_DETECTOR_WORKSPACE_END
#endif

namespace skl{
	class HumanDetectorWorkspaceEnd : public FilterMat2Mat<std::list<size_t> > {
		public:
			HumanDetectorWorkspaceEnd();
			HumanDetectorWorkspaceEnd(const cv::Mat& workspace_end);
			~HumanDetectorWorkspaceEnd();

			void setWorkspaceEnd(const cv::Mat& workspace_end);
			std::list<size_t> compute(const cv::Mat& src, const cv::Mat& mask, cv::Mat& human_region);
		protected:
			cv::Mat workspace_end;
	};
}
#endif // __HUMAN_DETECTOR_WORKSPACE_END_H__
