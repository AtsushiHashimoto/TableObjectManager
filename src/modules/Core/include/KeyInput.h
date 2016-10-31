/*!
 * @file KeyInput.h
 *
 * @author 橋本敦史
 * @date Last Change:2012/Oct/14.
 * */

#ifndef __SKL_KEY_INPUT__
#define __SKL_KEY_INPUT__

#include <string>
#include <sstream>

namespace skl{
	/*
	 * @class cvWaitKeyで取得したキーを整理する
	 * */
	class KeyInput{
		public:
			enum{
				SHIFT = 0x10000,
				CTRL = 0x40000,
				NUMLOCK = 0x100000,
				CAPSLOCK = 0x20000,
				ALT = 0x80000,
				LEFT_ARROW  = 65361,
				UP_ARROW    = 65362,
				RIGHT_ARROW = 65363,
				DOWN_ARROW  = 65364
			};
			KeyInput();
			~KeyInput();
			char set(int keystroke);

			inline char getChar(){
				return static_cast<char>(_code);
			}

			inline bool isValid(){return is_valid;}

			inline int code()const{return _code;}

			inline bool shift()const{return _shift;}
			inline bool ctrl()const{return _ctrl;}
			inline bool numlock()const{return _numlock;}
			inline bool capslock()const{return _capslock;}
			inline bool alt()const{return _alt;}
		protected:
			int _code;
			bool _shift;
			bool _ctrl;
			bool _numlock;
			bool _capslock;
			bool _alt;
			bool is_valid;
	};
}

#endif // __KEY_INPUT__


