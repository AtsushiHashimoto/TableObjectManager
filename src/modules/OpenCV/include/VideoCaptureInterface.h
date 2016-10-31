/*!
 * @file VideoCaptureInterface.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/18
 * @date Last Change:2013/Dec/02.
 */
#ifndef __SKL_VIDEO_CAPTURE_INTERFACE_H__
#define __SKL_VIDEO_CAPTURE_INTERFACE_H__

// OpenCV
#include <cv.h>

// SKL
#include "skl.h"

// SKL OpenCV Module
#include "VideoCaptureParams.h"


namespace skl{

	/*!
	 * @brief SKLにおけるVideoCapture用のインターフェイス
	 * @comment 実際にVideoCaptureを作るときには,この後で宣言されているoperator<<を持ったテンプレートつきインターフェイスVideoCapture<T>を継承すること
	 * */
	class _VideoCaptureInterface:public SensorModuleBase<cv::Mat>{
		public:
			_VideoCaptureInterface();
			~_VideoCaptureInterface();
			virtual bool open(const std::string& filename)=0;
			virtual bool open(int device)=0;
			virtual bool isOpened()const=0;
			virtual void release()=0;
			virtual bool grab()=0;
			virtual bool retrieve(cv::Mat& image, int channel=0)=0;
			inline size_t size()const{return isOpened();}

			//! 本来は取得データがないのに補間したデータなどを利用している
			//!  場合にtrueとなる。具象クラスで特に実装がなければfalseを返す
			virtual bool isPseudoCapture()const{return false;}

			//! カメラに値やモード(camera_mode_tがset(*,camera_mode_t mode)を通してvalに与えられる(modeは-4から-1までの整数)をセットする純粋仮想関数
			virtual bool set(capture_property_t prop_id,double val)=0;
			//! カメラから値をgetする純粋仮想関数(modeはインターフェイス簡素化のためサポートしない)
			virtual double get(capture_property_t prop_id)=0;

			// 純粋仮想関数bool set(capture_property_t prop_id, double val)を
			// 定義することで使えるようになる関数
			bool set(const VideoCaptureParams& params);
			bool set(const std::string& prop_name, double val);

			bool set(const std::string& prop_name,camera_mode_t mode);
			bool set(capture_property_t prop_id,camera_mode_t mode);

			// 純粋仮想関数double get(capture_proeprty_t prop_id)を
			// 提議することで使えるようになる関数
			const VideoCaptureParams& get();
			double get(const std::string& prop_name);
		protected:
			VideoCaptureParams params;
	};


	/*!
	 * @brief SKLにおけるVideoCapture用のインターフェイス(operator>>つき)
	 */
	template<class T> class VideoCaptureInterface: public _VideoCaptureInterface{
		public:
			VideoCaptureInterface();
			virtual ~VideoCaptureInterface();
/*
			// pure virtual functions from _VideoCaptureInterface
			virtual bool open(const std::string& filename)=0;
			virtual bool open(int device=0)=0;
			virtual bool isOpened()const=0;
			virtual void release()=0;
			virtual bool grab()=0;
			virtual bool retrieve(cv::Mat& image, int channel=0)=0;
			virtual bool set(capture_property_t prop_id,double val)=0;
			virtual double get(capture_property_t prop_id)=0;
*/
			// a function with Template T
			virtual inline T& operator >> (cv::Mat& image){
				if(!grab()){
					image.release();
				}
				else{
					retrieve(image,0);
				}
				return *dynamic_cast<T*>(this);
			}
		protected:
	};

	template<class T> VideoCaptureInterface<T>::VideoCaptureInterface():_VideoCaptureInterface(){}
	template<class T> VideoCaptureInterface<T>::~VideoCaptureInterface(){}


} // skl

#endif // __SKL_VIDEO_CAPTURE_INTERFACE_H__

