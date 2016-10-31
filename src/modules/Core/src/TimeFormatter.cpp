/*!
 * @file TimeFormatter.cpp
 * @author Takahrio Suzuki
 * @date Last Change:2012/Jan/06.
 **/

/**** Header ****/
/**** skl ****/
#include "TimeFormatter.h"
/**** C++ ****/
#include <iostream>
#include <sstream>
/**** C ****/
#include <ctime>
/***** OS ****/

///////////////////////////////////////////////////////////
namespace skl{
	// Constructor
	TimeFormatter::TimeFormatter(const std::string& name):name(name){}
	// Copy Constructor
	TimeFormatter::TimeFormatter(const TimeFormatter& other):name(other.name){};
	// Destructor
	TimeFormatter::~TimeFormatter(){}
	/////////////////////////////////////////////////////////// 
	// Functions
	// オブジェクトを複製する
	TimeFormatter* TimeFormatter::clone() const{
		return 0;
	}
	// 等価なオブジェクトか？
	bool TimeFormatter::equals(const TimeFormatter& other)const{
		return (this == &other);
	}

	std::string TimeFormatter::getName()const{
		return name;
	}
}
