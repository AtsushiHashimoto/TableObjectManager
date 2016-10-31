#ifndef __SKL_UTILS__
#define __SKL_UTILS__

#include <cstdlib>
#include <cmath>
namespace skl{
	double gauss_rand();
	double rayleigh_rand(double sigma);
#ifdef _DEBUG
#define printLocation() std::cerr << "in " << __FILE__ << ": at " << __LINE__ << std::endl
#else
#define printLocation() 
#endif
	void sleep(unsigned long msec);

	float radian(float x,float y, float offset_radian=0.f,float origin_return_value=-1);
	template<class Point> float radian(const Point& pt, float offset_radian=0.f,float origin_return_value=-1){
		return radian(static_cast<float>(pt.x),static_cast<float>(pt.y),offset_radian,origin_return_value);
	}
}
#endif // __SKL_UTILS__
