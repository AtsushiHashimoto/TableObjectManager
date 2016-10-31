#define USE_VIDEO_CAPTURE_OPT_PARSER
#define BOOST_CHRONO_VERSION 2

#include <csignal>
#include <boost/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include "skl.h"
#include "sklcv.h"
#include "sklcvgpu.h"

opt_on(int, step,1,"","<UINT>","set step to skip frames.");

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

opt_on(std::string, output_dir_for_put,"","-P","<DIR>","set output directory for put objects");
opt_on(std::string, output_dir_for_taken,"","-T","<DIR>","set output directory for taken objects");

opt_on(std::string, output_dir_for_background,"","-B","<DIR>","set output directory for background image");
opt_on(std::string, output_dir_for_humanbody,"","-H","<DIR>","set output directory for human body mask image");
opt_on(std::string, output_dir_for_log,"","-L","<DIR>","set output directory to notify completed image save");
opt_on(size_t, gpu_device,0,"","<UINT>", "set gpu device.");

skl::gpu::TableObjectManagerBiBackground* createTableObjectManager(const cv::gpu::GpuMat& bg1,const cv::gpu::GpuMat& bg2);
long long timestampMillseconds(void);
std::string formatTimestamp(long long timestamp);

namespace {
    bool loop;
    void exit_loop(int sig){loop = false;}
}

int main(int argc, char* argv[]){
    loop = true;
    signal(SIGINT,exit_loop);
    signal(SIGTERM,exit_loop);

    skl::OptParser options;
    std::vector<std::string> args;
    opt_parse(options,argc,argv,args);
    if(options.help()){
        std::cerr << "Usage: " << args[0] << " image.lst0 [options]" << std::endl;
        std::cerr << "Option" << std::endl;
        options.usage();
        return EXIT_FAILURE;
    }

    cv::gpu::setDevice(gpu_device);

    // prepare a camera
    skl::VideoCaptureParams params;
    if(!camera_setting.empty()){
        params.load(camera_setting);
    }
    // CAUTION: call opt_parse_cap_prop after opt_parse
    opt_parse_cap_prop(params);

    skl::gpu::VideoCaptureGpu cam;
    if(input.empty()){
        cam.open(device);
    }
    else{
        cam.open(input);
    }
    if(!cam.isOpened()){
        std::cerr << "ERROR: failed to open camera." << std::endl;
        return EXIT_FAILURE;
    }

    if(!cam.set(skl::SKIP_FAKE_CAPTURE,true)){
        std::cerr << "ERROR: failed to set SKIP_FAKE_CAPTURE." << std::endl;
    }

    std::cerr << "A camera is opened." << std::endl;
    // set camera parameters
    cam.set(params);
    std::cerr << "Parameters are set to the camera." << std::endl;

    // input images must be color for TableObjectManager.
//	assert(cam.get(skl::MONOCROME)<=0);


#ifdef _DEBUG // make debugしたときだけ実行される
    std::cout << "=== Parameter Setting of camera ===" << std::endl;
    std::cout << cam.get();
    std::cout << std::endl;
#endif


    if(visualize){
        cv::namedWindow("raw image",0);
        cv::namedWindow("human mask",0);
        cv::namedWindow("put object",0);
        cv::namedWindow("taken object",0);
        cv::namedWindow("background",0);
    }
    std::cerr << "Windows are created." << std::endl;

    cv::gpu::GpuMat bg1,bg2;
    cam >> bg1;
    cam >> bg2;

    std::cerr << "The background images are set." << std::endl;
    skl::gpu::TableObjectManagerBiBackground* to_manager = createTableObjectManager(bg1,bg2);
    std::cerr << "The table object manager is created." << std::endl;

    cv::Mat image,human_mask,human_mask_prev;
    cv::gpu::GpuMat image_gpu;
    std::vector<size_t> put_objects,taken_objects;
    std::list<size_t> hidden_objects,newly_hidden_objects,reappeared_objects;

    size_t pos_frames = cam.get(skl::POS_FRAMES);

    std::cerr << "Ready to start main loop." << std::endl;
    cv::Mat bg_image;
        int prev_frame_num=0;
//#ifdef _DEBUG
    double st, ed, st_t, ed_t;
    double t_frame = 0.0f, t_compute = 0.0f;
//#endif
    while(loop && cam.grab()){
//#ifdef _DEBUG
        st = cv::getTickCount();
//#endif
        while(loop && input.empty()==false && cam.get(skl::POS_FRAMES)-prev_frame_num < step){
            if(!cam.grab())break;
        }
        prev_frame_num = cam.get(skl::POS_FRAMES);

        // We cannot get properly frame number when input source is camera.
        // Therefore, we use pseudo frame number.
        if (input.empty()) {
            static int pseudo_frame_num = 0;
            ++pseudo_frame_num;
            prev_frame_num = pseudo_frame_num;
        }

        cam.retrieve(image,0);
        cam.retrieve(image_gpu,0);
        if(-1==prev_frame_num) break;
        std::cerr << "A new image is retrieved." << std::endl;
        if (input.empty()) {
            std::cerr << prev_frame_num << std::endl;
        } else {
            std::cerr << cam.get(skl::POS_FRAMES) << std::endl;
        }

        auto timestamp = formatTimestamp(timestampMillseconds());

        // keep bg_image for -B option.
        bg_image = to_manager->bg().clone();
		human_mask_prev = human_mask.clone();
        to_manager->compute(image,image_gpu,human_mask,put_objects,taken_objects);

//#ifdef _DEBUG
        ed_t = cv::getTickCount();
        t_compute += (ed_t -st) / cv::getTickFrequency();
//#endif
        if(visualize){
            cv::imshow("raw image",image);
            cv::imshow("human mask",human_mask);
            cv::imshow("background",to_manager->bg());
            for(size_t i=0;i<put_objects.size();i++){
                cv::imshow("put object",(*to_manager->patch_model())[put_objects[i]].print_image());
                // マスクだけ欲しい場合はprint_maskを使う
            }
            for(size_t i=0;i<taken_objects.size();i++){
                cv::imshow("taken object",(*to_manager->patch_model())[taken_objects[i]].print_image());
            }
            if('q'==cv::waitKey(5)){
                break;
            }
        }

        bool hasChange = false;
        if(!output_dir_for_put.empty()){
            for(size_t i=0;i<put_objects.size();i++){
                hasChange = true;
                std::stringstream ss;
                ss << output_dir_for_put << "/putobject_"
                    << std::setw(7) << std::setfill('0') << prev_frame_num << "_"
                    << std::setw(3) << std::setfill('0') << put_objects[i] << ".png";
                cv::imwrite(ss.str(),(*to_manager->patch_model())[put_objects[i]].print_image());				
            }
        }

        if(!output_dir_for_taken.empty()){
            for(size_t i=0;i<taken_objects.size();i++){
                hasChange = true;
                std::stringstream ss;
                ss << output_dir_for_taken << "/takenobject_"
                    << std::setw(7) << std::setfill('0') << prev_frame_num << "_"
                    << std::setw(3) << std::setfill('0') << taken_objects[i] << ".png";
				cv::imwrite(ss.str(),(*to_manager->patch_model())[taken_objects[i]].print_image());
                // 不要になったパッチはメモリから消去
                (*to_manager->mutable_patch_model()).removePatchFromModel(taken_objects[i]);
            }
        }
        if(hasChange && !output_dir_for_background.empty()){
            std::stringstream ss;
            ss << output_dir_for_background << "/bg_"
                << std::setw(7) << std::setfill('0') << prev_frame_num << ".png";
            cv::imwrite(ss.str(),bg_image);
        }
        if(hasChange && !output_dir_for_humanbody.empty()){
            std::stringstream ss;
            ss << output_dir_for_humanbody << "/humanbody_"
                << std::setw(7) << std::setfill('0') << prev_frame_num << ".png";
			cv::imwrite(ss.str(),human_mask_prev);
        }
        if(hasChange && !output_dir_for_log.empty()){
            std::stringstream ss;
			ss << output_dir_for_log << "/log_"
                << std::setw(7) << std::setfill('0') << prev_frame_num << ".log";			std::ofstream fout;
			fout.open(ss.str().c_str());
			fout << "frame: " << prev_frame_num << std::endl;
			fout << "PUT: " << "[" << skl::join(put_objects.begin(),put_objects.end(),",") << "]" << std::endl;
			fout << "TAKEN: " << "[" << skl::join(taken_objects.begin(),taken_objects.end(),",") << "]" << std::endl;
			fout.close();
		}
        // 下記のコードにより、それぞれ、
        // 1. 隠された(human mask領域と交差する)物体
        // 2. このフレームで新たに隠された物体
        // 3. 前のフレームまで隠されていたが現フレームでは隠されていない物体
        // のIDを取得できる
        hidden_objects = to_manager->patch_model()->hidden_objects();
        newly_hidden_objects = to_manager->patch_model()->newly_hidden_objects();
        reappeared_objects = to_manager->patch_model()->reappeared_objects();
        // これらのIDの物体の領域画像はput_objects,taken_objectsと同様の方法で取得できる

        pos_frames++;
//#ifdef _DEBUG
        ed = cv::getTickCount();
        t_frame += (ed -st) / cv::getTickFrequency();
//#endif
    }

    if(visualize){
        cv::destroyAllWindows();
    }
    std::cerr << "Deleting Table Object Manager." << std::endl;

//#ifdef _DEBUG
    double * t = to_manager->time_compute();
    double * t_bg = to_manager->time_bg();
    std::cout << "Total processing time / frames = " << t_frame << "/" << pos_frames << "=" << t_frame/pos_frames << std::endl;
    std::cout << "Total computation time / frames = " << t_compute << "/" << pos_frames << "=" << t_compute/pos_frames << std::endl;
    std::cout << "+Time for TableObjectManagerBiBackground::bg_subtract():" << to_manager->t_bg_subtract << "/"
        << pos_frames << "=" << to_manager->t_bg_subtract/pos_frames << std::endl;
    std::cout << "   -TexCut::setBackground().sobel: " << t_bg[0] << "/" 
        << pos_frames << "=" << t_bg[0]/pos_frames << std::endl;
    std::cout << "   -TexCut::compute(): " << to_manager->t_bg_subtract_compute << "/" 
        << pos_frames << "=" << to_manager->t_bg_subtract_compute/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().split: " << t[0] << "/" 
        << pos_frames << "=" << t[0]/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().sobel: " << t[1] << "/" 
        << pos_frames << "=" << t[1]/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().exposure: " << t[2] << "/" 
        << pos_frames << "=" << t[2]/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().data_term: " << t[3] << "/" 
        << pos_frames << "=" << t[3]/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().bindup_exposure: " << t[4] << "/" 
        << pos_frames << "=" << t[4]/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().bindup_data_term: " << t[5] << "/" 
        << pos_frames << "=" << t[5]/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().bindup smoothing_term: " << t[6] << "/" 
        << pos_frames << "=" << t[6]/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().enhance smoothing_term: " << t[7] << "/" 
        << pos_frames << "=" << t[7]/pos_frames << std::endl;
    std::cout << "      -TexCut::compute().graphcut: " << t[8] << "/" 
        << pos_frames << "=" << t[8]/pos_frames << std::endl;
    std::cout << "+Time for patching: " << to_manager->t_patch << "/"
        << pos_frames << "=" << to_manager->t_patch/pos_frames << std::endl;
    std::cout << "+time for masking: " << to_manager->t_mask << "/"
        << pos_frames << "=" << to_manager->t_mask/pos_frames << std::endl;
    std::cout << "+Time for TableObjectManagerBiBackground::bg_update():" << to_manager->t_bg_update << "/" 
        << pos_frames << "=" << to_manager->t_bg_update/pos_frames << std::endl;
    std::cout << "   -parallel for:" << to_manager->t_bg_update_parfor << "/" 
        << pos_frames << "=" << to_manager->t_bg_update_parfor/pos_frames << std::endl;
//#endif
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

skl::gpu::TableObjectManagerBiBackground* createTableObjectManager(const cv::gpu::GpuMat& bg1,const cv::gpu::GpuMat& bg2){

    cv::Mat workspace_end;
    cv::Size workspace_end_size(bg1.cols/TEXCUT_BLOCK_SIZE,bg2.rows/TEXCUT_BLOCK_SIZE);
    if(workspace_end_filename.empty()){
        workspace_end = createWorkspaceEnd(workspace_end_size);
    }
    else{
        workspace_end = cv::imread(workspace_end_filename,0);
		if(workspace_end.size()!=workspace_end_size){
			std::cerr << "WARNING: workspace_end image size is invalid. The image is automatically resized." << std::endl;
			std::cerr << "\tworkspace_end size should be " << workspace_end_size.width << "x" << workspace_end_size.height << std::endl;
			std::cerr << "\timage size is " << workspace_end.size().width << "x" << workspace_end.size().height << std::endl;
			cv::Mat temp;
			cv::resize(workspace_end,temp,workspace_end_size,0,0,cv::INTER_NEAREST);
			workspace_end = temp;
		}
        assert(workspace_end_size==workspace_end.size());
    }

    skl::gpu::TableObjectManagerBiBackground* to_manager_bb = new skl::gpu::TableObjectManagerBiBackground(
            learning_rate,
            learning_rate2,
            new skl::gpu::TexCut(bg1,bg2,alpha,smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh),
            new skl::RegionLabelingSimple(min_object_size),
            new skl::HumanDetectorWorkspaceEnd(workspace_end),
            new skl::StaticRegionDetector(static_threshold,static_object_lifetime),
            new skl::TouchedRegionDetector(touch_memory_length),
            new skl::gpu::TexCut(bg1,bg2,alpha,smoothing_term_weight,thresh_tex_diff,over_exposure_thresh,under_exposure_thresh), // for subtraction with dark background image.
            new skl::PatchModelBiBackground(cv::Mat(bg1))
            );


    return to_manager_bb;
}

long long timestampMillseconds(void)
{
  return boost::chrono::duration_cast<boost::chrono::milliseconds>(
      boost::chrono::system_clock::now().time_since_epoch()).count();
}

std::string formatTimestamp(long long timestamp)
{
  boost::chrono::milliseconds ms(timestamp);

  std::stringstream ss;
  ss << boost::chrono::time_fmt(boost::chrono::timezone::local, "%Y.%m.%d_%H.%M.%S.")
     << boost::chrono::system_clock::time_point(ms)
     << std::setfill('0')
     << std::setw(3)
     << (ms - boost::chrono::duration_cast<boost::chrono::seconds>(ms)).count();

  return ss.str();
}