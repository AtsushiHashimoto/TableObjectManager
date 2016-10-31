/*!
 * @file Flow.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jun/29
 * @date Last Change: 2012/Jul/06.
 */
#include "Flow.h"
#include "skl.h"
#include "sklcv.h"
#include <cassert>
#include <fstream>
using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
Flow::Flow(){

}

/*!
 * @brief Shallow Copyするコンストラクタ
 */
Flow::Flow(const Flow& other):u(other.u),v(other.v){

}

/*!
 * @brief Constractor which sets u and v.
 * */
Flow::Flow(const cv::Mat& u,const cv::Mat& v):u(u.clone()),v(v.clone()){
}

/*!
 * @brief デストラクタ
 */
Flow::~Flow(){

}

Flow Flow::clone()const{
	return Flow(this->u.clone(),this->v.clone());
}

/*!
 * @brief check size and type of u and v.
 */
bool Flow::isValid()const{
	if(u.size()!=v.size()) return false;
	if(u.cols<=1) return false;
	if(u.rows<=1) return false;
	if(u.type()!=CV_32FC1) return false;
	if(u.type()!=v.type()) return false;
	return true;
}
const cv::Mat& Flow::operator[](Flow::Direction d)const{
	if(X==d) return u;
	return v;
}

cv::Mat& Flow::operator[](Direction d){
	if(X==d) return u;
	return v;

}

/*!
 * convert {u,v} to moved distance sqrt(u^2+v^2)
 * */
void Flow::distance(cv::Mat& r){
	assert(isValid());
	if(r.size()!=u.size() || r.type()!=CV_32FC1){
		r = cv::Mat::zeros(u.size(),CV_32FC1);
	}

	for(int y=0;y<u.rows;y++){
		float* dx = u.ptr<float>(y);
		float* dy = v.ptr<float>(y);
		float* dist = r.ptr<float>(y);
		for(int x=0;x<u.cols;x++){
			dist[x] = sqrt(dx[x]*dx[x]+dy[x]*dy[x]);
		}
	}
}

void Flow::angle(cv::Mat& rad,float offset_rad,float origin_return_value){
	assert(isValid());
	if(rad.size()!=u.size() || rad.type()!=CV_32FC1){
		rad = cv::Mat::zeros(u.size(),CV_32FC1);
	}

	for(int y=0;y<u.rows;y++){
		float* dx = u.ptr<float>(y);
		float* dy = v.ptr<float>(y);
		float* angle = rad.ptr<float>(y);
		for(int x=0;x<u.cols;x++){
			angle[x] = skl::radian(dx[x],dy[x],offset_rad,origin_return_value);
		}
	}
}

cv::Mat Flow::visualize(cv::Mat& base, int interval, const cv::Scalar& arrow_color){
	cv::Mat tar = cv::Mat(base.size(),CV_8UC3);
	if(base.channels()==1){
		cv::cvtColor(base,tar,CV_GRAY2BGR);
	}
	else{
		tar = base.clone();
	}
	for(int y=0;y<u.rows;y+=interval){
		for(int x=0;x<u.cols;x+=interval){
			cv::Point move(u.at<float>(y,x), v.at<float>(y,x));
			if(move.x == 0 && move.y == 0) continue;
			cv::Point start(x,y);
			cv::Point end(x + move.x, y + move.y);
			arrow(tar,start,end,arrow_color,1,8);
		}
	}
	return tar;
}


bool Flow::read(const std::string& filename){
	std::ifstream fin;
	fin.open(filename.c_str(),std::ios::binary);
	if(!fin){
		return false;
	}
	int size[2];
	fin.read((char*)size,2*sizeof(int));
	u = cv::Mat(cv::Size(size[0],size[1]),CV_32FC1);
	v = cv::Mat(cv::Size(size[0],size[1]),CV_32FC1);
	for(int y=0;y<u.rows;y++){
		char* dx = (char*)u.ptr<float>(y);
		fin.read(dx,u.cols * sizeof(float));
	}
	for(int y=0;y<v.rows;y++){
		char* dx = (char*)v.ptr<float>(y);
		fin.read(dx,v.cols * sizeof(float));
	}
	fin.close();
	return true;
}

bool Flow::write(const std::string& filename)const{
	if(!isValid()) return false;
	std::ofstream fout;
	fout.open(filename.c_str(),std::ios::binary);
	if(!fout){
		return false;
	}

	fout.write((char*)&u.cols,sizeof(int));
	fout.write((char*)&u.rows,sizeof(int));
	for(int y=0;y<u.rows;y++){
		fout.write((char*)u.ptr<float>(y),sizeof(float)*u.cols);
	}
	for(int y=0;y<v.rows;y++){
		fout.write((char*)v.ptr<float>(y),sizeof(float)*v.cols);
	}
	fout.close();
	return true;
}
