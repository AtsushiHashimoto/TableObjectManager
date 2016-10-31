#include "sklcvutils.h"
cv::Rect operator&(const cv::Rect& left, const cv::Rect& right){
	cv::Rect rect(left.x,left.y,left.x+left.width,left.y+left.height);
	rect.x = rect.x > right.x ? rect.x : right.x;
	rect.y = rect.y > right.y ? rect.y : right.y;
	rect.width = rect.width < right.x + right.width ? rect.width : right.x + right.width;
	rect.height = rect.height < right.y + right.height ? rect.height : right.y + right.height;
	rect.width -= rect.x;
	rect.height -= rect.y;
	return rect;
}
bool operator&&(const cv::Rect& left, const cv::Rect& right){
	cv::Rect rect = left & right;
	return (rect.width > 0) && (rect.height > 0);
}

cv::Rect operator|(const cv::Rect& left, const cv::Rect& right){
	cv::Rect rect(left.x,left.y,left.x+left.width,left.y+left.height);
	rect.x = rect.x < right.x ? rect.x : right.x;
	rect.y = rect.y < right.y ? rect.y : right.y;
	rect.width = rect.width > right.x + right.width ? rect.width : right.x + right.width;
	rect.height = rect.height > right.y + right.height ? rect.height : right.y + right.height;
	rect.width -= rect.x;
	rect.height -= rect.y;
	return rect;
}