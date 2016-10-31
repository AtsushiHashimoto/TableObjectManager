/*!
 * @file Serializable.cpp
 * @author Atsushi HASHIMOTO
 * @date Last Change:2012/Apr/01.
 */
#include "Serializable.h"
#include <iostream>
#include <typeinfo>

using namespace std;

namespace skl{
/*!
 * @brief デフォルトコンストラクタ
 */
Serializable::Serializable():buf_size(0),buf(NULL){
}

/*!
 * @brief コピーコンストラクタ
  */
Serializable::Serializable(const Serializable& other):buf_size(0),buf(NULL){	

}

/*!
 * @brief デストラクタ
 */
Serializable::~Serializable(){
	if(buf!=NULL){
		// 何らかのバッファが存在するので、それらを開放する
		free(buf);
		buf = NULL;
	}
}

/*!
  @brief 関数名を返す
  */
std::string Serializable::getClassName()const{
	return (typeid (*this).name());
}

/*!
  @brief 直列化された場合のデータのサイズを返す
  */
long Serializable::serializedDataLength()const{
	return _buf_size();
}

/*!
 * @brief _buf_size()の返り値を元にbufのメモリ管理をしてから_serializeを呼び出す
 * @param buf blackboardに投げるバッファ
 * @return バッファのバイト長(sizeof(char)xNのN)
 */
long Serializable::serialize(char** buf){
	if(buf_size>0){
		// 何らかのバッファが存在するので、それらを開放する
		free(this->buf);
	}
	// 具象クラスから直列化したときのサイズを得る
	buf_size = _buf_size();
	if(buf_size<1){
		// 中身がないのでそのまま終了
		std::cerr << "at " << __FILE__ << ": " << __LINE__ << std::endl;
		std::cerr << "Warning: 空のバッファが生成されました。" << std::endl;
		return 0;
	}

	this->buf = (char*)malloc(sizeof(char)*buf_size);	

	// 具象クラスの直列化関数を呼ぶ
	_serialize();
	// 引数にこの関数のバッファのアドレスを格納する
	*buf = this->buf;
//	cout << "size: " << buf_size << endl;
	return buf_size;
}

/*!
 * @brief _buf_size()の返り値を元にbufのメモリ管理をしてから_serializeを呼び出す
 * @param buf blackboardに投げるバッファ
 * @return バッファのバイト長(sizeof(char)xNのN)
 */
void Serializable::deserialize(const char* buf,long buf_size){
	if(buf==NULL||buf_size<1){
		// バッファが空なのでエラーを返して終了
		std::cerr << "at " << __FILE__ << ": " << __LINE__ << std::endl;
		std::cerr << "Warning: 空のバッファが入力されました。" << std::endl;
		return;
	}
	_deserialize(buf,buf_size);
}

} // namespace skl
