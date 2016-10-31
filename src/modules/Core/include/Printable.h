/*!
 * @file Printable.h
 * @author 橋本敦史
 * @date Last Change:2012/Jan/13.
 */
#ifndef __SKL_PRINTABLE_H__
#define __SKL_PRINTABLE_H__

#include "Serializable.h"
#include "Comparable.h"
#include <iostream>
#include <string>
#include <memory.h>
namespace skl{

/*!
 * @brief 文字列として書き出しが可能であることを保証するインターフェイス
 */
template<class T> class Printable:public Serializable,public Comparable<T>{

	public:
		Printable();
		virtual ~Printable();
		virtual std::string print()const = 0;
		virtual bool scan(const std::string& str) = 0;
		virtual Printable<T>& operator=(const Printable<T>& other);
		
		// Comparable
		virtual bool operator<(const T& other)const;
	
		// clone
		T* clone()const;	
	protected:
		// Serializable
		virtual long _buf_size()const;
		virtual void _serialize();
		virtual void _deserialize(const char* buf, long buf_size);	
	private:
};


/*!
 * @brief デフォルトコンストラクタ
 */
template<class T>
Printable<T>::Printable(){

}

/*!
 * @brief デストラクタ
 */
template<class T>
Printable<T>::~Printable(){

}

/*!
  @brief 比較演算子(但し、printした文字列の順序を利用するので、意味のある比較をした場合には具象クラス毎に定義すること)
  */
template<class T>
bool Printable<T>::operator<(const T& other)const{
	return this->print() < other.print();
}

/*!
  * @brief 代入演算子
  */
template<class T>
Printable<T>& Printable<T>::operator=(const Printable<T>& other){
	this->scan(other.print());
	return *this;
}

//出力演算子
template<class T>
std::ostream& operator<<(std::ostream& lhs,const Printable<T>& rhs){
	lhs << rhs.print();
	return lhs;
}
//入力演算子
template<class T>
std::istream& operator>>(std::istream& lhs,Printable<T>& rhs){
	std::string str;
	lhs >> str;
	rhs.scan(str);
	return lhs;
}

// Serializable
template<class T>
long Printable<T>::_buf_size()const{
	return static_cast<long>(print().size());
}

template<class T>
void Printable<T>::_serialize(){
	memcpy(buf,print().c_str(),buf_size);
}

template<class T>
void Printable<T>::_deserialize(const char* buf, long buf_size){
	scan(std::string(buf,buf_size));
}

/*!
  @brief cloneを作る
  @return 新しく作られたインスタンス 
 */
template<class T>
T* Printable<T>::clone()const{
	return new T(*this);
}


} // namespace skl
#endif // __SKL_PRINTABLE_H__

