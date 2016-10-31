#include "TexCut.h"
#include <iostream>
#include <algorithm>


using namespace skl;
TexCut::TexCut(float alpha, float smoothing_term_weight,float thresh_tex_diff,unsigned char over_exposure_thresh,unsigned char under_exposure_thresh,bool doSmoothing):
	_doSmoothing(doSmoothing),
	noise_std_dev(3,3.5),
	gh_expectation(3,2.3),
	gh_std_dev(3,1.12),
	g(NULL){
	setParams(alpha, smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh);
#ifdef DEBUG_TEXCUT
	cv::namedWindow("data_term",0);
	cv::namedWindow("tex_int",0);
	cv::namedWindow("gradient_heterogenuity",0);
	cv::namedWindow("smoothing_term_x",0);
	cv::namedWindow("smoothing_term_y",0);
#endif
}

TexCut::TexCut(const cv::Mat& bg1, const cv::Mat& bg2, float alpha, float smoothing_term_weight,float thresh_tex_diff,unsigned char over_exposure_thresh,unsigned char under_exposure_thresh):g(NULL){
	setBackground(bg1);
	learnImageNoiseModel(bg2);
	setParams(alpha, smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh);
#ifdef DEBUG_TEXCUT
	cv::namedWindow("data_term",0);
	cv::namedWindow("tex_int",0);
	cv::namedWindow("gradient_heterogenuity",0);
	cv::namedWindow("smoothing_term_x",0);
	cv::namedWindow("smoothing_term_y",0);
#endif
}

TexCut::~TexCut(){
#ifdef DEBUG_TEXCUT
	cv::destroyWindow("data_term");
	cv::destroyWindow("tex_int");
	cv::destroyWindow("gradient_heterogenuity");
	cv::destroyWindow("smoothing_term_x");
	cv::destroyWindow("smoothing_term_y");
#endif

}

void TexCut::setParams(float alpha, float smoothing_term_weight, float thresh_tex_diff, unsigned char over_exposure_thresh, unsigned char under_exposure_thresh){
	this->alpha = alpha;
	this->smoothing_term_weight = smoothing_term_weight;
	this->thresh_tex_diff = thresh_tex_diff;
	this->over_exposure_thresh = over_exposure_thresh;
	this->under_exposure_thresh = under_exposure_thresh;
}

void TexCut::compute(const cv::Mat& __src,const cv::Mat& mask,cv::Mat& dest){
	// compute edge capacity and construct graph model
	// mask is not used.
	cv::Mat _src;
	if(_doSmoothing){
		cv::blur(__src,_src,cv::Size(3,3));
	}
	else{
		_src = __src;
	}
	
	std::vector<cv::Mat> src;
	if(_src.channels() == 3){
		cv::split(_src, src);
	}
	else if(_src.channels() == 1){
		src.push_back(_src);
	}
	else{
		bool isValidImageChannel = false;
		assert(isValidImageChannel);
	}
	assert(!bg_img.empty());
	assert(!nodes.empty());

	std::vector<cv::Mat> sobel_x,sobel_y;
	getSobel(src,&sobel_x,&sobel_y);

	cv::Mat data_term,smoothing_term_x,smoothing_term_y;
	calcEdgeCapacity(
			src,sobel_x,sobel_y,
			bg_img,bg_sobel_x,bg_sobel_y,
			noise_std_dev,
			gh_expectation,
			gh_std_dev,
			alpha,
			smoothing_term_weight,
			thresh_tex_diff,
			data_term,
			smoothing_term_x,
			smoothing_term_y);
/*	cv::Mat temp;
	data_term.convertTo(temp,CV_8UC1,255.0/QUANTIZATION_LEVEL);
	cv::imwrite("data_term_with_gh.png",temp);
	smoothing_term_x.convertTo(temp,CV_8UC1,128.0/QUANTIZATION_LEVEL);
	cv::imwrite("smoothing_term_x_with_gh.png",temp);
	smoothing_term_y.convertTo(temp,CV_8UC1,128.0/QUANTIZATION_LEVEL);
	cv::imwrite("smoothing_term_y_with_gh.png",temp);
*/	
	/*int flow = */calcGraphCut(data_term,smoothing_term_x,smoothing_term_y);

	setResult(_src,dest);
	return;// static_cast<double>(flow);
}

void TexCut::setBackground(const cv::Mat& ___bg,bool noSmoothing){
	if(_doSmoothing && !noSmoothing){
		cv::blur(___bg,_background,cv::Size(3,3));
	}
	else{
		_background = ___bg.clone();
	}
	if(_background.channels()==3){
		cv::split(_background,this->bg_img);
	}
	else{
		this->bg_img.push_back(_background);
	}


	getSobel(bg_img,&bg_sobel_x,&bg_sobel_y);

	int graph_width = ___bg.cols / TEXCUT_BLOCK_SIZE;
	int graph_height = ___bg.rows / TEXCUT_BLOCK_SIZE;
	nodes.assign(
			graph_height, std::vector<TexCutGraph::node_id>(graph_width,0));
}

void TexCut::learnImageNoiseModel(const cv::Mat& ___bg2){
	assert(!bg_img.empty());
	size_t channels = bg_img.size();
	std::vector<cv::Mat> bg_img2;
	cv::Mat bg2;
	if(_doSmoothing){
		cv::blur(___bg2,bg2,cv::Size(3,3));
	}
	else{
		bg2 = ___bg2;
	}
	if(bg2.channels()==3){
		cv::split(bg2,bg_img2);
	}
	else{
		bg_img2.push_back(bg2);
	}


	assert(bg_img.size()==bg_img2.size());
	assert(bg_img[0].rows == bg_img2[0].rows);
	assert(bg_img[0].cols == bg_img2[0].cols);
	assert(bg_img[0].type() == bg_img2[0].type());

	noise_std_dev.assign(channels,0);
	gh_expectation.assign(channels,0);
	gh_std_dev.assign(channels,0);
	cv::parallel_for(
			cv::BlockedRange(0,(int)channels),
			ParallelNoiseEstimate(
				&bg_img,
				&bg_img2,
				&noise_std_dev,
				&gh_expectation,
				&gh_std_dev
				)
			);

// debug
#ifdef DEBUG_TEXCUT
	std::cerr << "Noise SD: ";
	for(size_t i=0;i<channels;i++){
		if(i!=0) std::cerr << ",";
		std::cerr << noise_std_dev[i];
	}
	std::cerr << std::endl;

	std::cerr << "GH Expec: ";
	for(size_t i=0;i<channels;i++){
		if(i!=0) std::cerr << ",";
		std::cerr << gh_expectation[i];
	}
	std::cerr << std::endl;

	std::cerr << "GH SD   : ";
	for(size_t i=0;i<channels;i++){
		if(i!=0) std::cerr << ",";
		std::cerr << gh_std_dev[i];
	}
	std::cerr << std::endl;
#endif //DEBUG

}

void TexCut::setNoiseModel(
		const std::vector<float>& noise_std_dev,
		const std::vector<float>& gh_expectation,
		const std::vector<float>& gh_std_dev){
	assert(noise_std_dev.size()==1 || noise_std_dev.size()==3);
	assert(noise_std_dev.size()==gh_expectation.size());
	assert(noise_std_dev.size()==gh_std_dev.size());
	this->noise_std_dev = noise_std_dev;
	this->gh_expectation = gh_expectation;
	this->gh_std_dev = gh_std_dev;
}

void TexCut::getSobel(
		const std::vector<cv::Mat>& img,
		std::vector<cv::Mat>* sobel_x,
		std::vector<cv::Mat>* sobel_y){
	size_t channels = img.size();
	sobel_x->resize(channels);
	sobel_y->resize(channels);
	for(size_t c=0;c<channels;c++){
		cv::Mat temp = cv::Mat::zeros(img[c].rows,img[c].cols,CV_16SC1);
		sobel_x->at(c) = cv::Mat::zeros(img[c].rows,img[c].cols,CV_8UC1);
		sobel_y->at(c) = sobel_x->at(c).clone();

		CvMat bg = img[c];
		CvMat stemp = temp;

		cvSobel(&bg,&stemp,1,0,3);
		CvMat sobel = sobel_x->at(c);
		cvConvertScaleAbs(&stemp,&sobel,1,0);

		cvSobel(&bg,&stemp,0,1,3);
		sobel = sobel_y->at(c);
		cvConvertScaleAbs(&stemp,&sobel,1,0);

/*
		cv::imshow("bgsobelx",bg_sobel_x[c]);
		cv::imshow("bgsobely",bg_sobel_y[c]);
		cv::waitKey(-1);
*/
	}
}

void TexCut::setResult(const cv::Mat& src,cv::Mat& dest)const{
	int graph_width = src.cols/TEXCUT_BLOCK_SIZE;
	int graph_height = src.rows/TEXCUT_BLOCK_SIZE;
	dest = cv::Mat::zeros(graph_height,graph_width,CV_8UC1);
	for(int y=0;y<dest.rows;y++){
		for(int x=0;x<dest.cols;x++){
			if(g->what_segment(nodes[y][x],TexCutGraph::SOURCE)){
				dest.at<unsigned char>(y,x) = 255;
			}
		}
	}
}

void TexCut::calcEdgeCapacity(
		const std::vector<cv::Mat>& src,
		const std::vector<cv::Mat>& sobel_x,
		const std::vector<cv::Mat>& sobel_y,
		const std::vector<cv::Mat>& bg_img,
		const std::vector<cv::Mat>& bg_sobel_x,
		const std::vector<cv::Mat>& bg_sobel_y,
		const std::vector<float>& noise_std_dev,
		const std::vector<float>& gh_expectation,
		const std::vector<float>& gh_std_dev,
		float alpha,
		float smoothing_term_weight,
		float thresh_tex_diff,
		cv::Mat& data_term,
		cv::Mat& smoothing_term_x,
		cv::Mat& smoothing_term_y){

	// * smoothing_term_weight
	cv::Size graph_size;
	graph_size.width = src[0].cols / TEXCUT_BLOCK_SIZE;
	graph_size.height = src[0].rows / TEXCUT_BLOCK_SIZE;
	int graph_node_size = graph_size.width * graph_size.height;

	data_term = cv::Mat::zeros(graph_size,CV_32SC1);
	smoothing_term_x = cv::Mat::zeros(graph_size,CV_32SC1);
	smoothing_term_y = cv::Mat::zeros(graph_size,CV_32SC1);
#ifdef DEBUG_TEXCUT
	cv::Mat tex_int = cv::Mat::zeros(graph_size,CV_32FC1);
#endif
	cv::Mat gradient_heterogenuity = cv::Mat::zeros(graph_size,CV_32FC1);

	cv::parallel_for(
			cv::BlockedRange(0,graph_node_size),
			ParallelCalcEdgeCapacity(
				src,sobel_x,sobel_y,
				bg_img,bg_sobel_x,bg_sobel_y,
				noise_std_dev,
				gh_expectation,
				gh_std_dev,
				alpha,
				thresh_tex_diff,
				over_exposure_thresh,
				under_exposure_thresh,
				data_term,
#ifdef DEBUG_TEXCUT
				tex_int,
#endif
				gradient_heterogenuity,
				smoothing_term_x,
				smoothing_term_y
				)
			);

	cv::parallel_for(
			cv::BlockedRange(0,graph_node_size),
			ParallelAddGradientHeterogenuity(
					data_term,
					gradient_heterogenuity,
					smoothing_term_x,
					smoothing_term_y,
					smoothing_term_weight
				)
			);

#ifdef DEBUG_TEXCUT
	cv::namedWindow("data_term",0);
	cv::namedWindow("tex_int",0);
	cv::namedWindow("gradient_heterogenuity",0);
	cv::namedWindow("smoothing_term_x",0);
	cv::namedWindow("smoothing_term_y",0);
	cv::imshow("data_term",data_term);
	cv::imshow("smoothing_term_x",smoothing_term_x);
	cv::imshow("smoothing_term_y",smoothing_term_y);
	cv::imshow("tex_int",tex_int);
	cv::imshow("gradient_heterogenuity",gradient_heterogenuity);
//	cv::waitKey(-1);
/*	cv::Mat temp;
	tex_int.convertTo(temp,CV_8UC1,255);
	cv::imwrite("tex_int_with_gh.png",temp);
*/
#endif
}

int TexCut::calcGraphCut(const cv::Mat& data_term,const cv::Mat& smoothing_term_x, const cv::Mat& smoothing_term_y){
	if(g!=NULL){
		delete g;
	}
	size_t graph_width = nodes[0].size();
	size_t graph_height = nodes.size();
	size_t graph_size = graph_width * graph_height;
	g = new TexCutGraph((int)graph_size,(int)graph_size * 2 - graph_width - graph_height );
	for(size_t x=0;x<graph_width;x++){
		nodes[0][x] = g->add_node();
	}
	
	for(size_t y=0;y<graph_height-1;y++){
		for(size_t x=0;x<graph_width;x++){
			nodes[y+1][x] = g->add_node();
			setCapacity(g,nodes,x,y,
					data_term,smoothing_term_x,smoothing_term_y);
		}
	}
	for(size_t x=0;x<graph_width;x++){
		setCapacity(g,nodes,x,graph_height-1,
				data_term,smoothing_term_x,smoothing_term_y);
	}

	return g->maxflow();
}

void TexCut::setCapacity(
		TexCutGraph* g,std::vector<std::vector<TexCutGraph::node_id> >& nodes, size_t x,size_t y, const cv::Mat& data_term,const cv::Mat& smoothing_term_x, const cv::Mat& smoothing_term_y){
	int _data_term = data_term.at<int>((int)y,(int)x);
	g->add_tweights(
			nodes[y][x],
			QUANTIZATION_LEVEL - _data_term,
			_data_term);
	if(x!=nodes[0].size()-1){
		int _smoothing_term_x = static_cast<int>(smoothing_term_weight * smoothing_term_x.at<int>((int)y,(int)x));
		if(_smoothing_term_x < 0){
			std::cerr << smoothing_term_x.at<int>((int)y,(int)x) << std::endl;
			std::cerr << smoothing_term_weight << std::endl;
			std::cerr << _smoothing_term_x << std::endl;
			std::cerr << QUANTIZATION_LEVEL << std::endl;
		}
//		std::cerr << x << ", " << y << ": " << _smoothing_term_x << std::endl;
		g->add_edge(
				nodes[y][x],
				nodes[y][x+1],//QUANTIZATION_LEVEL*2, QUANTIZATION_LEVEL*2);
				_smoothing_term_x,
				_smoothing_term_x);
	}
	if(y!=nodes.size()-1){
		int _smoothing_term_y = static_cast<int>(smoothing_term_weight * smoothing_term_y.at<int>((int)y,(int)x));
		g->add_edge(
				nodes[y][x],
				nodes[y+1][x],//QUANTIZATION_LEVEL*2,QUANTIZATION_LEVEL*2);
				_smoothing_term_y,
				_smoothing_term_y);
	}
}

void TexCut::updateBackgroundModel(const cv::Mat& img){
	setBackground(img,true);
}

/****** Definitions for Parallel Processing ******/
void ParallelCalcEdgeCapacity::operator()(const cv::BlockedRange& range)const{
	for(int i=range.begin();i<range.end();i++){
		int graph_width = data_term.cols;
		int graph_height = data_term.rows;
		int gx = i % graph_width;
		int gy = i / graph_width;
		cv::Rect roi;
		roi.x = gx * TEXCUT_BLOCK_SIZE;
		roi.y = gy * TEXCUT_BLOCK_SIZE;
		roi.width = TEXCUT_BLOCK_SIZE;
		roi.height = TEXCUT_BLOCK_SIZE;

		bool ignore_data_term = false;
		if(isOverUnderExposure(src,roi) || isOverUnderExposure(bg_img,roi)){
			data_term.at<int>(gy,gx) = QUANTIZATION_LEVEL_HARF;
#ifdef DEBUG_TEXCUT
			tex_int.at<float>(gy,gx) = 0.0f;
#endif
			gradient_heterogenuity.at<float>(gy,gx) = 0.0f;
			ignore_data_term = true;
		}

		int channels = (int)src.size();
		std::vector<float> t_i(channels, 0);
		std::vector<float> d_t(channels, 0);
		std::vector<float> s_x(channels, FLT_MAX);
		std::vector<float> s_y(channels, FLT_MAX);
		std::vector<float> g_h(channels,0);
		for(int c=0;c<channels;c++){
			cv::Mat _src = cv::Mat(src[c],roi);
			cv::Mat _sobel_x = cv::Mat(sobel_x[c],roi);
			cv::Mat _sobel_y = cv::Mat(sobel_y[c],roi);
			cv::Mat _bg = cv::Mat(bg_img[c],roi);
			cv::Mat _bg_sobel_x = cv::Mat(bg_sobel_x[c],roi);
			cv::Mat _bg_sobel_y = cv::Mat(bg_sobel_y[c],roi);


			if(!ignore_data_term){
				calcDataTerm(_sobel_x,_sobel_y,_bg_sobel_x,_bg_sobel_y,
						noise_std_dev[c], gh_expectation[c], gh_std_dev[c],
						&t_i[c],&g_h[c],&d_t[c]);
			}
			if(gx < graph_width -1){
				cv::Rect right = roi;
				right.x += TEXCUT_BLOCK_SIZE;
				cv::Mat _src_right = cv::Mat(src[c],right);
				cv::Mat _bg_right = cv::Mat(bg_img[c],right);
				calcSmoothingTerm(_src,_src_right,_bg,_bg_right,&s_x[c], noise_std_dev[c]);
			}

			if(gy < graph_height -1){
				_src = _src.clone();
				_src.t();
				_bg = _bg.clone();
				_bg.t();
				cv::Rect bottom = roi;
				bottom.y += TEXCUT_BLOCK_SIZE;
				cv::Mat _src_bottom = cv::Mat(src[c],bottom).clone();
				_src_bottom.t();
				cv::Mat _bg_bottom = cv::Mat(bg_img[c],bottom).clone();
				_bg_bottom.t();
				calcSmoothingTerm(_src,_src_bottom,_bg,_bg_bottom,&s_y[c],noise_std_dev[c]);
			}
		}
		if(!ignore_data_term){
			int argmax_tex = 0;
			for(int c = 1; c<channels;c++){
				argmax_tex = t_i[argmax_tex] > t_i[c] ? argmax_tex : c;
			}

#ifdef DEBUG_TEXCUT
			tex_int.at<float>(gy,gx) = t_i[argmax_tex];
#endif
			gradient_heterogenuity.at<float>(gy,gx) = g_h[argmax_tex];

			assert(0 <= d_t[argmax_tex]);
			if(1.0f < d_t[argmax_tex]){
				d_t[argmax_tex] = 1.0f;
			}
			data_term.at<int>(gy,gx) = static_cast<int>(d_t[argmax_tex] * QUANTIZATION_LEVEL);
			//		std::cerr << d_t[argmax_tex] << ", " << data_term.at<int>(gy,gx) << std::endl;
		}

		if(gx < graph_width-1){
			float s_t = s_x[0];
			for(int c = 1; c<channels;c++){
				s_t = s_t < s_x[c] ? s_t : s_x[c];
			}
			s_t *= QUANTIZATION_LEVEL;
			s_t = s_t < QUANTIZATION_LEVEL ? s_t : QUANTIZATION_LEVEL;
			smoothing_term_x.at<int>(gy,gx) = static_cast<int>(s_t);
		}

		if(gy < graph_height-1){
			float s_t = s_y[0];
			for(int c = 1; c<channels;c++){
				s_t = s_t < s_y[c] ? s_t : s_y[c];
			}
			s_t *= QUANTIZATION_LEVEL;
			s_t = s_t < QUANTIZATION_LEVEL ? s_t : QUANTIZATION_LEVEL;
			smoothing_term_y.at<int>(gy,gx) = static_cast<int>(s_t);
		}
	}
}
int ParallelCalcEdgeCapacity::isOverUnderExposure(const cv::Mat& block)const{
	int exposure_state = 0;
	int temp;
	for(int y=0;y<block.rows;y++){
		for(int x=1;x<block.cols;x++){
			temp = 0;
			unsigned char val = block.at<unsigned char>(y,x);
			if(val > over_exposure_thresh) temp = 1;
			else if(val < under_exposure_thresh) temp = -1;
			if(temp==0) return 0;
			if(exposure_state==0){
				exposure_state = temp;
			}
			else if(exposure_state!=temp){
				return 0;
			}
		}
	}
	return exposure_state;
}

bool ParallelCalcEdgeCapacity::isOverUnderExposure(const std::vector<cv::Mat>& img_planes,const cv::Rect& roi)const{
	for(size_t c=0;c<img_planes.size();c++){
		int check = isOverUnderExposure(cv::Mat(img_planes[c],roi));
		if(check == 0) return false;
	}
	return true;
}

void ParallelCalcEdgeCapacity::calcDataTerm(
		const cv::Mat& sobel_x, const cv::Mat& sobel_y,
		const cv::Mat& bg_sobel_x, const cv::Mat& bg_sobel_y,
		float nsd, float gh_mean, float gh_sd,
		float* tex_int, float* gh, float* tex_diff)const{
	float auto_cor_src(0),auto_cor_bg(0);
	float cross_cor(0);
	int square_size = TEXCUT_BLOCK_SIZE * TEXCUT_BLOCK_SIZE;
	std::vector<float> sort_tar(square_size,0),bg_sort_tar(square_size,0);
//	std::vector<float> sort_tar_x(square_size,0),sort_tar_y(square_size,0);
//	std::vector<float> bg_sort_tar_x(square_size,0),bg_sort_tar_y(square_size,0);
	for(int y=0,i=0;y<TEXCUT_BLOCK_SIZE;y++){
		for(int x=0;x<TEXCUT_BLOCK_SIZE;x++,i++){
			float ssx,ssy,bsx,bsy;
			ssx = static_cast<float>(sobel_x.at<unsigned char>(y,x));
			ssy = static_cast<float>(sobel_y.at<unsigned char>(y,x));
			bsx = static_cast<float>(bg_sobel_x.at<unsigned char>(y,x));
			bsy = static_cast<float>(bg_sobel_y.at<unsigned char>(y,x));
			sort_tar[i] = (std::pow(ssx,2) + std::pow(ssy,2));
			auto_cor_src += sort_tar[i];
			bg_sort_tar[i] = (std::pow(bsx,2) + std::pow(bsy,2));
			auto_cor_bg += bg_sort_tar[i];
			cross_cor += (ssx * bsx + ssy * bsy);

//			sort_tar_x[i] = ssx;
//			sort_tar_y[i] = ssy;
//			bg_sort_tar_x[i] = bsx;
//			bg_sort_tar_y[i] = bsy;
		}
	}

	// calc texture intenxity
	*tex_int = auto_cor_src > auto_cor_bg ? auto_cor_src : auto_cor_bg;
	*tex_int = sqrt(*tex_int/square_size);
	*tex_int = static_cast<float>(
		normalize(*tex_int,2*sqrt(3.0f) * nsd));

	// calc gradient heterogenuity
	float grad_hetero = calcGradHetero(sort_tar);
	float temp = calcGradHetero(bg_sort_tar);
	grad_hetero = grad_hetero > temp ? grad_hetero : temp;
/*	float grad_hetero = calcGradHetero(sort_tar_x);
	float temp = calcGradHetero(bg_sort_tar_x);
	grad_hetero = grad_hetero > temp ? grad_hetero : temp;
	temp = calcGradHetero(sort_tar_y);
	grad_hetero = grad_hetero > temp ? grad_hetero : temp;
	temp = calcGradHetero(bg_sort_tar_y);
	grad_hetero = grad_hetero > temp ? grad_hetero : temp;
*/
	grad_hetero = normalize(grad_hetero, 2*gh_sd, gh_mean);
	*gh = grad_hetero;

	float auto_cor = auto_cor_src + auto_cor_bg;
	if(auto_cor==0){
		*tex_int = 0;
		*tex_diff = 0.0f;
		return;
	}

	float normalized_correlation_dist = 1.0f - (2.0f * cross_cor)/auto_cor;
	assert(0 <= normalized_correlation_dist && normalized_correlation_dist <= 1.0);
	*tex_int = exp((grad_hetero * *tex_int) - 1.0f);
//	*tex_int = exp(*tex_int -1.0f);
//	*tex_int = grad_hetero * *tex_int;
	*tex_diff = (thresh_tex_diff + *tex_int * ( normalized_correlation_dist - thresh_tex_diff )) / (2 * thresh_tex_diff);
/*
	if(*tex_diff > 0.5){
		std::cerr << *tex_int << ", " << normalized_correlation_dist << std::endl;
		std::cerr << *tex_diff << std::endl;
	}
*/
}

float ParallelCalcEdgeCapacity::normalize(float val,float sigma, float mean)const{
	val -= mean + sigma;
	if(val<0) return 0;
	val /= 2 * alpha * sigma;
	if(val>1) return 1;
	return val;
}


float ParallelCalcEdgeCapacity::calcGradHetero(std::vector<float>& power){
	float max = sqrt(*std::max_element(power.begin(),power.end()));
	float factor = sqrt(*std::min_element(power.begin(),power.end()));
/*	std::vector<float>::iterator median = power.begin()+(power.size()/2);
	std::nth_element(power.begin(),median,power.end(),std::greater<float>());
	float factor = *median;
*/
	if(factor == 0.0f){
		if(max==0.0f){
			return 0;
		}
		else{
			return FLT_MAX;
		}
	}
	return max/factor;
}
void ParallelCalcEdgeCapacity::calcSmoothingTerm(
		const cv::Mat& src_left, const cv::Mat& src_right,
		const cv::Mat& bg_left, const cv::Mat& bg_right,
		float* smoothing_term, float nsd)const{
	float diff_l2r(0),diff_r2l(0);
	for(int i=0;i<TEXCUT_BLOCK_SIZE;i++){
		float s1,s2,b1,b2;
		s1 = src_left.at<unsigned char>(i,TEXCUT_BLOCK_SIZE-1);
		b1 = std::max<unsigned char>(1,bg_left.at<unsigned char>(i,TEXCUT_BLOCK_SIZE-1));
		s2 = src_right.at<unsigned char>(i,TEXCUT_BLOCK_SIZE-1);
		b2 = std::max<unsigned char>(1,bg_right.at<unsigned char>(i,TEXCUT_BLOCK_SIZE-1));
		diff_l2r += s1 - (b1 * (s2/b2));

		s1 = src_right.at<unsigned char>(i,0);
		b1 = bg_right.at<unsigned char>(i,0);
		s2 = src_left.at<unsigned char>(i,0);
		b2 = bg_left.at<unsigned char>(i,0);
		diff_r2l += s1 - (b1 * (s2/b2));
	}
	diff_l2r = std::abs(diff_l2r);
	diff_r2l = std::abs(diff_r2l);

	float diff = diff_l2r > diff_r2l ? diff_l2r : diff_r2l;
//	std::cerr << diff << " > " << (2*nsd * sqrt(2.0/TEXCUT_BLOCK_SIZE)) << std::endl;
	diff /= TEXCUT_BLOCK_SIZE;
	*smoothing_term = static_cast<float>(normalize(diff, nsd / sqrt(static_cast<float>(TEXCUT_BLOCK_SIZE))));
//	*smoothing_term = 1.0f - *smoothing_term;
	*smoothing_term = 1.0f - exp(*smoothing_term - 1.0f);
}

ParallelNoiseEstimate::ParallelNoiseEstimate(
					std::vector<cv::Mat>* img1,
					std::vector<cv::Mat>* img2,
					std::vector<float>* noise_std_dev,
					std::vector<float>* gh_expectation,
					std::vector<float>* gh_std_dev
					):img1(img1),img2(img2),noise_std_dev(noise_std_dev),gh_expectation(gh_expectation),gh_std_dev(gh_std_dev){}

void ParallelNoiseEstimate::operator()(const cv::BlockedRange& range)const{
	for(int c = range.begin(); c < range.end(); c++){
		float nsd, ghe, ghsd;
		noiseEstimate(img1->at(c),img2->at(c),&nsd);
		ghNoiseEstimate(nsd,&ghe,&ghsd);

		noise_std_dev->at(c) = nsd;
		gh_expectation->at(c) = ghe;
		gh_std_dev->at(c) = ghsd;
	}
}

void ParallelNoiseEstimate::ghNoiseEstimate(
					float noise_std_dev,
					float* gh_expectation,
					float* gh_std_dev)const{
				assert(noise_std_dev > 0);
				size_t iteration_time = GRADIENT_HETEROGENUITY;
				float moment1(0),moment2(0);
				int elem_num = TEXCUT_BLOCK_SIZE * TEXCUT_BLOCK_SIZE;
				std::vector<float> powers(elem_num,0.0);
				for(size_t i = 0; i < iteration_time; i++){
					for(int e = 0; e < elem_num; e++){
						powers[e] = static_cast<float>(rayleigh_rand(std::sqrt(6.0)*noise_std_dev));
					}
					float gh = ParallelCalcEdgeCapacity::calcGradHetero(powers);
					if(gh==0||gh==FLT_MAX){
						i--;
						continue;
					}
					moment1 += gh;
					moment2 += std::pow(gh,2);
				}
				moment1 /= iteration_time;
				moment2 /= iteration_time;
				*gh_expectation = moment1;
				*gh_std_dev = 2 * sqrt(moment2 - std::pow(moment1,2));
			}
void ParallelNoiseEstimate::noiseEstimate(
					const cv::Mat& img1,
					const cv::Mat& img2,
					float* noise_std_dev)const{
				float moment1(0),moment2(0);
				float temp1(0),temp2(0);
				for(int y = 0; y < img1.rows; y++){
					temp1 = 0;
					temp2 = 0;
					for(int x = 0; x < img1.cols; x++){
						int diff = 
							static_cast<int>(img1.at<unsigned char>(y,x))
							- img2.at<unsigned char>(y,x);
						temp1 += diff;
						temp2 += diff * diff;
					}
					moment1 += temp1/img1.cols;
					moment2 += temp2/img2.cols;
				}
				moment1 /= img1.rows;
				moment2 /= img2.rows;
				*noise_std_dev = (float)sqrt(moment2 - std::pow(moment1,2));
			}


ParallelAddGradientHeterogenuity::ParallelAddGradientHeterogenuity(
					const cv::Mat& data_term,
					const cv::Mat& gradient_heterogenuity,
					cv::Mat& smoothing_term_x,
					cv::Mat& smoothing_term_y,
					float smoothing_term_weight
					):
				data_term(data_term), gradient_heterogenuity(gradient_heterogenuity),
				smoothing_term_x(smoothing_term_x),smoothing_term_y(smoothing_term_y),smoothing_term_weight(smoothing_term_weight){}

void ParallelAddGradientHeterogenuity::operator()(const cv::BlockedRange& range)const{
	for(int i = range.begin();i != range.end(); i++){
		int graph_width = data_term.cols;
		int graph_height = data_term.rows;
		int gx = i % graph_width;
		int gy = i / graph_width;
		int data_term_penalty = calcDataTermPenalty(data_term.at<int>(gy,gx));
		float grad_hetero_penalty = gradient_heterogenuity.at<float>(gy,gx);
		int dt_x = data_term_penalty;
		int dt_y = data_term_penalty;

		float gh_x = grad_hetero_penalty;
		float gh_y = grad_hetero_penalty;

		if(gx < graph_width-1){
			int temp = calcDataTermPenalty(data_term.at<int>(gy,gx+1));
			dt_x = dt_x > temp ? dt_x : temp;
			float temp2 = gradient_heterogenuity.at<float>(gy,gx+1);
			gh_x = gh_x > temp2 ? exp(-gh_x) : exp(-temp2);
			dt_x += static_cast<int>(gh_x * QUANTIZATION_LEVEL);
			int s_x = smoothing_term_x.at<int>(gy,gx) + dt_x;
			smoothing_term_x.at<int>(gy,gx) = s_x > 0 ? static_cast<int>(smoothing_term_weight * s_x) : 0;

		}
		if(gy < graph_height-1){
			int temp = calcDataTermPenalty(data_term.at<int>(gy+1,gx));
			dt_y = dt_y > temp ? dt_y : temp;
			float temp2 = gradient_heterogenuity.at<float>(gy+1,gx);
			gh_y = gh_y > temp2 ? exp(-gh_y) : exp(-temp2);
			dt_y += static_cast<int>(gh_y * QUANTIZATION_LEVEL);
			int s_y = smoothing_term_y.at<int>(gy,gx) + dt_y;
			smoothing_term_y.at<int>(gy,gx) = s_y > 0.f ? static_cast<int>(smoothing_term_weight * s_y) : 0;
		}
	}
}

ParallelCalcEdgeCapacity::ParallelCalcEdgeCapacity(
					const std::vector<cv::Mat>& src,
					const std::vector<cv::Mat>& sobel_x,
					const std::vector<cv::Mat>& sobel_y,
					const std::vector<cv::Mat>& bg_img,
					const std::vector<cv::Mat>& bg_sobel_x,
					const std::vector<cv::Mat>& bg_sobel_y,
					const std::vector<float>& noise_std_dev,
					const std::vector<float>& gh_expectation,
					const std::vector<float>& gh_std_dev,
					float alpha,
					float thresh_tex_diff,
					unsigned char over_exposure_thresh,
					unsigned char under_exposure_thresh,
					cv::Mat& data_term,
#ifdef DEBUG_TEXCUT
					cv::Mat& tex_int,
#endif
					cv::Mat& gradient_heterogenuity,
					cv::Mat& smoothing_term_x,
					cv::Mat& smoothing_term_y
						):src(src),sobel_x(sobel_x),sobel_y(sobel_y),
						bg_img(bg_img),bg_sobel_x(bg_sobel_x),bg_sobel_y(bg_sobel_y),
						noise_std_dev(noise_std_dev),gh_expectation(gh_expectation),
						gh_std_dev(gh_std_dev),alpha(alpha),
						thresh_tex_diff(thresh_tex_diff),
						over_exposure_thresh(over_exposure_thresh),under_exposure_thresh(under_exposure_thresh),
						data_term(data_term),
#ifdef DEBUG_TEXCUT
						tex_int(tex_int),
#endif
						gradient_heterogenuity(gradient_heterogenuity),
						smoothing_term_x(smoothing_term_x),
						smoothing_term_y(smoothing_term_y){}



