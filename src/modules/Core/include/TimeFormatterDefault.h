/*!
 * @file TimeFormatterDefault.h
 * @author Takahiro Suzuki
 * @date Last Change:2012/Jan/06.
 **/

#ifndef __TIME_FORMATTER_DEFAULT_H__
#define __TIME_FORMATTER_DEFAULT_H__
/**** C++ ****/
#include <iostream>
#include <string>
#include <iomanip>

/**** C ****/
#include <cmath>
#include <ctime>
#include <cstdlib>
/**** OS ****/

/**** skl ****/
#include "TimeFormatter.h"
#include "TimeFormatException.h"
/**** define ****/

namespace skl{
	/** 
	 * @brief TimeFormatterDefault
	 * @brief YYYY.MM.DD_hh.mm.ss.fff 形式を扱う
	 */
	class TimeFormatterDefault : public TimeFormatter{
		public:
			// Constructor
			TimeFormatterDefault();
			// Destructor
			virtual ~TimeFormatterDefault();
			// オブジェクトを複製する.
			virtual TimeFormatterDefault* clone() const;
			// 等価なオブジェクトか?
			virtual bool equals(const TimeFormatter& other) const;
			 //             
			// 文字列を解析する
			void parseString(const std::string& str,int* Year,int* Mon,int* Day,int* Hour,int* Min,int* Sec,long* USec)const throw (TimeFormatException);
			// 文字列を作る
			std::string toString(int Year,int Mon,int Day,int Hour,int Min,int Sec,long USec) const;
		protected:
			// Copy Constructor(子クラスのみ使用可能)
			TimeFormatterDefault(const TimeFormatterDefault& other);
		private:
			// 代入演算子は無効にする
			TimeFormatterDefault& operator=(const TimeFormatterDefault& other);
	};

	typedef TimeFormatterDefault TimeFormatterDefaultDefault;
} // namespace skl

#endif
