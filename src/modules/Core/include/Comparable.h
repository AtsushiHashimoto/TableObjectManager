/*!
 * @file Comparable.h
 * @author 橋本敦史
 * @date Last Change:2012/Jan/06.
 */
#ifndef __SKL_COMPARABLE_H__
#define __SKL_COMPARABLE_H__


namespace skl{

	/*!
	 * @brief 比較可能なオブジェクトであることを示す(operator<を定義するだけで他の比較演算子が出来る) 
	 */
	template<class T> class Comparable{

		public:
			Comparable(){};
			virtual ~Comparable(){};
			// これだけは定義しなければならない純粋仮想関数
			virtual bool operator<(const T& other)const=0;
			friend bool operator>=(const T& lhs,const T& rhs){
				return !(lhs < rhs);
			}

			friend  bool operator>(const T& lhs,const T& rhs){
				return rhs < lhs;
			}

			friend  bool operator<=(const T& lhs,const T& rhs){
				return !(lhs > rhs);
			}

			friend  bool operator!=(const T& lhs,const T& rhs){
				return (lhs<rhs || lhs>rhs);
			}

			friend  bool operator==(const T& lhs,const T& rhs){
				return (lhs<=rhs && lhs>=rhs);
			}

		protected:

		private:

	};

} // namespace skl
#endif // __COMPARABLE_H__

