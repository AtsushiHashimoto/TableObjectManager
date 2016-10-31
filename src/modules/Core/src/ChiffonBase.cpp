/*!
 * @file ChiffonBase.cpp
 * @author kitchen
 * @date Date Created: 2015/Feb/07
 * @date Last Change: 2015/Feb/07.
 */
#include "ChiffonBase.h"

using namespace skl;

/*!
 * @brief デフォルトコンストラクタ
 */
ChiffonBase::ChiffonBase(const std::string& host,int port):HTTPClient(host,port){

}

/*!
 * @brief デストラクタ
 */
ChiffonBase::~ChiffonBase(){

}

bool ChiffonBase::get(const std::string& path, const picojson::value& query){
	std::string query_str = query.serialize();
	return get(path,query_str);
}
