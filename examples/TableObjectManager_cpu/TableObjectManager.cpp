#define USE_VIDEO_CAPTURE_OPT_PARSER

#include "skl.h"
#include "sklcv.h"


/*** Params for TexCut ***/
opt_on(float,alpha,1.5,"-a","<FLOAT>","set parameter alpha for TexCut.");
opt_on(float,smoothing_term_weight,1.0,"-s","<FLOAT>","set parameter smoothing term weight for TexCut");
opt_on(float, thresh_tex_diff,0.4,"","<FLOAT>","set threshold parameter for TexCut");
opt_on(unsigned char, over_exposure_thresh, 248, "","<UCHAR>","set lowest overexposing value.");
opt_on(unsigned char, under_exposure_thresh, 8, "", "<UCHAR>","set highest underexposing value.");

/*** Params which should be fixed as the default values. ***/
opt_on(size_t,min_object_size, 0, "", "<UINT>", "set minimum object file for region labeling.");
opt_on(size_t,static_object_lifetime,1,"","<UINT>","set lifetime of static object");

/*** Params for other Sub-Algorithms of TableObjectManager ***/
opt_on(std::string, workspace_end_filename, "","-w","<IMG_FILE>","set end of workspace by binary image.");
opt_on(double,static_threshold,0.95,"","<DOUBLE>","set threshold to check static object.");
opt_on(float,learning_rate,0.05,"","<FLOAT>","set learning rate for background updating.");
opt_on(float,learning_rate2,0.10,"","<FLOAT>","set second learning rate for background updating.");
opt_on(size_t,touch_memory_length,8,"","<UINT>","set length of memory for touch reasoning.");


/*** Params for IO ***/
opt_on(std::string, input, "", "-i", "<FILE>", "set input video|image-list file.");
opt_on(size_t, device, 0, "-d", "<UINT>", "set input video device num.");
opt_on(std::string, camera_setting, "-C", "", "<CONFFILE>", "set camera parameters");

opt_on(bool,visualize,false,"-v","<BOOL>","view put/taken object");

opt_on(std::string, output_dir_for_put,"","-P","<FILE>","set output directory for put objects");
opt_on(std::string, output_dir_for_taken,"","-T","<FILE>","set output directory for taken objects");

skl::TableObjectManager* createTableObjectManagerAlgorithm(const cv::Mat& bg1,const cv::Mat& bg2);

int main(int argc, char* argv[]){
	skl::OptParser options;
	std::vector<std::string> args;
	opt_parse(options,argc,argv,args);
	if(options.help()){
		std::cerr << "Usage: " << args[0] << " image.lst0 [options]" << std::endl;
		std::cerr << "Option" << std::endl;
		options.usage();
		return EXIT_FAILURE;
	}

	// prepare a camera
	skl::VideoCaptureParams params;
	if(!camera_setting.empty()){
		params.load(camera_setting);
	}
	// CAUTION: call opt_parse_cap_prop after opt_parse
	opt_parse_cap_prop(params);

	skl::VideoCapture cam;
	if(input.empty()){
		cam.open(device);
	}
	else{
		cam.open(input);
	}
	
	// set camera parameters
	cam.set(params);

	// input images must be color for TableObjectManager.
	assert(cam.get(skl::MONOCROME)<=0);

#ifdef _DEBUG // make debugしたときだけ実行される
	std::cout << "=== Parameter Setting of camera ===" << std::endl;
	std::cout << cam.get();
	std::cerr << std::endl;
#endif

	if(visualize){
		cv::namedWindow("raw image",0);
		cv::namedWindow("human mask",0);
		cv::namedWindow("put object",0);
		cv::namedWindow("taken object",0);
		cv::namedWindow("background",0);
	}

	cv::Mat bg1,bg2;
	cam >> bg1;
	cam >> bg2;


	skl::TableObjectManager* to_manager = createTableObjectManagerAlgorithm(bg1,bg2);

	cv::Mat image,human_mask;
	std::vector<size_t> put_objects,taken_objects;
	std::list<size_t> hidden_objects,newly_hidden_objects,reappeared_objects;

	size_t pos_frames = cam.get(skl::POS_FRAMES);

	while(cam.grab()){
		cam.retrieve(image,0);


		to_manager->compute(image,human_mask,put_objects,taken_objects);

		if(visualize){
			cv::imshow("raw image",image);
			cv::imshow("human mask",human_mask);
			cv::imshow("background",to_manager->bg());
			for(size_t i=0;i<put_objects.size();i++){
				cv::imshow("put object",(*to_manager->patch_model())[put_objects[i]].print_image());
				// マスクだけ欲しい場合はprint_mask()を使う
			}
			for(size_t i=0;i<taken_objects.size();i++){
				cv::imshow("taken object",(*to_manager->patch_model())[taken_objects[i]].print_image());
			}
			cv::waitKey(5);
		}

		if(!output_dir_for_put.empty()){
			for(size_t i=0;i<put_objects.size();i++){
				std::stringstream ss;
				ss << output_dir_for_put << "/putobject_"
					<< std::setw(6) << std::setfill('0') << pos_frames
					<< std::setw(2) << std::setfill('0') << i;
				cv::imwrite(ss.str(),(*to_manager->patch_model())[put_objects[i]].print_image());
			}
		}

		if(!output_dir_for_taken.empty()){
			for(size_t i=0;i<taken_objects.size();i++){
				std::stringstream ss;
				ss << output_dir_for_taken << "/takenobject_"
					<< std::setw(6) << std::setfill('0') << pos_frames
					<< std::setw(2) << std::setfill('0') << i;
				cv::imwrite(ss.str(),(*to_manager->patch_model())[taken_objects[i]].print_image());
			}
		}


		// 下記のコードにより、それぞれ、
		// 1. 隠された(human mask領域と交差する)物体
		// 2. このフレームで新たに隠された物体
		// 3. 前のフレームまで隠されていたが現フレームでは隠されていない物体
		// のIDを取得できる
		hidden_objects = to_manager->patch_model()->hidden_objects();
		newly_hidden_objects = to_manager->patch_model()->hidden_objects();
		reappeared_objects = to_manager->patch_model()->reappeared_objects();
		// これらのIDの物体の領域画像はput_objects,taken_objectsと同様の方法で取得できる

		pos_frames++;
	}

	if(visualize){
		cv::destroyAllWindows();
	}
	delete to_manager;
	return EXIT_SUCCESS;
}

cv::Mat createWorkspaceEnd(cv::Size size){
	cv::Mat dist(size,CV_8UC1,cv::Scalar(0));


	// 本当の端の画素は色補完アルゴリズムの都合で偽色などである場合があるため、少し内側を使う
	int padding = 4;
	unsigned char* pdist1 = dist.ptr<unsigned char>(padding);
	unsigned char* pdist2 = dist.ptr<unsigned char>(size.height-1-padding);
	for(int x=0;x<size.width;x++){
		pdist1[x] = 255;
		pdist2[x] = 255;
	}
	for(int y=0;y<size.height;y++){
		dist.at<unsigned char>(y,padding) = 255;
		dist.at<unsigned char>(y,size.width-1-padding) = 255;
	}
	return dist;
}

skl::TableObjectManager* createTableObjectManagerAlgorithm(const cv::Mat& bg1,const cv::Mat& bg2){

	cv::Mat workspace_end;
	cv::Size workspace_end_size(bg1.cols/TEXCUT_BLOCK_SIZE,bg2.rows/TEXCUT_BLOCK_SIZE);
	if(workspace_end_filename.empty()){
		workspace_end = createWorkspaceEnd(workspace_end_size);
	}
	else{
		workspace_end = cv::imread(workspace_end_filename,0);
		assert(workspace_end_size==workspace_end.size());
	}

	skl::TableObjectManagerBiBackground* to_manager_bb = new skl::TableObjectManagerBiBackground(
			learning_rate,
			learning_rate2,
			new skl::TexCut(bg1,bg2,alpha,smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh),
			new skl::RegionLabelingSimple(min_object_size),
			new skl::HumanDetectorWorkspaceEnd(workspace_end),
			new skl::StaticRegionDetector(static_threshold,static_object_lifetime),
			new skl::TouchedRegionDetector(touch_memory_length),
			new skl::TexCut(bg1,bg2,alpha,smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh), // for subtraction with dark background image.
			new skl::PatchModelBiBackground(bg1)
			);


	return dynamic_cast<skl::TableObjectManager*>(to_manager_bb);
}
