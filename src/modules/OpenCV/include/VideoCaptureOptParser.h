#ifdef USE_VIDEO_CAPTURE_OPT_PARSER
#ifndef __SKL__VIDEO_CAPTURE_OPT_PARSER_H__
#define __SKL__VIDEO_CAPTURE_OPT_PARSER_H__

#define opt_on_cam_prop(PROP_NAME) opt_on(double, PROP_NAME, -DBL_MAX,"","<PROP_VAL>", std::string("set camera parameter ") + std::string(#PROP_NAME))
namespace skl{
	namespace options{
		opt_on_cam_prop(POS_MSEC);
		opt_on_cam_prop(POS_FRAMES);
		opt_on_cam_prop(POS_AVI_RATIO);
		opt_on_cam_prop(FRAME_WIDTH);
		opt_on_cam_prop(FRAME_HEIGHT);
		opt_on_cam_prop(FPS);
		opt_on_cam_prop(FOURCC);
		opt_on_cam_prop(FRAME_COUNT);
		opt_on_cam_prop(FORMAT);
		opt_on_cam_prop(MODE);
		opt_on_cam_prop(BRIGHTNESS);
		opt_on_cam_prop(CONTRAST);
		opt_on_cam_prop(SATURATION);
		opt_on_cam_prop(HUE);
		opt_on_cam_prop(GAIN);
		opt_on_cam_prop(EXPOSURE);
		opt_on_cam_prop(CONVERT_RGB);
		opt_on_container(std::vector, double, WHITE_BALANCE, "0.0:0.0","","<BLUE:RED>", "set camera parameter WHITE_BALANCE",":",2);
		opt_on_cam_prop(RECTIFICATION);
		opt_on_cam_prop(MONOCROME);
		opt_on_cam_prop(SHARPNESS);
		opt_on_cam_prop(AUTO_EXPOSURE);
		opt_on_cam_prop(GAMMA);
		opt_on_cam_prop(TEMPERATURE);
		opt_on_cam_prop(TRIGGER);
		opt_on_cam_prop(TRIGGER_DELAY);

		// Options for FlyCapture2
		opt_on_cam_prop(IRIS);
		opt_on_cam_prop(FOCUS);
		opt_on_cam_prop(ZOOM);
		opt_on_cam_prop(PAN);
		opt_on_cam_prop(TILT);
		opt_on_cam_prop(SHUTTER);
	}//namespace options
}//namespace params


#define opt_set_cap_prop(prop_name,params) \
	if(skl::options::prop_name!=-DBL_MAX){ assert(params.set(skl::prop_name,skl::options::prop_name));}

#define opt_parse_cap_prop(params) \
	opt_set_cap_prop(POS_MSEC,params)\
opt_set_cap_prop(POS_FRAMES,params)\
opt_set_cap_prop(POS_AVI_RATIO,params)\
opt_set_cap_prop(FRAME_WIDTH,params)\
opt_set_cap_prop(FRAME_HEIGHT,params)\
opt_set_cap_prop(FPS,params)\
opt_set_cap_prop(FOURCC,params)\
opt_set_cap_prop(FRAME_COUNT,params)\
opt_set_cap_prop(FORMAT,params)\
opt_set_cap_prop(MODE,params)\
opt_set_cap_prop(BRIGHTNESS,params)\
opt_set_cap_prop(CONTRAST,params)\
opt_set_cap_prop(SATURATION,params)\
opt_set_cap_prop(HUE,params)\
opt_set_cap_prop(GAIN,params)\
opt_set_cap_prop(EXPOSURE,params)\
opt_set_cap_prop(CONVERT_RGB,params)\
if(skl::options::WHITE_BALANCE.size()==2 && skl::options::WHITE_BALANCE[0]!=0.0 && skl::options::WHITE_BALANCE[1]!=0.0){\
	assert(params.set(skl::WHITE_BALANCE_BLUE_U,skl::options::WHITE_BALANCE[0]));\
	assert(params.set(skl::WHITE_BALANCE_RED_V,skl::options::WHITE_BALANCE[1]));\
}\
opt_set_cap_prop(RECTIFICATION,params)\
opt_set_cap_prop(MONOCROME,params);\
opt_set_cap_prop(SHARPNESS,params);\
opt_set_cap_prop(AUTO_EXPOSURE,params);\
opt_set_cap_prop(GAMMA,params);\
opt_set_cap_prop(TEMPERATURE,params);\
opt_set_cap_prop(TRIGGER,params);\
opt_set_cap_prop(TRIGGER_DELAY,params);\


#endif
#endif
