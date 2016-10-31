/*!
 * @file Graphcut.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jun/25
 * @date Last Change: 2012/Jun/25.
 */

#include "Graphcut.h"
using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
Graphcut::Graphcut():graph(NULL){

}

/*!
 * @brief デストラクタ
 */
Graphcut::~Graphcut(){

}

int Graphcut::compute(
		const cv::Mat& terminals,
		const cv::Mat& leftTransp,
		const cv::Mat& rightTransp,
		const cv::Mat& top,
		const cv::Mat& bottom,
		cv::Mat& dest){
	if(terminals.size()!=graph_size){
		setGraphSize(terminals.size());
	}

	if(graph!=NULL){
		delete graph;
	}
	graph = new Graph<int,int,int>((int)graph_node_num,(int)graph_node_num * 2 - graph_size.width - graph_size.height);


	for(int x=0;x<graph_size.width;x++){
		nodes[0][x] = graph->add_node();
	}

	for(int y=0;y<graph_size.height-1;y++){
		for(int x=0;x<graph_size.width;x++){
			nodes[y+1][x] = graph->add_node();
			setCapacity(graph,nodes,x,y,
					terminals,
					leftTransp,
					rightTransp,
					top,
					bottom);
		}
	}
	for(int x=0;x<graph_size.width;x++){
		setCapacity(graph,nodes,x,graph_size.height-1,
					terminals,
					leftTransp,
					rightTransp,
					top,
					bottom);
	}
	int total_cost = graph->maxflow();

	if(dest.size()!=graph_size){
		dest = cv::Mat::zeros(graph_size,CV_8UC1);
	}
	else{
		dest = cv::Scalar(0);
	}

	for(int y=0;y<graph_size.height;y++){
		for(int x=0;x<graph_size.width;x++){
			if(graph->what_segment(nodes[y][x],Graph<int,int,int>::SOURCE)){
				dest.at<unsigned char>(y,x) = 255;
			}
		}
	}
	return total_cost;
}

void Graphcut::setGraphSize(const cv::Size& size){
	graph_size = size;
	graph_node_num = size.width * size.height;
	nodes.resize(
			size.height,
			std::vector<Graph<int,int,int>::node_id>(size.width));
}

void Graphcut::setCapacity(
				Graph<int,int,int>* graph,
				const std::vector<std::vector<Graph<int,int,int>::node_id > >& nodes,
				int x, int y,
				const cv::Mat& terminals,
				const cv::Mat& leftTransp,
				const cv::Mat& rightTransp,
				const cv::Mat& top,
				const cv::Mat& bottom){
	int terminal = terminals.at<int>(y,x);
	int data_term_fg = std::max(0,terminal);
	int data_term_bg = std::max(0,-terminal);
	graph->add_tweights(
			nodes[y][x],
			data_term_bg,
			data_term_fg);
	if(x!=graph_size.width-1){
		graph->add_edge(
				nodes[y][x],
				nodes[y][x+1],
				rightTransp.at<int>(x,y),
				leftTransp.at<int>(x+1,y));
	}
	if(y!=graph_size.height-1){
		graph->add_edge(
				nodes[y][x],
				nodes[y+1][x],
				bottom.at<int>(y,x),
				top.at<int>(y+1,x));
	}
}

