/*!
 * @file Filter.h
 * @author a_hasimoto
 * @date Date Created: 2012/Feb/10
 * @date Last Change:2012/May/22.
 */
#ifndef __SKL_FILTER_H__
#define __SKL_FILTER_H__


namespace skl{

/*!
 * @brief compute関数を持っているクラスであることを保証するインターフェイス
 */
template<class RET,class SRC, class DIST> class _Filter{

	public:
		_Filter(){}
		virtual RET compute(const SRC& src, DIST& dist) = 0;
		virtual ~_Filter(){}
	protected:
		
};

} // skl

#endif // __SKL_FILTER_H__

