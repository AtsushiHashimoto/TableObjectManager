/*!
 * @file TimeFormatterDefault.h
 * @author Takahiro Suzuki
 **/

/**** Header ****/
/**** skl ****/
#include "TimeFormatterDefault.h"
#include <sstream>
///////////////////////////////////////////////////////////
namespace skl{
	// Constructor
	TimeFormatterDefault::TimeFormatterDefault():TimeFormatter("TimeFormatterDefault"){}
	// Copy Constructor
	TimeFormatterDefault::TimeFormatterDefault(const TimeFormatterDefault& other):TimeFormatter(other){}
	// Destructor
	TimeFormatterDefault::~TimeFormatterDefault(){}
	/////////////////////////////////////////////////////////// 
	// Functions
	// オブジェクトを複製する.
	TimeFormatterDefault* TimeFormatterDefault::clone() const{
		return new TimeFormatterDefault(*this);
	}

	// 等価なオブジェクトか?
	bool TimeFormatterDefault::equals(const TimeFormatter& other) const{
		if (this == &other){
			return true;
		}
		const TimeFormatterDefault* skltf = dynamic_cast<const TimeFormatterDefault*>(&other);
		if (skltf == 0){
			return false;
		}
		return true;
	}
	/** 
	 * @brief 文字列を解析し，数字を代入する
	 * 
	 * @param str 解析する文字列
	 * @param Year 年を入れる変数
	 * @param Mon 月を入れる変数
	 * @param Day 日を入れる変数
	 * @param Hour 時を入れる変数
	 * @param Min 分を入れる変数
	 * @param Sec 秒を入れる変数
	 * @param USec USec(micro sec.)を入れる変数
	 */
	void TimeFormatterDefault::parseString(const std::string    & str,int* Year,int* Mon,int* Day,int* Hour,int* Min,int* Sec,long* USec)const throw (TimeFormatException){
		if(str.length() < 10){
			throw TimeFormatException(str,this->getName());
		}

		// YYYYの判定
		if(str.find(".",0) != 4){
			throw TimeFormatException(str,this->getName());
		}
		*Year = (atoi(str.substr(0,4).c_str()) - 1900);
		// MMの判定
		if(str.find(".",5) != 7){
			throw TimeFormatException(str,this->getName());
		}
		*Mon = (atoi( str.substr(5,2).c_str()) -1);
		// DDの判定
		if((str.find("_",8)) != 10){
			throw TimeFormatException(str,this->getName());
		}
		*Day = atoi( str.substr(8,2).c_str());

		// hh.mm.ss.mmm can be skiped;
		*Hour = 0;
		*Min = 0;
		*Sec = 0;
		*USec = 0;


		// hhの判定
		if(str.length()==10) return;
		if(str.find(".",11) != 13){
			throw TimeFormatException(str,this->getName());
		}
		*Hour = atoi( str.substr(11,2).c_str());
		// mmの判定
		if(str.length()==13) return;
		if(str.find(".",14) != 16 ){
			throw TimeFormatException(str,this->getName());
		}
		*Min = atoi( str.substr(14,2).c_str());

		// ssの判定
		if(str.length()==16) return;
		if(str.find(".",17) != 19){
			throw TimeFormatException(str,this->getName());
		}
		*Sec = atoi(str.substr(17,2).c_str());
		// mmmの判定(判定省略)
		if(str.length()==20) return;
		if(str.length() == 26){
			*USec = atol(str.substr(20,6).c_str());
		}
		else if(str.length() == 23){
			*USec = atol(str.substr(20,3).c_str())*1000;
		}
		else{
			throw TimeFormatException(str,this->getName());
		}
	}

	/** 
	 * @brief 文字列へ変換する関数
	 * 
	 * @param Year 年
	 * @param Mon 月
	 * @param Day 日
	 * @param Hour 時
	 * @param Min 分
	 * @param Sec 秒
	 * @param USec マイクロ秒
	 * 
	 * @return 文字列表現
	 */
	std::string TimeFormatterDefault::toString(int Year,int Mon,int Day,int Hour,int Min,int Sec,long USec)const{
		std::ostringstream sstr;
		sstr.str("");

		sstr << std::setw(4) << std::setfill('0') << Year << "."
			<< std::setw(2) << std::setfill('0') <<Mon << "."
			<< std::setw(2) << std::setfill('0') << Day << "_"
			<< std::setw(2) << std::setfill('0') << Hour << "."
			<< std::setw(2) << std::setfill('0') << Min << "."
			<< std::setw(2) << std::setfill('0') << Sec << "."
			<< std::setw(6) << std::setfill('0') << USec;
		return sstr.str();
	}
}
