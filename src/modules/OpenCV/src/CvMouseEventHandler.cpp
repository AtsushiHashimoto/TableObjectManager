/*!
 * @file CvMouseEventHandler.cpp
 *
 * OpenCV Window上でのマウスイベントをMouseCallback関数の外に持ち出すClass
 * @author 橋本敦史
 * @date Last Change:2012/Oct/15.
 * */

#include "CvMouseEventHandler.h"

namespace skl{

	/*!
	 * @brief コンストラクタ
	 * */
	CvMouseEventHandler::CvMouseEventHandler(const std::string& win_name){
		cvSetMouseCallback (win_name.c_str(), this->onMouse, (void *)this);
	}

	/*!
	 * @brief デストラクタ
	 * */
	CvMouseEventHandler::~CvMouseEventHandler(){}

	/*!
	 * @brief cvSetMouseCallbackの第二引数としてセットするコールバック関数
	 * @param event マウスイベント(OpenCV側から与えられる)
	 * @param x マウスのx座標(OpenCV側から与えられる)
	 * @param y マウスのy座標(OpenCV側から与えられる)
	 * @param flag マウスの状態フラグ(OpenCV側から与えられる)
	 * @param params 外部で定義されたobjectへのポインタ(ここではMouseEventHandlerを想定)
	 * */
	void CvMouseEventHandler::onMouse(int event_type, int x, int y, int flag, void *params){
		CvMouseEventHandler *md = (CvMouseEventHandler*)params;
		// マウスの現在地を獲得
		md->_location = cv::Point(x,y);

		// set flags
		md->_flag = flag;

		if(event_type == CV_EVENT_MOUSEMOVE) return;
		MouseEvent event;
		event.type = event_type;
		event.flag = flag;
		event.location = md->location();
		md->_events.push(event);
	}
}
