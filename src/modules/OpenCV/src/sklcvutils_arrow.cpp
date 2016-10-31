#include "sklcvutils.h"

namespace skl{

	/* private subfunctions */
	void _cvDrawArrowOption(
		cv::Mat& img,
		cv::Point pt,
		const cv::Scalar& color,
		int thickness,
		int lineType,
		ArrowType arrowType,
		int size,
		double degree_offset,
		int degree,
		int shift);

	cv::Point _getRelativePoint(
		cv::Point pt,
		double size,
		double degree);

	/* defininition of the public function */
	void arrow(
		cv::Mat& img,
		cv::Point pt1,
		cv::Point pt2,
		const cv::Scalar& color,
		int thickness,
		int lineType,
		ArrowType head_type,
		ArrowType tail_type,
		int head_size,
		int tail_size,
		int head_degree,
		int tail_degree,
		int shift){

			cv::line(img,pt1,pt2,color,thickness,lineType,shift);

			double degree_offset = atan( (double)(pt1.y - pt2.y)/(pt1.x - pt2.x) );
			// inverse direction (if arrow directs ->, then degree_offset is <-.)
			if(pt1.x < pt2.x){
				degree_offset += CV_PI;
			}

			_cvDrawArrowOption(
				img,
				pt2,
				color,
				thickness,
				lineType,
				head_type,
				head_size,
				degree_offset,
				head_degree,
				shift);
			_cvDrawArrowOption(
				img,
				pt1,
				color,
				thickness,
				lineType,
				tail_type,
				tail_size,
				degree_offset + CV_PI,
				tail_degree,
				shift);
	}

	/* definitions of the private subfunctions */
	void _cvDrawArrowOption(
		cv::Mat& img,
		cv::Point pt,
		const cv::Scalar& color,
		int thickness,
		int lineType,
		ArrowType arrowType,
		int size,
		double degree_offset,
		int degree,
		int shift){

			if(arrowType==NONE) return;

			// CIRCLE and CIRCLE_FILL
			if(arrowType == CIRCLE){
				cv::circle(img,pt,size,color,thickness,lineType,shift);
				return;
			}
			if(arrowType == CIRCLE_FILL){
				cv::circle(img,pt,size,color,-1,lineType,shift);
				return;
			}

			cv::Point points[4];

			// ARROW and ARROW_FILL
			if(arrowType == ARROW || arrowType == ARROW_FILL
				|| arrowType == INV_ARROW || arrowType == INV_ARROW_FILL){
					double _degree;
					if(arrowType == ARROW || arrowType == ARROW_FILL){
						_degree = CV_PI*(double)degree/180.0;
					}
					else{
						_degree = CV_PI + CV_PI*(double)degree/180.0;
					}
					points[0] = _getRelativePoint(pt,size,degree_offset + _degree);
					points[1] = _getRelativePoint(pt,size,degree_offset - _degree);
					switch(arrowType){
					case ARROW:
					case INV_ARROW:
						cv::line(img,pt,points[0],color,thickness,lineType,shift);
						cv::line(img,pt,points[1],color,thickness,lineType,shift);
						return;
					case ARROW_FILL:
					case INV_ARROW_FILL:
						points[2] = pt;
						cv::fillConvexPoly(img,points,3,color,lineType,shift);
						return;
					default:
						break;
					}
			}

			// SQUARE, SQUARE_FILL, DIAMOND and DIAMOND_FILL
			if(arrowType == SQUARE || arrowType == SQUARE_FILL){
				double _45deg = CV_PI/4;
				points[0] = _getRelativePoint(pt,size,degree_offset + _45deg);
				points[1] = _getRelativePoint(pt,size,degree_offset - _45deg);
				points[2] = _getRelativePoint(pt,size,degree_offset + CV_PI + _45deg);
				points[3] = _getRelativePoint(pt,size,degree_offset + CV_PI - _45deg);

			}
			else if(arrowType == DIAMOND || arrowType == DIAMOND_FILL){
				double _90deg = CV_PI/2;
				points[0] = _getRelativePoint(pt,size,degree_offset + _90deg);
				points[1] = _getRelativePoint(pt,size,degree_offset + CV_PI);
				points[2] = _getRelativePoint(pt,size,degree_offset - _90deg);
				points[3] = _getRelativePoint(pt,size,degree_offset);
			}
			else if(arrowType == ABS_SQUARE || arrowType == ABS_SQUARE_FILL){
				double _45deg = CV_PI/4;
				points[0] = _getRelativePoint(pt,size,_45deg);
				points[1] = _getRelativePoint(pt,size,- _45deg);
				points[2] = _getRelativePoint(pt,size,CV_PI + _45deg);
				points[3] = _getRelativePoint(pt,size,CV_PI - _45deg);
			}
			else if(arrowType == ABS_DIAMOND || arrowType == ABS_DIAMOND_FILL){
				double _90deg = CV_PI/2;
				points[0] = _getRelativePoint(pt,size,_90deg);
				points[1] = _getRelativePoint(pt,size,CV_PI);
				points[2] = _getRelativePoint(pt,size,- _90deg);
				points[3] = _getRelativePoint(pt,size,0);
			}

			switch(arrowType){
			case SQUARE:
			case DIAMOND:
			case ABS_SQUARE:
			case ABS_DIAMOND:
				cv::line(img,points[0],points[1],color,thickness,lineType,shift);
				cv::line(img,points[1],points[2],color,thickness,lineType,shift);
				cv::line(img,points[2],points[3],color,thickness,lineType,shift);
				cv::line(img,points[3],points[0],color,thickness,lineType,shift);
				return;
			case SQUARE_FILL:
			case DIAMOND_FILL:
			case ABS_SQUARE_FILL:
			case ABS_DIAMOND_FILL:
				cv::fillConvexPoly(img,points,4,color,lineType,shift);
				return;
			default:
				break;
			}

			// Unknown ArrowType
			return;
	}

	cv::Point _getRelativePoint(
		cv::Point pt,
		double size,
		double degree){
			double dx = size * cos(degree);
			double dy = size * sin(degree);
			cv::Point dist = pt;
			dist.x += (int)dx;
			dist.y += (int)dy;
			return dist;
	}

} // namespace skl
