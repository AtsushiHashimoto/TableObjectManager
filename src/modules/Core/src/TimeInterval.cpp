/*!
 * @file TimeInterval.cpp
 * @author 橋本敦史
 * @date Date Created: 2010-06-21.
 * @date Last Change:2012/Jan/13.
 */
#include "TimeInterval.h"
#include <sstream>
#include <iomanip>
#include <cassert>

namespace skl{


/*!
 * @brief デフォルトコンストラクタ
 */
TimeInterval::TimeInterval(long long msec):msec(msec){

}

/*!
 * @brief 文字列を引数にとるコンストラクタ
 */
TimeInterval::TimeInterval(const std::string& str){
	scan(str);
}

/*!
 * @brief 要素毎に指定するコンストラクタ
 */
TimeInterval::TimeInterval(long hour,int minute,int sec,int msec,int sign){
	assert(sign==1 || sign==-1);
	assert(0<=msec && msec<1000);
	assert(0<=sec && sec<60);
	assert(0<=minute && minute<60);
	this->msec = hour;
	this->msec = this->msec*60 + minute;
	this->msec = this->msec*60 + sec;
	this->msec = this->msec*1000 + msec;
	this->msec *= sign;
}



/*!
 * @brief コピーコンストラクタ
 */
TimeInterval::TimeInterval(const TimeInterval& other){
	this->msec = other.msec;
}


/*!
 * @brief デストラクタ
 */
TimeInterval::~TimeInterval(){

}

/*!
 * @brief millisecond以下を小数として、秒単位で取り出す
 * @return sec(double)
 * */
double TimeInterval::getAsSecond()const{
	return static_cast<double>(msec)/1000;
}

/*!
 * @brief 文字列として書き出す(本体はprint())
 * @retval 時間情報の文字列([-]HH:MM:SS.mmm)
 * */
std::string TimeInterval::toString()const{
	return print();
}

/*!
 * @brief 文字列として書き出す
 * @retval 時間情報の文字列([-]HH:MM:SS.mmm)
 * */
std::string TimeInterval::print()const{
	std::stringstream ss;
	long long val = msec;
	if(val<0){
		ss << "-";
		val *=-1;
	}
	int millisec = val%1000;
	val = val/1000;
	int sec = val%60;
	val /= 60;
	int minute = val%60;
	int hour  = (int)(val/60);
	ss << hour << ":";
	ss << std::setw(2) << std::setfill('0') << minute << ":";
	ss << std::setw(2) << std::setfill('0') << sec << ".";
	ss << std::setw(3) << std::setfill('0') << millisec;
	return ss.str();
}

/*!
 * @brief 文字列から読み出す(本体はscanで定義)
 * @param str ([-]HH:MM:SS.mmm)という書式でかかれた文字列
 * */
void TimeInterval::parseString(const std::string& str){
	scan(str);
}

/*!
 * @brief 文字列から読み出す
 * @param str ([-]HH:MM:SS.mmm)という書式でかかれた文字列
 * */
bool TimeInterval::scan(const std::string& _str){
	// 符号の読み取り
	int sign = 1;
	std::string str = _str;
	if(str.size()>0 && str[0]=='-'){
		sign = -1;
		str = str.substr(1,std::string::npos);
	}
	// HHの読み取り
	size_t idx = str.find(":");
	if(idx==std::string::npos){
		return false;
/*		std::cerr << "at " << __FILE__ << ": "<< __LINE__ << std::endl;
		std::cerr << "Warning: Invalid format '" << _str << "'." <<std::endl;
		std::cerr << "         A valid format is HH:MM:SS.mmm" << std::endl;		return;*/
	}
	std::string val = str.substr(0,idx);
	str = str.substr(idx+1,std::string::npos);
	int hour = atoi(val.c_str());

	// MMの読み取り
	idx = str.find(":");
	if(idx==std::string::npos){
/*		std::cerr << "at " << __FILE__ << ": "<< __LINE__ << std::endl;
		std::cerr << "Warning: Invalid format '" << _str << "'." <<std::endl;
		std::cerr << "         A valid format is HH:MM:SS.mmm" << std::endl;
*/		return false;
	}
	val = str.substr(0,idx);
	str = str.substr(idx+1,std::string::npos);
	int minute = atoi(val.c_str());

	// SSの読み取り
	idx = str.find(".");
	if(idx==std::string::npos){
/*		std::cerr << "at " << __FILE__ << ": "<< __LINE__ << std::endl;
		std::cerr << "Warning: Invalid format '" << _str << "'." <<std::endl;
		std::cerr << "         A valid format is HH:MM:SS.mmm" << std::endl;
*/		return false;
	}
	val = str.substr(0,idx);
	str = str.substr(idx+1,std::string::npos);
	int sec = atoi(val.c_str());

	// mmmの読み取り(小数点4桁以下は無視)
	if(str==""){
/*		std::cerr << "at " << __FILE__ << ": "<< __LINE__ << std::endl;
		std::cerr << "Warning: Invalid format '" << _str << "'." <<std::endl;
		std::cerr << "         A valid format is HH:MM:SS.mmm" << std::endl;
*/		return false;
	}
	int millisec = atoi(str.substr(0,3).c_str());

	// 読み取った分を変換する
	msec = hour;
	msec *= 60;
	msec += minute;
	msec *= 60;
	msec += sec;
	msec *= 1000;
	msec += millisec;
	msec *= sign;
	return true;
}

/*
 * @brief longへの静的キャスト演算子
 * */
TimeInterval::operator long long()const{
	return msec;
}
/*
 * @brief 比較演算子('<'以外についてはPrintableの親クラスであるComparableで自動的に定義される)
 * @as Comparable
 * */
bool TimeInterval::operator<(const TimeInterval& other)const{
	return this->msec<other.msec;
}

/*
 * @brief 代入演算子
 * */
TimeInterval& TimeInterval::operator=(const TimeInterval& other){
	this->msec = other.msec;
	return *this;
}

/*
 * @brief 減算演算子
 * */
long long TimeInterval::operator-(const TimeInterval& other)const{
	return this->msec-other.msec;
}

/*
 * @brief 加算演算子
 * */
long long TimeInterval::operator+(const TimeInterval& other)const{
	return this->msec+other.msec;
}

/*
 * @brief 正負を逆転する
 * */
long long TimeInterval::operator-()const{
	return -msec;
}
/*
 * @brief 減算演算子
 * */
TimeInterval& TimeInterval::operator-=(const TimeInterval& other){
	this->msec -= other.msec;
	return *this;
}

/*
 * @brief 加算演算子
 * */
TimeInterval& TimeInterval::operator+=(const TimeInterval& other){
	this->msec += other.msec;
	return *this;
}

/*
 * @brief 除算演算子
 * */
TimeInterval& TimeInterval::operator/=(long long factor){
	*this = *this / factor;
	return *this;
}

/*
 * @brief 乗算演算子
 * */
TimeInterval& TimeInterval::operator*=(long long factor){
	*this = *this * factor;
	return *this;
}


TimeInterval& TimeInterval::operator%=(const TimeInterval& other){
	msec %= other.msec;
	return *this;
}

/*
 * @brief 直列化するときのBufferの長さ
 * */
long TimeInterval::_buf_size()const{
	return sizeof(long);
}

/*
 * @brief 直列化を行う
 * */
void TimeInterval::_serialize(){
	memcpy(buf,&msec,sizeof(long));
}

/*
 * @brief 直列化されたデータを読み込む
 * */
void TimeInterval::_deserialize(const char* buf,long buf_size){
	memcpy(&msec,buf,sizeof(long));
}

} // namespace skl




