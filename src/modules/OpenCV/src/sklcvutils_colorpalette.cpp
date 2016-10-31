#include "sklcvutils.h"

#define HUE_MAX 360.f
#define SAT_MAX 100.f
#define VAL_MAX 100.f
#define BGR_MAX 255
// local functions
cv::Scalar convHSV2BGR(double hue,double sat,double val);

namespace skl{


	void fillColorPalette(
			std::vector<cv::Scalar>& palette,
			size_t hue_pattern_num,
			bool use_gray,
			int min_luminance_value){
		if(palette.empty()) return;
		float sat = SAT_MAX;

		size_t cycle = hue_pattern_num+use_gray;

		size_t palette_size = palette.size();
		size_t val_pattern_num = (palette_size+cycle-1)/cycle;
		float val_step = (VAL_MAX-min_luminance_value)/val_pattern_num;
		float hue_step = HUE_MAX/hue_pattern_num;

		for(size_t v=0,p=0;v<val_pattern_num;v++){
			float val = VAL_MAX - val_step * v;
			for(size_t h=0;h<cycle && p < palette.size();h++,p++){
				if(h==0 && use_gray){
					int _val = static_cast<int>((val/VAL_MAX)*BGR_MAX);
					palette[p] = cv::Scalar(_val,_val,_val);
				}
				float hue = hue_step * h;
				palette[p] = convHSV2BGR(hue,sat,val);
			}
		}
	}

}

cv::Scalar convHSV2BGR(double h,double s,double v){
	unsigned char r,g,b;
	b=g=r=0;

	unsigned char hi,p,q,t;
	double ds=s/SAT_MAX;
	double f;
	hi=((int)h / 60) % 6;
	f=(double)h*2/60-(double)hi;
	p=static_cast<unsigned char>(v*(1-ds));
	q=static_cast<unsigned char>(v*(1-(f*ds)));
	t=static_cast<unsigned char>(v*(1-((1-f)*ds)));

	switch(hi){
		case 0:
			r=(unsigned char)v;g=(unsigned char)t;b=(unsigned char)p;
			break;
		case 1:
			r=(unsigned char)q;g=(unsigned char)v;b=(unsigned char)p;
			break;
		case 2:
			r=(unsigned char)p;g=(unsigned char)v;b=(unsigned char)t;
			break;
		case 3:
			r=(unsigned char)p;g=(unsigned char)q;b=(unsigned char)v;
			break;
		case 4:
			r=(unsigned char)t;g=(unsigned char)p;b=(unsigned char)v;
			break;
		case 5:
			r=(unsigned char)v;g=(unsigned char)p;b=(unsigned char)q;
			break;
	}
	return cv::Scalar(b,g,r);
}
