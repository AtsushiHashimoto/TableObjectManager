/*!
 * @file Serializable.h
 * @author Atsushi HASHIMOTO
 * @date Last Change:2012/Jan/06.
 */
#ifndef __SERIALIZABLE_H__
#define __SERIALIZABLE_H__

#include <stdlib.h>
#include <string>
#include <list>
namespace skl{
/*!
 * @brief BlackBoardを介してデータをやりとりするクラスのインターフェイス
 */
class Serializable{
	public:
		Serializable();
		virtual ~Serializable();
		virtual std::string getClassName()const;
		virtual long serialize(char** buf);
		virtual void deserialize(const char* buf,long buf_size=0);
		long serializedDataLength()const;
	protected:
		explicit Serializable(const Serializable& other);

		//! 直前に呼ばれたserializeで生成されたbufの長さ
		long buf_size;
		//! 直前に呼ばれたserializeで生成されたbufの中身
		char* buf;

		//! _serializeで生成される予定のbufの長さを返す
		virtual long _buf_size()const=0;
		//! クラスの中身を直列化してメンバ変数のbufに格納する
		virtual void _serialize()=0;
		//! 直列化されたクラスの中身を読み込む
		virtual void _deserialize(const char* buf,long buf_size)=0;	
};

} // namespace skl

namespace skl{
	typedef skl::Serializable Serializable;
}
#endif // __SERIALIZABLE_H__

