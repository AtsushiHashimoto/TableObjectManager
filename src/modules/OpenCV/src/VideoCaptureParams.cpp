/*!
 * @file VideoCaptureParams.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/18
 * @date Last Change: 2012/Sep/27.
 */

// C++
#include <fstream>

// SKL Core Module
#include "skl.h"

// SKL OpenCV Module
#include "cvtypes.h"
#include "VideoCaptureParams.h"

using namespace skl;

std::map<std::string,capture_property_t> VideoCaptureParams::property_name_id_map;

#define set_name_id_map( prop_name ) property_name_id_map[#prop_name] = prop_name

/*!
 * @brief デフォルトコンストラクタ
 */
VideoCaptureParams::VideoCaptureParams(){
	set_name_id_map(POS_MSEC);
	set_name_id_map(POS_FRAMES);
	set_name_id_map(POS_AVI_RATIO);
	set_name_id_map(FRAME_WIDTH);
	set_name_id_map(FRAME_HEIGHT);
	set_name_id_map(FPS);
	set_name_id_map(FOURCC);
	set_name_id_map(FRAME_COUNT);
	set_name_id_map(FORMAT);
	set_name_id_map(MODE);
	set_name_id_map(BRIGHTNESS);
	set_name_id_map(CONTRAST);
	set_name_id_map(SATURATION);
	set_name_id_map(HUE);
	set_name_id_map(GAIN);
	set_name_id_map(EXPOSURE);
	set_name_id_map(CONVERT_RGB);
	set_name_id_map(WHITE_BALANCE_BLUE_U); // reserved parameter by OpenCV
	set_name_id_map(RECTIFICATION);
	set_name_id_map(MONOCROME);
	set_name_id_map(SHARPNESS);
	set_name_id_map(AUTO_EXPOSURE);
	set_name_id_map(GAMMA);
	set_name_id_map(TEMPERATURE);
	set_name_id_map(TRIGGER);
	set_name_id_map(TRIGGER_DELAY);
	set_name_id_map(WHITE_BALANCE_RED_V);
}

/*!
 * @brief デストラクタ
 */
VideoCaptureParams::~VideoCaptureParams(){

}
VideoCaptureParams::VideoCaptureParams(const std::string& filename){load(filename);}
VideoCaptureParams::VideoCaptureParams(const VideoCaptureParams& other){
	property_name_id_map = other.property_name_id_map;
	property_id_value_map = other.property_id_value_map;
}

bool VideoCaptureParams::set(const std::string& prop_name,double val){
	std::map<std::string,capture_property_t>::iterator pp = property_name_id_map.find(prop_name);
	if(property_name_id_map.end()==pp) return false;
	return set(pp->second,val);
}
bool VideoCaptureParams::set(capture_property_t prop_id,double val){
	property_id_value_map[prop_id] = val;
	return true;
}

double VideoCaptureParams::get(const std::string& prop_name)const{
	std::map<std::string,capture_property_t>::const_iterator pp = property_name_id_map.find(prop_name);
	if(property_name_id_map.end()==pp) return 0.0;
	return get(pp->second);
}

double VideoCaptureParams::get(capture_property_t prop_id)const{
	VideoCaptureParamIter pp = property_id_value_map.find(prop_id);
	if(property_id_value_map.end() == pp){
		return 0.0;
	}
	return pp->second;
}

VideoCaptureParamIter VideoCaptureParams::begin()const{return property_id_value_map.begin();}

VideoCaptureParamIter VideoCaptureParams::end()const{return property_id_value_map.end();}

std::string VideoCaptureParams::print()const{
	std::stringstream ss;
	for(std::map<std::string,capture_property_t>::const_iterator iter = property_name_id_map.begin();
			iter != property_name_id_map.end();iter++){
		VideoCaptureParamIter pval = property_id_value_map.find(iter->second);
		if(property_id_value_map.end()==pval)continue;
		ss << iter->first << ": " << pval->second << std::endl;
	}
	return ss.str();
}

bool VideoCaptureParams::scan(const std::string& str){
	std::vector<std::string> buf = split(str,"\n");
	for(size_t i=0;i<buf.size();i++){
		std::vector<std::string> key_val = split(buf[i],":");
		if(key_val.size()!=0) continue;
		if( !set( key_val[0], atof(key_val[1].c_str()) ) ){
			return false;
		}
	}
	return true;
}

bool VideoCaptureParams::load(const std::string& filename){
	std::map<std::string,std::string> param_map;
	if(!parse_conffile(filename,param_map,":")) return false;
	for(std::map<std::string,std::string>::iterator iter = param_map.begin();
			iter != param_map.end();iter++){
		assert(set(iter->first,atof(iter->second.c_str())));
	}
	return true;
}

void VideoCaptureParams::save(const std::string& filename)const{
	std::ofstream fout;
	fout.open(filename.c_str());
	fout << print();
	fout.close();
}

