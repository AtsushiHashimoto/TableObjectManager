/*!
 * @file Graphcut.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jun/25
 * @date Last Change:2012/Jun/25.
 */
#ifndef __SKL_GRAPHCUT_H__
#define __SKL_GRAPHCUT_H__

#include "skl.h"
#include <cv.h>
#include <highgui.h>
#include <vector>

namespace skl{

/*!
 * @class Graphcut
 * @brief Wrapper Class of graphcut by Vladimir Komogorov.
 */
class Graphcut{

	public:
		Graphcut();
		virtual ~Graphcut();
		int compute(const cv::Mat& terminals,
				const cv::Mat& leftTransp,
				const cv::Mat& rightTransp,
				const cv::Mat& top,
				const cv::Mat& bottom,
				cv::Mat& dest);
	protected:
		Graph<int,int,int>* graph;
		std::vector<std::vector<Graph<int,int,int>::node_id> > nodes;
		cv::Size graph_size;
		size_t graph_node_num;
	private:
		virtual void setGraphSize(const cv::Size& size);
		virtual void setCapacity(
				Graph<int,int,int>* graph,
				const std::vector<std::vector<Graph<int,int,int>::node_id > >& nodes,
				int x, int y,
				const cv::Mat& terminals,
				const cv::Mat& leftTransp,
				const cv::Mat& rightTransp,
				const cv::Mat& top,
				const cv::Mat& bottom);
};

} // skl

#endif // __SKL_GRAPHCUT_H__

