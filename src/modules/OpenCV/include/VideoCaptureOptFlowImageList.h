/*!
 * @file VideoCaptureOptFlowImageList.h
 * @author yamamoto
 * @date Date Created: 2012/May/25
 * @date Last Change:2012/Jul/06.
 */
#ifndef __SKL_VIDEO_CAPTURE_OPT_FLOW_IMAGE_LIST_H__
#define __SKL_VIDEO_CAPTURE_OPT_FLOW_IMAGE_LIST_H__

#include "skl.h"
#include "sklcv.h"


namespace skl{

/*!
 * @class VideoCaptureOptFlowImageList
 * @brief オプティカルフローとして得られた32FC1の行列からなるImageListを読み出すための関数
 */
 class VideoCaptureOptFlowImageList: public VideoCaptureImageList{

	public:
		VideoCaptureOptFlowImageList();
		virtual ~VideoCaptureOptFlowImageList();
		bool open(const std::string& filename);
		bool grab();
		bool retrieve(cv::Mat& image, int channel=1);
	protected:
		Flow flow;
		bool hasFlow;
	private:
		
};

} // skl

#endif // __SKL_VIDEO_CAPTURE_OPT_FLOW_IMAGE_LIST_H__

