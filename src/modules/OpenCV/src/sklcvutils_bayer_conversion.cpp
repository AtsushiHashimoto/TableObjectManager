#ifndef __SKL_CV_UTILS_BAYER_CONVERSION_H__
#define __SKL_CV_UTILS_BAYER_CONVERSION_H__

#include <cv.h>
#include "sklcvutils_bayer_conversion.h"

#define CLIP(in, out)\
	in = in < 0 ? 0 : in;\
	in = in > 255 ? 255 : in;\
	out=in;

namespace skl{
	/*!
	* @brief calc HLS value
	* */
	unsigned char getHLSvalue(int min,int max,unsigned char hue){
		if(max>255)max=255;if(min>255)min=255;
		if(hue<=30) return (unsigned char)(min+(max-min)*hue/30);
		else if(hue<=90)return (unsigned char)max;
		else if(hue<=120) return (unsigned char)(min+(max-min)*(120-hue)/30);
		else return (unsigned char)min;
	}

	cv::Vec3b convHLS2BGR(const cv::Vec3b& hls){
		unsigned char r,g,b,h,l,s;
		h=hls[0];
		l=hls[1];
		s=hls[2];

		int max,min;
		int hue;

		if(l<=128) max=(l*(255+(int)s))/255;
		else max=(l*(255-s)+s)/255;
		min=2*l-max;

		if(s==0)r=g=b=l;
		else{
			hue=(int)h+60;
			if(hue>=180) hue=hue-180;
			else if(hue<0) hue=hue+180;
			r=getHLSvalue(min,max,hue);

			hue=(int)h;
			if(hue>=180) hue=hue-180;
			else if(hue<0) hue=hue+180;
			g=getHLSvalue(min,max,hue);

			hue=(int)h-60;
			if(hue>=180) hue=hue-180;
			else if(hue<0) hue=hue+180;
			b=getHLSvalue(min,max,hue);
		}

		cv::Vec3b bgr;
		bgr[0]=b;
		bgr[1]=g;
		bgr[2]=r;
		return bgr;
	}

	void cvtBayer2BGR_NN(const cv::Mat& bayer, cv::Mat& bgr, int code);
	void cvtBayer2BGR_EDGESENSE(const cv::Mat& bayer, cv::Mat& bgr, int code);

	void cvtBayer2BGR(const cv::Mat& _bayer, cv::Mat& bgr, int code, int algo_type){
		cv::Mat bayer;
		if(!_bayer.isContinuous()){
			bayer = cv::Mat(_bayer.size(),_bayer.type());
			_bayer.copyTo(bayer);
		}
		else{
			bayer = _bayer;
		}
		assert(bayer.isContinuous());
		bgr = cv::Mat::zeros(bayer.rows,bayer.cols,CV_8UC3);
		if(algo_type == BAYER_SIMPLE){
			cv::cvtColor(bayer,bgr,code);
		}
		else if(algo_type == BAYER_NN){
			cvtBayer2BGR_NN(bayer,bgr,code);
		}
		else if(algo_type == BAYER_EDGE_SENSE){
			cvtBayer2BGR_EDGESENSE(bayer,bgr,code);
		}
	}

	void cvtBayer2BGR_NN(const cv::Mat& bayer, cv::Mat& bgr, int type){
		int sx = bayer.cols;
		int sy = bayer.rows;
		const unsigned char* src = bayer.ptr<unsigned char>(0);
		unsigned char* dest = bgr.ptr<unsigned char>(0);

		unsigned char *outR=NULL, *outG=NULL, *outB=NULL;
		register int i,j;

		// sx and sy should be even
		switch (type) {
			case CV_BayerGR2BGR:
			case CV_BayerBG2BGR:
				outR=&dest[0];
				outG=&dest[1];
				outB=&dest[2];
				break;
			case CV_BayerGB2BGR:
			case CV_BayerRG2BGR:
				outR=&dest[2];
				outG=&dest[1];
				outB=&dest[0];
				break;
			default:
				std::cerr << "Bad Bayer pattern ID: " << type << std::endl;
				return;
				break;
		}

		switch (type) {
			case CV_BayerGR2BGR: //-------------------------------------------
			case CV_BayerGB2BGR:
				// copy original RGB data to output images
				for (i=0;i<sy;i+=2) {
					for (j=0;j<sx;j+=2) {
						outG[(i*sx+j)*3]=src[i*sx+j];
						outG[((i+1)*sx+(j+1))*3]=src[(i+1)*sx+(j+1)];
						outR[(i*sx+j+1)*3]=src[i*sx+j+1];
						outB[((i+1)*sx+j)*3]=src[(i+1)*sx+j];
					}
				}
				// R channel
				for (i=0;i<sy;i+=2) {
					for (j=0;j<sx-1;j+=2) {
						outR[(i*sx+j)*3]=outR[(i*sx+j+1)*3];
						outR[((i+1)*sx+j+1)*3]=outR[(i*sx+j+1)*3];
						outR[((i+1)*sx+j)*3]=outR[(i*sx+j+1)*3];
					}
				}
				// B channel
				for (i=0;i<sy-1;i+=2)  { //every two lines
					for (j=0;j<sx-1;j+=2) {
						outB[(i*sx+j)*3]=outB[((i+1)*sx+j)*3];
						outB[(i*sx+j+1)*3]=outB[((i+1)*sx+j)*3];
						outB[((i+1)*sx+j+1)*3]=outB[((i+1)*sx+j)*3];
					}
				}
				// using lower direction for G channel

				// G channel
				for (i=0;i<sy-1;i+=2)//every two lines
					for (j=1;j<sx;j+=2)
						outG[(i*sx+j)*3]=outG[((i+1)*sx+j)*3];

				for (i=1;i<sy-2;i+=2)//every two lines
					for (j=0;j<sx-1;j+=2)
						outG[(i*sx+j)*3]=outG[((i+1)*sx+j)*3];

				// copy it for the next line
				for (j=0;j<sx-1;j+=2)
					outG[((sy-1)*sx+j)*3]=outG[((sy-2)*sx+j)*3];

				break;
			case CV_BayerBG2BGR: //-------------------------------------------
			case CV_BayerRG2BGR:
				// copy original data
				for (i=0;i<sy;i+=2) {
					for (j=0;j<sx;j+=2) {
						outB[(i*sx+j)*3]=src[i*sx+j];
						outR[((i+1)*sx+(j+1))*3]=src[(i+1)*sx+(j+1)];
						outG[(i*sx+j+1)*3]=src[i*sx+j+1];
						outG[((i+1)*sx+j)*3]=src[(i+1)*sx+j];
					}
				}
				// R channel
				for (i=0;i<sy;i+=2){
					for (j=0;j<sx-1;j+=2) {
						outR[(i*sx+j)*3]=outR[((i+1)*sx+j+1)*3];
						outR[(i*sx+j+1)*3]=outR[((i+1)*sx+j+1)*3];
						outR[((i+1)*sx+j)*3]=outR[((i+1)*sx+j+1)*3];
					}
				}
				// B channel
				for (i=0;i<sy-1;i+=2) { //every two lines
					for (j=0;j<sx-1;j+=2) {
						outB[((i+1)*sx+j)*3]=outB[(i*sx+j)*3];
						outB[(i*sx+j+1)*3]=outB[(i*sx+j)*3];
						outB[((i+1)*sx+j+1)*3]=outB[(i*sx+j)*3];
					}
				}
				// using lower direction for G channel

				// G channel
				for (i=0;i<sy-1;i+=2)//every two lines
					for (j=0;j<sx-1;j+=2)
						outG[(i*sx+j)*3]=outG[((i+1)*sx+j)*3];

				for (i=1;i<sy-2;i+=2)//every two lines
					for (j=0;j<sx-1;j+=2)
						outG[(i*sx+j+1)*3]=outG[((i+1)*sx+j+1)*3];

				// copy it for the next line
				for (j=0;j<sx-1;j+=2)
					outG[((sy-1)*sx+j+1)*3]=outG[((sy-2)*sx+j+1)*3];

				break;

			default:  //-------------------------------------------
				std::cerr << "Bad Bayer pattern ID: " << type << std::endl;
				return;
				break;
		}

	}

	void ClearBorders(unsigned char* dest, int sx, int sy, int w);
	void cvtBayer2BGR_EDGESENSE(const cv::Mat& bayer, cv::Mat& bgr, int type){
		int sx = bayer.cols;
		int sy = bayer.rows;
		const unsigned char* src = bayer.ptr<unsigned char>(0);
		unsigned char* dest = bgr.ptr<unsigned char>(0);
		unsigned char *outR=NULL, *outG=NULL, *outB=NULL;
		register int i,j,idx;
		int dh, dv;
		int tmp;

		// sx and sy should be even
		switch (type) {
			case CV_BayerGR2BGR:
			case CV_BayerBG2BGR:
				outR=&dest[0];
				outG=&dest[1];
				outB=&dest[2];
				break;
			case CV_BayerGB2BGR:
			case CV_BayerRG2BGR:
				outR=&dest[2];
				outG=&dest[1];
				outB=&dest[0];
				break;
			default:
				std::cerr << "Bad Bayer pattern ID: " << type << std::endl;
				return;
				break;
		}

		switch (type) {
			case CV_BayerGR2BGR://---------------------------------------------------------
			case CV_BayerGB2BGR:
				// copy original RGB data to output images
				for (i=0;i<sy;i+=2) {
					for (j=0;j<sx;j+=2) {
						idx = i*sx+j;
						outG[idx*3]=src[idx];
						outR[(idx+1)*3]=src[idx+1];
						idx += sx;
						outB[idx*3]=src[idx];
						outG[(idx+1)*3]=src[idx+1];
					}
				}
				// process GREEN channel
				for (i=3;i<sy-2;i+=2) {
					for (j=2;j<sx-3;j+=2) {
						idx = i*sx+j;
						dh=abs((outB[(idx-2)*3]+outB[(idx+2)*3])/2-outB[idx*3]);
						dv=abs((outB[(idx-2*sx)*3]+outB[(idx+2*sx)*3])/2-outB[(idx)*3]);
						if (dh<dv)
							tmp=(outG[(idx-1)*3]+outG[(idx+1)*3])/2;
						else {
							if (dh>dv)
								tmp=(outG[(idx-sx)*3]+outG[(idx+sx)*3])/2;
							else
								tmp=(outG[(idx-1)*3]+outG[(idx+1)*3]+outG[(idx-sx)*3]+outG[(idx+sx)*3])/4;
						}
						CLIP(tmp,outG[idx*3]);
					}
				}

				for (i=2;i<sy-3;i+=2) {
					for (j=3;j<sx-2;j+=2) {
						idx = i*sx+j;
						dh=abs((outR[(idx-2)*3]+outR[(idx+2)*3])/2-outR[idx*3]);
						dv=abs((outR[(idx-2*sx)*3]+outR[(idx+2*sx)*3])/2-outR[idx*3]);
						if (dh<dv)
							tmp=(outG[(idx-1)*3]+outG[(idx+1)*3])/2;
						else {
							if (dh>dv)
								tmp=(outG[(idx-sx)*3]+outG[(idx+sx)*3])/2;
							else
								tmp=(outG[(idx-1)*3]+outG[(idx+1)*3]+outG[(idx-sx)*3]+outG[(idx+sx)*3])/4;
						} 
						CLIP(tmp,outG[idx*3]);
					}
				}
				// process RED channel
				for (i=0;i<sy-1;i+=2) {
					for (j=2;j<sx-1;j+=2) {
						idx = i*sx+j;
						tmp=outG[idx*3]+(outR[(idx-1)*3]-outG[(idx-1)*3]+
								outR[(idx+1)*3]-outG[(idx+1)*3])/2;
						CLIP(tmp,outR[idx*3]);
					}
				}
				for (i=1;i<sy-2;i+=2) {
					for (j=1;j<sx;j+=2) {
						idx = i*sx+j;
						tmp=outG[idx*3]+(outR[(idx-sx)*3]-outG[(idx-sx)*3]+
								outR[(idx+sx)*3]-outG[(idx+sx)*3])/2;
						CLIP(tmp,outR[idx*3]);
					}
					for (j=2;j<sx-1;j+=2) {
						idx = i*sx+j;
						tmp=outG[idx*3]+(outR[(idx-sx-1)*3]-outG[(idx-sx-1)*3]+
								outR[(idx-sx+1)*3]-outG[(idx-sx+1)*3]+
								outR[(idx+sx-1)*3]-outG[(idx+sx-1)*3]+
								outR[(idx+sx+1)*3]-outG[(idx+sx+1)*3])/4;
						CLIP(tmp,outR[idx*3]);
					}
				}

				// process BLUE channel
				for (i=1;i<sy;i+=2) {
					for (j=1;j<sx-2;j+=2) {
						idx = i*sx+j;
						tmp=outG[idx*3]+(outB[(idx-1)*3]-outG[(idx-1)*3]+
								outB[(idx+1)*3]-outG[(idx+1)*3])/2;
						CLIP(tmp,outB[idx*3]);
					}
				}
				for (i=2;i<sy-1;i+=2) {
					for (j=0;j<sx-1;j+=2) {
						idx = i*sx+j;
						tmp=outG[idx*3]+(outB[(idx-sx)*3]-outG[(idx-sx)*3]+
								outB[(idx+sx)*3]-outG[(idx+sx)*3])/2;
						CLIP(tmp,outB[idx*3]);
					}
					for (j=1;j<sx-2;j+=2) {
						idx = i*sx+j;
						tmp=outG[idx*3]+(outB[(idx-sx-1)*3]-outG[(idx-sx-1)*3]+
								outB[(idx-sx+1)*3]-outG[(idx-sx+1)*3]+
								outB[(idx+sx-1)*3]-outG[(idx+sx-1)*3]+
								outB[(idx+sx+1)*3]-outG[(idx+sx+1)*3])/4;
						CLIP(tmp,outB[idx*3]);
					}
				}
				break;

			case CV_BayerBG2BGR: //---------------------------------------------------------
			case CV_BayerRG2BGR:
				// copy original RGB data to output images
				for (i=0;i<sy;i+=2) {
					for (j=0;j<sx;j+=2) {
						idx = i*sx+j;
						outB[idx*3]=src[idx];
						outG[(idx+1)*3]=src[idx+1];

						idx += sx;
						outG[idx*3]=src[idx];
						outR[(idx+1)*3]=src[idx+1];
					}
				}
				// process GREEN channel
				for (i=2;i<sy-2;i+=2) {
					for (j=2;j<sx-3;j+=2) {
						idx = i*sx+j;
						dh=abs((outB[(idx-2)*3]+outB[(idx+2)*3])/2-outB[idx*3]);
						dv=abs((outB[(idx-2*sx)*3]+outB[(idx+2*sx)*3])/2-outB[idx*3]);
						if (dh<dv)
							tmp=(outG[(idx-1)*3]+outG[(idx+1)*3])/2;
						else {
							if (dh>dv)
								tmp=(outG[(idx-sx)*3]+outG[(idx+sx)*3])/2;
							else
								tmp=(outG[(idx-1)*3]+outG[(idx+1)*3]+outG[(idx-sx)*3]+outG[(idx+sx)*3])/4;
						}
						CLIP(tmp,outG[(idx)*3]);
					}
				}
				for (i=3;i<sy-3;i+=2) {
					for (j=3;j<sx-2;j+=2) {
						idx = i*sx+j;
						dh=abs((outR[(idx-2)*3]+outR[(idx+2)*3])/2-outR[(idx)*3]);
						dv=abs((outR[(idx-2*sx)*3]+outR[(idx+2*sx)*3])/2-outR[(idx)*3]);
						if (dh<dv)
							tmp=(outG[(idx-1)*3]+outG[(idx+1)*3])/2;
						else {
							if (dh>dv)
								tmp=(outG[(idx-sx)*3]+outG[(idx+sx)*3])/2;
							else
								tmp=(outG[(idx-1)*3]+outG[(idx+1)*3]+outG[(idx-sx)*3]+outG[(idx+sx)*3])/4;
						}
						CLIP(tmp,outG[(idx)*3]);
					}
				}
				// process RED channel
				for (i=1;i<sy-1;i+=2) { // G-points (1/2)
					for (j=2;j<sx-1;j+=2) {
						idx = i * sx + j;
						tmp=outG[(i*sx+j)*3]+(outR[(i*sx+j-1)*3]-outG[(i*sx+j-1)*3]+
								outR[(i*sx+j+1)*3]-outG[(i*sx+j+1)*3])/2;
						CLIP(tmp,outR[(i*sx+j)*3]);
					}
				}
				for (i=2;i<sy-2;i+=2)  {
					for (j=1;j<sx;j+=2) { // G-points (2/2)
						tmp=outG[(i*sx+j)*3]+(outR[((i-1)*sx+j)*3]-outG[((i-1)*sx+j)*3]+
								outR[((i+1)*sx+j)*3]-outG[((i+1)*sx+j)*3])/2;
						CLIP(tmp,outR[(i*sx+j)*3]);
					}
					for (j=2;j<sx-1;j+=2) { // B-points
						tmp=outG[(i*sx+j)*3]+(outR[((i-1)*sx+j-1)*3]-outG[((i-1)*sx+j-1)*3]+
								outR[((i-1)*sx+j+1)*3]-outG[((i-1)*sx+j+1)*3]+
								outR[((i+1)*sx+j-1)*3]-outG[((i+1)*sx+j-1)*3]+
								outR[((i+1)*sx+j+1)*3]-outG[((i+1)*sx+j+1)*3])/4;
						CLIP(tmp,outR[(i*sx+j)*3]);
					}
				}

				// process BLUE channel
				for (i=0;i<sy;i+=2) {
					for (j=1;j<sx-2;j+=2) {
						tmp=outG[(i*sx+j)*3]+(outB[(i*sx+j-1)*3]-outG[(i*sx+j-1)*3]+
								outB[(i*sx+j+1)*3]-outG[(i*sx+j+1)*3])/2;
						CLIP(tmp,outB[(i*sx+j)*3]);
					}
				}
				for (i=1;i<sy-1;i+=2) {
					for (j=0;j<sx-1;j+=2) {
						idx = i*sx+j;
						tmp=outG[idx*3]+(outB[(idx-sx)*3]-outG[(idx-sx)*3]+
								outB[(idx+sx)*3]-outG[(idx+sx)*3])/2;
						CLIP(tmp,outB[idx*3]);
					}
					for (j=1;j<sx-2;j+=2) {
						idx = i*sx+j;
						tmp=outG[idx*3]+(outB[(idx-sx-1)*3]-outG[(idx-sx-1)*3]+
								outB[(idx-sx+1)*3]-outG[(idx-sx+1)*3]+
								outB[(idx+sx-1)*3]-outG[(idx+sx-1)*3]+
								outB[(idx+sx+1)*3]-outG[(idx+sx+1)*3])/4;
						CLIP(tmp,outB[idx*3]);
					}
				}
				for (i=1;i<sy-1;i+=2) {
					for (j=0;j<sx-1;j+=2) {
						tmp=outG[(i*sx+j)*3]+(outB[((i-1)*sx+j)*3]-outG[((i-1)*sx+j)*3]+
								outB[((i+1)*sx+j)*3]-outG[((i+1)*sx+j)*3])/2;
						CLIP(tmp,outB[(i*sx+j)*3]);
					}
					for (j=1;j<sx-2;j+=2) {
						tmp=outG[(i*sx+j)*3]+(outB[((i-1)*sx+j-1)*3]-outG[((i-1)*sx+j-1)*3]+
								outB[((i-1)*sx+j+1)*3]-outG[((i-1)*sx+j+1)*3]+
								outB[((i+1)*sx+j-1)*3]-outG[((i+1)*sx+j-1)*3]+
								outB[((i+1)*sx+j+1)*3]-outG[((i+1)*sx+j+1)*3])/4;
						CLIP(tmp,outB[(i*sx+j)*3]);
					}
				}
				break;
			default: //---------------------------------------------------------
				std::cerr << "Bad Bayer pattern ID: " << type << std::endl;
				return;
				break;
		}

		ClearBorders(dest, sx, sy, 3);
	}

	void ClearBorders(unsigned char* dest, int sx, int sy, int w)
	{
		int i,j;

		// repeat the edge at cv::Rect(2,2,sx-4,sy-4);
		i=3*sx*w-1;
		j=3*sx*(sy-1)-1;
		int idx;
		int offset = 3*sx*w;
		int inv = sx*sy*w-1;
		for(j=0,idx=0;j<3;j++){
			for(i=0;i<sx*w;i++,idx++){
				dest[idx] = dest[offset + i];
				dest[inv - idx] = dest[inv - ( offset + i )];
			}
		}

		inv = sx * w - 1;
		for(j=0;j<sy;j++){
			offset = j * sx * w;
			for(i=0;i<3 * w;i++){
				dest[offset + i] = dest[offset + 3 * w + i%w];
				dest[offset + inv - i] = dest[offset + inv - (3 * w + i%w)];
			}
		}
	}

}// namespace skl

#endif // __SKL_CV_UTILS_BAYER_CONVERSION_H__