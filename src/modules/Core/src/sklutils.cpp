#include "sklutils.h"
/**** 環境依存 ****/
/**** WIN32 ****/
#ifdef _WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#define NOMINMAX
#include <windows.h>
/**** Linux ****/
#elif __linux__
#include <time.h>
#endif
/**** ifdef linux ****/
namespace skl{
	/*
	 * @brief 平均0,標準偏差1のガウス分布を作成する
	 * */
	double gauss_rand(){
		double v1,v2,s;
		do{
			double u1 = static_cast<double>(rand())/RAND_MAX;
			double u2 = static_cast<double>(rand())/RAND_MAX;
			v1 = 2*u1-1;
			v2 = 2*u2-1;
			s = v1*v1 + v2*v2;
		}while(s>=1||s<0);

		return v1*sqrt(-2*log(s)/s);
	}

	double rayleigh_rand(double sigma){
		return sqrt(pow(gauss_rand()*sigma,2)+pow(gauss_rand()*sigma,2));
	}

	float radian(float x, float y, float offset_radian,float origin_return_value){
		if(x==0 && y==0) return origin_return_value;
		float rad = atan2f(y,x);
		rad += offset_radian;
		while(rad>2*M_PI){
			rad -= static_cast<float>(2*M_PI);
		}
		while(rad<0){
			rad += static_cast<float>(2*M_PI);
		}
		return rad;
	}

	void sleep(unsigned long msec){
		/**** WIN32 ****/
#ifdef WIN32
		// WIN側の動作確認はしてません。動作が確認できたらここを外してください。
		Sleep(msec);

		/**** Linux ****/
#elif __linux__
			struct timespec tc;
		tc.tv_sec = msec/1000;
		tc.tv_nsec = (msec%1000)*1000;// nano sec
		nanosleep(&tc,NULL);
#endif	/**** ifdef linux ****/
	}

}
