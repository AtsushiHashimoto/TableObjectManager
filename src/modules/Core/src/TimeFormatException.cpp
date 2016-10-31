/*!
 * @file TimeFormatException.h
 * @author Takahiro Suzuki
 **/

/**** Header ****/
/**** mmpl ****/
#include "TimeFormatException.h"

///////////////////////////////////////////////////////////
namespace skl{
	/*!
	 * @brief コンストラクタ
	 *
	 * 実行されないようprivateにしておく
	 * */
	TimeFormatException::TimeFormatException():str(""),formatter(""){}

	/*!
	 * @brief デストラクタ
	 * */
	TimeFormatException::~TimeFormatException() throw (){}

	/** 
	 * @brief コンストラクタ
	 * @param sError エラーメッセージ
	 */
	TimeFormatException::TimeFormatException(const std::string& sError,const std::string& formatter):str(sError),formatter(formatter){}

///////////////////////////////////////////////////////////
	/** 
	 * @brief エラーメッセージを返す関数
	 * @return エラーメッセージ
	 */
	std::string TimeFormatException::getErrMessage() const{
		return "An Error occured while parsing \"" + str + "\" with " + formatter;
	}

	/** 
	 * @brief パースしていた文字列を返す
	 * 
	 * @return 例外を生じさせたString
	 */
	std::string TimeFormatException::getString() const{
		return str;
	}

	/** 
	 * @brief TimeFormatterの名前を返す
	 * 
	 * @return 例外を生じさせたTimeFormatterの名前
	 */
	std::string TimeFormatException::getFormatterName() const{
		return formatter;
	}

	/** 
	 * @brief 出力演算子
	 * 
	 * @param lhs 出力ストリーム
	 * @param rhs 例外オブジェクト
	 * 
	 * @return 出力ストリーム
	 */
	std::ostream& operator<<(std::ostream& lhs,const TimeFormatException& rhs){
		lhs << rhs.getErrMessage();
		return lhs;
	}
}
