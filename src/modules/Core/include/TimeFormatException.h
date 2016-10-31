/*!
 * @file TimeFormagtException.h
 * @author Takahiro Suzuki
 * @date Last Change:2012/Jan/06.
 **/

#ifndef TIME_FORMAT_EXCEPTION_H__
#define TIME_FORMAT_EXCEPTION_H__


/**** C++ ****/
#include <iostream>
#include <string>
/**** C ****/

namespace skl{
	/*!
	 * @brief TimeFormatException
	 * @brief 時間←→の変換ができないときの例外クラス
	 * */
	class TimeFormatException :public std::exception{
		public:
			virtual ~TimeFormatException() throw();
			explicit TimeFormatException(const std::string& sError,const std::string& formatter);
			//
			std::string getErrMessage() const;
			std::string getString() const;
			std::string getFormatterName() const;
		private:
			TimeFormatException();
			TimeFormatException operator=(const TimeFormatException& other);
			// Variables
			std::string str;
			std::string formatter;
	};
	// 出力演算子 -- [CLASSNAME]部分を作成したクラス名に変換する
	std::ostream& operator<<(std::ostream& lhs,const TimeFormatException& rhs);
}
#endif

