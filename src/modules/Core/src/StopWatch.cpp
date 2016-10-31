/*!
 * @file StopWatch.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/06
 * @date Last Change: 2012/Feb/04.
 */
#include "StopWatch.h"

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
StopWatch::StopWatch(){
	start();
}

/*!
 * @brief デストラクタ
 */
StopWatch::~StopWatch(){

}

/*!
 * @brief start measure
 * */
void StopWatch::start(){
	base_time = Time::now();
	lap_times.resize(1,0);
}

/*!
 * @brief stop measure
 * */
TimeInterval StopWatch::stop(){
	return Time::now() - base_time;
}

void StopWatch::reset(){
	lap_times.clear();
	start();
}

TimeInterval StopWatch::lap_time(size_t i)const{
	if(i+1>=lap_times.size()) return 0;
	return lap_times[i+1];
}

TimeInterval StopWatch::lap(){
	Time cur_time = Time::now();
	TimeInterval time = cur_time - base_time;
	lap_times.push_back(time);
	base_time = cur_time;
	return time;
}

size_t StopWatch::record_num()const{
	return lap_times.size()-1;
}

TimeInterval StopWatch::elapsedTime()const{
	TimeInterval sum=0;
	for(size_t i=1;i<lap_times.size();i++){
		sum+=lap_times[i];
	}
	return sum;
}
