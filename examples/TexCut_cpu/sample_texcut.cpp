#define USE_VIDEO_CAPTURE_OPT_PARSER
#include "skl.h"
#include "sklcv.h"
//#include "sklflycap.h"
#include <sstream>

opt_on(std::string, camera_setting, "", "-C","<FILE>","load camera conf parameters.");
opt_on(std::string, input_file,"","-i","<FILE>","load video file");
opt_on(std::string, output_dir,"","-o","<DIR>","save image files");
opt_on(float,alpha,1.5,"-a","<FLOAT>","set TexCut parameter alpha");
opt_on(float,smoothing_term_weight,1.0,"-s","<FLOAT>","set TexCut parameter smoothing_term_weight");
opt_on(float,thresh_tex_diff,0.4,"-t","<FLOAT>","set TexCut parameter thresh_tex_diff");
opt_on(unsigned char,over_exposure_thresh,248,"","<UCHAR>","set TexCut parameter over_exposure_thresh");
opt_on(unsigned char,under_exposure_thresh,8,"","<UCHAR>","set TexCut parameter under_exposure_thresh");

opt_on_bool(frame_diff,"","calc frame difference instead of background subtraction");

opt_on(unsigned int, step, 1, "", "<UINT>", "set step for skipping frames.");

skl::TexCut* bgs_algo_cpu;

int main(int argc,char* argv[]){
	skl::OptParser options;
	std::vector<std::string> args;
	opt_parse(options,argc,argv,args);

	if(options.help()){
		options.usage();
		return EXIT_FAILURE;
	}

//	skl::FlyCapture cam;
	skl::VideoCapture cam;
	skl::VideoCaptureParams params;

	if(!camera_setting.empty()){
		params.load(camera_setting);
	}
	// CAUTION: call opt_parse_cap_prop after opt_parse
	opt_parse_cap_prop(params);



	if(input_file.empty()){
		cam.open(0);
	}
	else{
		cam.open(input_file);
	}

	// CAUTION: set params after open the camera.
	cam.set(params);
	std::cout << "=== Parameter Settings of Camera ===" << std::endl;
	std::cout << cam.get();

	if(!cam.isOpened()){
		std::cerr << "ERROR: failed to open video." << std::endl;
		std::cerr << "       Maybe, camera is not connected to the PC??" << std::endl;
		return EXIT_FAILURE;
	}

	std::stringstream ss;
	cv::namedWindow("image",0);
	cv::namedWindow("result",0);

	cv::Mat mat,result;

	// prepare background subtraction algorithms
	skl::TexCut bgs_algo;

	// get first background image
	cam >> mat;
	bgs_algo.setParams(alpha,smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh);
	bgs_algo.setBackground(mat);

	// get second background image
	cam >> mat;
	bgs_algo.learnImageNoiseModel(mat);


	int frame = cam.get("POS_FRAMES");
	skl::StopWatch swatch;
	unsigned int _step_buf=0;
	while('q'!=cv::waitKey(10)){
		std::cout << "frame: " << frame++ << std::endl;
		cam >> mat;
		_step_buf++;
		if(step>_step_buf){
			continue;
		}
		else{
			_step_buf=0;
		}
		if(!cam.isOpened()||mat.cols==0||mat.rows==0) break;

		cv::imshow("image",mat);

		swatch.start();
		bgs_algo.compute(mat,result);
		std::cerr << "ElaspedTime: " << swatch.lap() << std::endl;
		if(frame_diff){
			bgs_algo.setBackground(mat);
		}
		cv::imshow("result",result);

		if(!output_dir.empty()){
			std::stringstream filename;
			filename << output_dir << "/";
			if(input_file.empty()){
				filename << skl::Time::now();
			}
			else{
				filename << std::setw(5) << std::setfill('0') << frame;
			}
			filename << ".png";
			cv::imwrite(filename.str(),result);
		}
	}
	cv::destroyAllWindows();
	return EXIT_SUCCESS;
}
