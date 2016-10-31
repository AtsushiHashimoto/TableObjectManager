/*!
 * @file TimeFormatter.h
 * @author Takahiro Suzuki
 **/

#ifndef __TIME_FORMATTER_H__
#define __TIME_FORMATTER_H__
#include <string>
/**** mmpl ****/
#include "TimeFormatException.h"

/**** define ****/

namespace skl{
	/** 
	 * @brief TimeFormatter's abstrcut class
	 */

	/*!
	 * @brief TimeFormatter
	 * @brief 時間と文字列の相互変換を行う基底クラス(仮想)
	 * */
	class TimeFormatter{
		public:
			// Defalut Constructor
			explicit TimeFormatter(const std::string& name="");
			
			// virtual Destructor
			virtual ~TimeFormatter();

			// オブジェクトを複製する
			virtual TimeFormatter* clone()const;

			// 等価なオブジェクトか？
			virtual bool equals(const TimeFormatter& other)const;

			// あるStringからTimeオブジェクトに変換する
			virtual void parseString(const std::string& str,int* Year,int* Mon,int* Day,int* Hour,int* Min,int* Sec,long* USec)const throw (TimeFormatException)=0;
			// TimeオブジェクトをStringに変換する
			virtual std::string toString(int Year,int Mon,int Day,int Hour,int Min,int Sec,long USec) const=0;

			std::string getName()const;
		protected:
			// Copy Constructor(子クラスのみ使用可能)
			TimeFormatter(const TimeFormatter& other);
			std::string name;
		private:
			// 代入演算子を無効にする
			TimeFormatter& operator=(const TimeFormatter& other);
	};

} // namespace skl

#endif
