/*!
 * @file   Time.h
 * @author Takahiro Suzuki
 * @date Last Change:2012/Jul/09.
 **/

#ifndef __SKL_TIME_H__
#define __SKL_TIME_H__


#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

/**** C++ ****/
#include <iostream>
#include <string>

/**** C ****/
#include <ctime>
#include <cmath>
#include <cassert>
#include <cstring>

/**** 環境依存 ****/
/**** WIN32 ****/
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
/**** Linux ****/
#elif __linux__
#include <sys/time.h>
#endif	/**** ifdef linux ****/

/**** Mmpl ****/
#include "Serializable.h"
#include "TimeFormatException.h"
/**** typedef ****/
#ifdef _WIN32
typedef long suseconds_t;
#endif	/**** ifdef WIN32 ****/

namespace skl{
	class TimeFormatter;
	/*!
	 * @brief Time
	 * @brief 非負で1usec単位のタイムスタンプを扱えるクラス
	 * */
	class Time : public Serializable{
		public:
			static int JP_LOCAL;
			/*!
			 * @brief 現在時刻を表すオブジェクトを返す静的メソッド
			 * @return 現在時刻を表す Time オブジェクト
			 */
			static Time now(int utc_local_diff=JP_LOCAL){

				namespace pt = boost::posix_time;
				auto _now = pt::microsec_clock::universal_time();
				_now = boost::date_time::local_adjustor<pt::ptime, 9, pt::no_dst>::utc_to_local(_now);
				struct tm tm= pt::to_tm(_now);

				struct timeval tv;
				tv.tv_sec=(long)mktime(&tm);
				auto _now_sec = pt::ptime_from_tm(tm);
				tv.tv_usec=(long)(_now - _now_sec).fractional_seconds();

				return Time(static_cast<int>(tv.tv_sec),tv.tv_usec/1000,tv.tv_usec%1000);
			};

			Time();
			Time(long nSec,int nMSec,int nUSec=0);
			Time(const Time& other);	//コピーコンストラクタ
			Time(int YYYY,int MM,int DD,int hh, int mm, int ss=0, int mmm=0,int usec=0);	// Set
			~Time();

			Time& operator=(const Time& other);

///////////////////////////////////////////////////////////
// Accessor
			int getYear()const;
			int getMonth()const;
			int getDay()const;
			int getHour()const;
			int getMinute()const;
			int getSecond()const;
			int getMilliSecond()const;

///////////////////////////////////////////////////////////
// TimeFormatter
			void parseString(const std::string& strxx) throw (TimeFormatException);
			std::string toString()const;
			void parseString(const std::string& str,TimeFormatter* timeformatter) throw (TimeFormatException);	// 指定したTimeFormatter型で自分のオブジェクトに変換する
			std::string toString(TimeFormatter* timeformatter)const;	// 指定したTimeFormatter型で自分のオブジェクトをStringに変換する

			void setDefaultTimeFormatter(const TimeFormatter* tf);
////////////////////////////////////////////////////////////
// 演算子オーバーロード
			// 比較演算子
			bool operator==(const Time& other) const;
			bool operator!=(const Time& other) const;
			bool operator<(const Time& other) const;
			bool operator>(const Time& other) const;
			bool operator<=(const Time& other) const;
			bool operator>=(const Time& other) const;

			// 加減演算
			long long operator-(const Time& other) const;
/*			Time operator+(const Time& other) const;
			Time operator-(const Time& other) const;
			Time& operator+=(const Time& other);
			Time& operator-=(const Time& other);
*/			Time operator+(long long MSec) const;
			Time operator-(long long MSec) const;
			Time& operator+=(long long MSec);
			Time& operator-=(long long MSec);
			long long operator%(long long MSec);
			//
			void print(std::ostream& out)const;

////////////////////////////////////////////////////////////
// Serialzable 純粋仮想関数
			long _buf_size()const;
			void _serialize();
			void _deserialize(const char* buf,long buf_size);
		private:
			// TimeFormatter
			TimeFormatter* defaulttf;
#ifdef _WIN32
			struct timeval{
				time_t tv_sec;
				suseconds_t tv_usec; 
			}tv_st;
#endif	// ifdef WIN32
#ifdef __linux__
			struct timeval tv_st;	//timeval構造体
#endif	// ifdef linux

	};
	// 出力演算子
	std::ostream& operator<<(std::ostream& lhs, const Time& rhs);
} // namespace skl
#endif
