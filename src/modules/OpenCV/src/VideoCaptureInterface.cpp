/*!
 * @file VideoCaptureInterface.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/12
 * @date Last Change: 2013/Dec/02.
 */
#include "VideoCaptureInterface.h"

using namespace skl;

_VideoCaptureInterface::_VideoCaptureInterface(){}
_VideoCaptureInterface::~_VideoCaptureInterface(){}


bool _VideoCaptureInterface::set(const VideoCaptureParams& params){
	bool any_success = false;
	for(VideoCaptureParamIter iter = params.begin();
			iter != params.end();iter++){
		if(!this->set(iter->first,iter->second)) continue;
		any_success = true;
	}
	return any_success;
}

bool _VideoCaptureInterface::set(const std::string& prop_name, double val){
	std::map<std::string,capture_property_t>::const_iterator pp = params.getPropertyNameIDMap().find(prop_name);
	if(params.getPropertyNameIDMap().end() == pp) return false;

	return set(pp->second,val);
}


bool _VideoCaptureInterface::set(const std::string& prop_name, camera_mode_t mode){
	std::map<std::string,capture_property_t>::const_iterator pp = params.getPropertyNameIDMap().find(prop_name);
	if(params.getPropertyNameIDMap().end() == pp) return false;
	return set(pp->second,mode);
}

bool _VideoCaptureInterface::set(capture_property_t prop_id,camera_mode_t mode){
	double val = params.get(prop_id);
	bool success = set(prop_id, static_cast<double>(mode));
	success &= params.set(prop_id,val);
	return success;
}


const VideoCaptureParams& _VideoCaptureInterface::get(){
	for(std::map<std::string,capture_property_t>::const_iterator iter = params.getPropertyNameIDMap().begin();
			iter!= params.getPropertyNameIDMap().end();iter++){
		double val = this->get(iter->second);
		if(val==0.0) continue;
		assert(params.set(iter->second,val));
	}
	return params;
}


double _VideoCaptureInterface::get(const std::string& prop_name){
	std::map<std::string,capture_property_t>::const_iterator pp =  params.getPropertyNameIDMap().find(prop_name);
	if(params.getPropertyNameIDMap().end() == pp) return 0;
	return get(pp->second);
}


