/*!
 * @file   MmTime.cpp
 * @author Takahiro Suzuki
 * @date Last Change:2012/Jul/09.
 **/

/**** Header ****/
#include <climits>
/**** skl2 ****/
#include "sklTime.h"
#include "TimeFormatter.h"
// Default Formatterとして
#include "TimeFormatterDefault.h"
///////////////////////////////////////////////////////////
namespace skl{
	int Time::JP_LOCAL = 9;

	Time::Time(){
		defaulttf = new TimeFormatterDefault();
		tv_st.tv_sec = 0;
		tv_st.tv_usec = 0;
	}


	/** 
	 * @brief コンストラクタ
	 * @param nSec 紀元(1970年1月1日00:00:00 UTC)からの経過時間 Second
	 * @param nMSec MiliSecond 3桁
	 * @param nUSec MicroSecond 3桁
	 */
	Time::Time( long nSec,int nMSec, int nUSec){
		defaulttf = new TimeFormatterDefault();
		tv_st.tv_sec=nSec;
		tv_st.tv_usec=nMSec*1000+nUSec;
	}
	Time::Time(const Time& other){
		defaulttf = other.defaulttf->clone();
		tv_st.tv_sec = other.tv_st.tv_sec;
		tv_st.tv_usec = other.tv_st.tv_usec;
	}

	Time::Time(int YYYY,int MM,int DD,int hh, int mm, int ss, int mmm, int usec){
		defaulttf = new TimeFormatterDefault();

		struct tm tm_st;
		tm_st.tm_year=YYYY-1900;
		tm_st.tm_mon=MM-1;
		tm_st.tm_mday=DD;
		tm_st.tm_hour=hh;
		tm_st.tm_min=mm;
		tm_st.tm_sec=ss;

		// tm構造体からUNIX Timeを獲得する
		tv_st.tv_sec = mktime(&tm_st);
		tv_st.tv_usec = mmm*1000 + usec;
	}

	Time::~Time(){
		delete defaulttf;
	}
	///////////////////////////////////////////////////////////
	// TimeFormatter用
	/** 
	 * @brief DefaultFormatterに従った文字列を解析し，値を得る
	 * 
	 * @param str 変換元の文字列
	 */
	void Time::parseString(const std::string& str) throw (TimeFormatException){
		assert(defaulttf!=NULL);
		int nYear,nMon,nDay,nHour,nMin,nSec;
		long nUSec;

		// 具体的な処理はdefaulttfへ移譲
		defaulttf->parseString(str,&nYear,&nMon,&nDay,&nHour,&nMin,&nSec,&nUSec);


		struct tm tm_st;
		tm_st.tm_sec=nSec;
		tm_st.tm_min=nMin;
		tm_st.tm_hour=nHour;
		tm_st.tm_mday=nDay;
		tm_st.tm_mon=nMon;
		tm_st.tm_year=nYear;
		tv_st.tv_sec = mktime(&tm_st);
		tv_st.tv_usec = nUSec;
	}

	/** 
	 * @brief 文字列表現を返す
	 * 
	 * @return DefaultFormatterに従って変換した文字列
	 */
	std::string Time::toString()const{
		assert(defaulttf!=NULL);
		int nYear,nMon,nDay,nHour,nMin,nSec;
		time_t Sec = tv_st.tv_sec;
		long USec = tv_st.tv_usec;
		struct tm* tm_st = localtime(&Sec);

		nYear = tm_st->tm_year+1900;
		nMon = tm_st->tm_mon+1;
		nDay = tm_st->tm_mday;
		nHour = tm_st->tm_hour;
		nMin = tm_st->tm_min;
		nSec = tm_st->tm_sec;


		return defaulttf->toString(nYear,nMon,nDay,nHour,nMin,nSec,USec);
	}

	/** 
	 * @brief 指定されたFormatterを使って文字列を解析し，値を得る
	 * 
	 * @param str 指定されたFormatterに従った文字列
	 * @param timeformatter Formatter
	 */
	void Time::parseString(const std::string& str,TimeFormatter* timeformatter) throw (TimeFormatException){
		int nYear,nMon,nDay,nHour,nMin,nSec;
		long nUSec;

		timeformatter->parseString(str,&nYear,&nMon,&nDay,&nHour,&nMin,&nSec,&nUSec);

		struct tm tm_st;
		tm_st.tm_sec=nSec;
		tm_st.tm_min=nMin;
		tm_st.tm_hour=nHour;
		tm_st.tm_mday=nDay;
		tm_st.tm_mon=nMon;
		tm_st.tm_year=nYear;
		tv_st.tv_sec = mktime(&tm_st);
		tv_st.tv_usec = nUSec;
	}

	/** 
	 * @brief 指定されたFormatterを使って文字列表現に変換する
	 * 
	 * @param timeformatter Formatter
	 * 
	 * @return このオブジェクトの文字列表現
	 */
	std::string Time::toString(TimeFormatter* timeformatter)const{
		int nYear,nMon,nDay,nHour,nMin,nSec;
		time_t Sec = tv_st.tv_sec;
		long USec = tv_st.tv_usec;
		struct tm* tm_st = localtime(&Sec);

		nYear = tm_st->tm_year+1900;
		nMon = tm_st->tm_mon+1;
		nDay = tm_st->tm_mday;
		nHour = tm_st->tm_hour;
		nMin = tm_st->tm_min;
		nSec = tm_st->tm_sec;

		return timeformatter->toString(nYear,nMon,nDay,nHour,nMin,nSec,USec);
	}

	/** 
	 * @brief デフォルトのFormatterを設定する．
	 * cloneされるので，引数で渡したTimeFormatterオブジェクトは自分で解放すること．
	 * 
	 * @param tf 指定したいTimeFormatter
	 */
	void Time::setDefaultTimeFormatter(const TimeFormatter *tf){
		if(defaulttf!=NULL)delete defaulttf;
		defaulttf=tf->clone();
	}

	///////////////////////////////////////////////////////////
	// accessor
	int Time::getYear()const{
		struct tm* tm_st = localtime(&(tv_st.tv_sec));
		return tm_st->tm_year+1900;
	}

	int Time::getMonth()const{
		struct tm* tm_st = localtime(&(tv_st.tv_sec));
		return tm_st->tm_mon+1;
	}

	int Time::getDay()const{
		struct tm* tm_st = localtime(&(tv_st.tv_sec));
		return tm_st->tm_mday;
	}

	int Time::getHour()const{
		struct tm* tm_st = localtime(&(tv_st.tv_sec));
		return tm_st->tm_hour;
	}

	int Time::getMinute()const{
		struct tm* tm_st = localtime(&(tv_st.tv_sec));
		return tm_st->tm_min;
	}

	int Time::getSecond()const{
		struct tm* tm_st = localtime(&(tv_st.tv_sec));
		return tm_st->tm_sec;
	}

	int Time::getMilliSecond()const{
		return tv_st.tv_usec/1000;
	}

	///////////////////////////////////////////////////////////
	// operator
	Time& Time::operator=(const Time& other){
		if(defaulttf!=NULL) delete defaulttf;
		defaulttf = other.defaulttf->clone();
		
		tv_st.tv_sec = other.tv_st.tv_sec;
		tv_st.tv_usec = other.tv_st.tv_usec;

		return *this;
	}

	bool Time::operator==(const Time& other) const{
		if(!defaulttf->equals(*(other.defaulttf))) return false;
		if(tv_st.tv_sec != other.tv_st.tv_sec) return false;
		else if(tv_st.tv_usec != other.tv_st.tv_usec) return false;
		else return true;
	}

	bool Time::operator!=(const Time& other) const{
		return !(*this == other);
	}

	bool Time::operator<(const Time& other) const{
		if(tv_st.tv_sec < other.tv_st.tv_sec) return true;
		else if (tv_st.tv_sec == other.tv_st.tv_sec && tv_st.tv_usec < other.tv_st.tv_usec) return true;
		else return false;
	}

	bool Time::operator>(const Time& other) const{
		return !(*this < other || *this==other);
	}

	bool Time::operator<=(const Time& other) const{
		return !(*this > other);
	}

	bool Time::operator>=(const Time& other) const{
		return !(*this < other);
	}


	std::ostream& operator<<(std::ostream& lhs,const Time& rhs){
		rhs.print(lhs);
		return lhs;
	}

	void Time::print(std::ostream& out)const{
		out << toString();
	}

	/*
	 * @brief millisecond単位で差分を返す
	 * */
	long long Time::operator-(const Time& other) const{
		long long msec = static_cast<long long>(this->tv_st.tv_usec)/1000;
		msec -= static_cast<long long>(other.tv_st.tv_usec)/1000;
		long long sec = this->tv_st.tv_sec;
		if(msec<0){
			msec += 1000;
			sec -= 1;
		}
		sec -= other.tv_st.tv_sec;
		msec += sec * 1000;
		return msec;
	}

	// 加減演算
	Time Time::operator+(long long MSec)const{
		Time temp(*this);
		long long usec = tv_st.tv_usec + MSec * 1000;
		temp.tv_st.tv_sec += usec / 1000000;
		temp.tv_st.tv_usec = usec % 1000000;
		return temp;
	}

	Time Time::operator-(long long MSec)const{
		Time temp(*this);
		long long usec = MSec * 1000;
		while(temp.tv_st.tv_usec < usec ){
			temp.tv_st.tv_sec --;
			temp.tv_st.tv_usec += 1000000;
		}
		temp.tv_st.tv_usec -= usec;
		return temp;
	}

	Time& Time::operator+=(long long MSec){
		*this=*this+MSec;
		return *this;
	}

	Time& Time::operator-=(long long MSec){
		*this = *this - MSec;
		return *this;
	}

	long long Time::operator%(long long MSec){
		long long msec = static_cast<long long>(this->tv_st.tv_usec)/1000;
		msec += static_cast<long long>(this->tv_st.tv_sec) * 1000;
		return msec % MSec;
	}
	///////////////////////////////
	// Serializable
	long Time::_buf_size()const{
		// MM,DD,hh,mm,ssはcharで、YYYY,mmmはintで送る
		return sizeof(int)*2+sizeof(char)*5;
	}
	void Time::_serialize(){
		unsigned int offset;
		int YY = getYear();
		std::memcpy(buf,&YY,sizeof(int));
		offset=sizeof(int);
		buf[offset] = static_cast<char>(getMonth());
		buf[offset+1] = static_cast<char>(getDay());
		buf[offset+2] = static_cast<char>(getHour());
		buf[offset+3] = static_cast<char>(getMinute());
		buf[offset+4] = static_cast<char>(getSecond());
		int msec = getMilliSecond();
		std::memcpy(buf+offset+5,&msec,sizeof(int));
	}
	void Time::_deserialize(const char* buf,long buf_size){
		// このクラスのbuf_sizeは常に固定であるので無視
		int YYYY,msec;
		char MM,DD,hh,mm,ss;
		std::memcpy(&YYYY,buf,sizeof(int));
		MM = buf[sizeof(int)];
		DD = buf[sizeof(int)+1];
		hh = buf[sizeof(int)+2];
		mm = buf[sizeof(int)+3];
		ss = buf[sizeof(int)+4];	
		std::memcpy(&msec,buf+sizeof(int)+5,sizeof(int));	
		*this = Time(YYYY,MM,DD,hh,mm,ss,msec);
	}

}
