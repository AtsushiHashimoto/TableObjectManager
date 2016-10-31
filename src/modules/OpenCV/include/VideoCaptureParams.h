/*!
 * @file VideoCaptureParams.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/18
 * @date Last Change:2012/Sep/27.
 */
#ifndef __SKL_VIDEO_CAPTURE_PARAMS_H__
#define __SKL_VIDEO_CAPTURE_PARAMS_H__

// STL
#include <map>

// SKL Core Module
#include "skl.h"

// SKL OpenCV Module
#include "cvtypes.h"

namespace skl{

	typedef std::map<capture_property_t,double>::const_iterator VideoCaptureParamIter;

	/*!
	 * @brief VideoCaptureクラスの取るパラメタをあらわす値オブジェクト
	 */
	class VideoCaptureParams: public Printable<VideoCaptureParams>{
		friend class VideoParamAdder;
		public:
		VideoCaptureParams();
		virtual ~VideoCaptureParams();
		VideoCaptureParams(const std::string& filename);
		VideoCaptureParams(const VideoCaptureParams& other);

		std::string print()const;
		bool scan(const std::string& buf);

		bool load(const std::string& filename);
		void save(const std::string& filename)const;

		bool set(const std::string& prop_name, double val);
		bool set(capture_property_t prop_id,double val);

		double get(const std::string& prop_name)const;
		double get(capture_property_t prop_id)const;

		VideoCaptureParamIter begin()const;
		VideoCaptureParamIter end()const;

		inline const std::map<std::string,capture_property_t>& getPropertyNameIDMap(){return property_name_id_map;}
		protected:

		static std::map<std::string,capture_property_t> property_name_id_map;
		std::map<capture_property_t,double> property_id_value_map;

		private:

	};

} // skl

#endif // __SKL_VIDEO_CAPTURE_PARAMS_H__

