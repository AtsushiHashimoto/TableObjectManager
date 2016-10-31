/*!
 * @file ChiffonBase.h
 * @author kitchen
 * @date Date Created: 2015/Feb/07
 * @date Last Change:2015/Feb/07.
 */
#ifndef __SKL_CHIFFON_BASE_H__
#define __SKL_CHIFFON_BASE_H__

#include "picojson/picojson.h"
#include "HTTPClient.h"
namespace skl{

/*!
 * @class ChiffonBase
 * @brief ChiffonVieer/Navigator共通の処理を記述するインターフェイス　 
 */
class ChiffonBase : public HTTPClient{
	using HTTPClient::get;
	public:
		ChiffonBase(const std::string& host,int port);
		virtual ~ChiffonBase();
	protected:
		bool get(const std::string& path, const picojson::value& query);	
		bool post(const std::string& path, const picojson::value& query);	
	private:
		
};

} // skl

#endif // __SKL_CHIFFON_BASE_H__

