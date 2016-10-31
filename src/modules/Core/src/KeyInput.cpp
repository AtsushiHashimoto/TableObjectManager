/*!
 * @file KeyInput.cpp
 *
 * @author 橋本敦史
 * @date Last Change:2012/Oct/14.
 * */

#include "KeyInput.h"

using namespace std;

using namespace skl;

	/*
	 * @brief コンストラクタ
	 * */
	KeyInput::KeyInput(){
		is_valid = false;
	}

	/*
	 * @brief デストラクタ
	 * */
	KeyInput::~KeyInput(){}

	/*
	 * @brief cvWaitKeyを通して入力されたキーを解析する
	 * @param input cvWaitKey()の返り値
	 * @return shift等を無視した文字だけの返り値(shift等の情報も欲しければgetKey()を使う)
	 * */
	char KeyInput::set(int input){
		_code = input;
		_numlock = input / NUMLOCK == 0 ? true : false;
		if(!_numlock)	input -= NUMLOCK;

		_alt = input / ALT == 0 ? false : true;
		if(_alt)	input -= ALT;

		_ctrl = input / CTRL == 0 ? false : true;
		if(_ctrl)	input -= CTRL;

		_capslock = input / CAPSLOCK == 0 ? false : true;
		if(_capslock)	input -= CAPSLOCK;

		_shift = input / SHIFT == 0 ? false : true;
		if(_shift)	input -= SHIFT;

		char __char = getChar();
		is_valid = (__char > 0);
		return __char;
}

