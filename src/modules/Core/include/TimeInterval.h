/*!
 * @file TimeInterval.h
 * @author 橋本敦史
 * @date Date Created: 2010-06-21.
 * @date Last Change:2012/Jan/13.
 */

#ifndef __SKL_TIME_INTERVAL_H__
#define __SKL_TIME_INTERVAL_H__

#include "Printable.h"

namespace skl{
/*!
 * @brief 時間間隔を表すクラス(最小単位:msec)
 */
class TimeInterval:public Printable<TimeInterval>{
	public:
		TimeInterval(long long msec=0);
		TimeInterval(long hour,int minute,int sec=0,int msec=0,int sign=1);
		TimeInterval(const std::string& str);
		TimeInterval(const TimeInterval& other);
		virtual ~TimeInterval();

		// MmplTimeと同様の書式も与える(本体はprint,scan)
		std::string toString()const;
		void parseString(const std::string& str);

		// Printableに対応
		std::string print()const;
		bool scan(const std::string& str);

		// 秒単位を基準として値を取り出す
		double getAsSecond()const;

		operator long long()const;
		bool operator<(const TimeInterval& other)const;
		TimeInterval& operator=(const TimeInterval& other);
		long long operator-(const TimeInterval& other)const;
		long long operator+(const TimeInterval& other)const;
		TimeInterval& operator-=(const TimeInterval& other);
		TimeInterval& operator+=(const TimeInterval& other);
		long long operator-()const;
		TimeInterval& operator%=(const TimeInterval& other);
		TimeInterval& operator/=(long long factor);
		TimeInterval& operator*=(long long factor);

// Serialzable 純粋仮想関数
		long _buf_size()const;
		void _serialize();
		void _deserialize(const char* buf,long buf_size);

	protected:
		long long msec;
	private:
		
};


} // namespace skl


#endif // __SKL_TIME_INTERVAL_H__


