/*!
 * @file PatchModel.cpp
 * @author a_hasimoto
 * @date Last Change:2012/Jun/26.
 */
#include "skl.h"
#include "PatchModel.h"
#include <fstream>
#include <iomanip>
#include <highgui.h>
#include <iterator>

using namespace skl;

PatchModel::PatchModel():layer(&patches),max_id(0){}
PatchModel::~PatchModel(){}

PatchModel::PatchModel(const cv::Mat& base_bg):layer(&patches),max_id(0){
	base(base_bg);
}



/*
 * @brief 背景画像をセットし、それ以外は全てリセットする
 * */
void PatchModel::base(const cv::Mat& __bg){
	this->_base = __bg.clone();
	_latest_bg = __bg.clone();
	hidden_image = cv::Mat::zeros(__bg.size(),CV_8UC3);
	hidden_mask = cv::Mat::zeros(__bg.size(),CV_32FC1);
	_updated_mask = cv::Mat::zeros(__bg.size(),CV_32FC1);

	patches.clear();
	changed_bg = false;
}


/*
 * @brief 完全に新しいパッチを登録する
 * */
size_t PatchModel::putPatch(const cv::Mat& img, const cv::Mat& fg_edge, const cv::Mat& mask, const cv::Rect& roi){

	size_t ID = max_id;
	patches[ID] = Patch(mask, img, fg_edge, roi);


	std::map<size_t,Patch>::iterator ppatch = patches.find(ID);
	/////////////////////
/*	cv::Rect ___roi = ppatch->second.roi(skl::Patch::original);
	std::cerr << "ROI: " << ___roi.x << ", " << ___roi.y << ", " << ___roi.width << ", " << ___roi.height << std::endl;
	cvWaitKey(-1);
*/	/////////////////////

	patches_underside[ID] = cv::Mat(_latest_bg,ppatch->second.roi(Patch::dilate)).clone();

	// 重なったパッチのvisivility maskを更新する
	// パッチの上下関係を更新する
	layer.push(ID);
	std::vector<size_t> lower_patches = layer.getAllBeneathPatch(ID,Patch::original);
	for(size_t i=0;i<lower_patches.size();i++){
		patches[lower_patches[i]].setCoveredState(
				ppatch->second.roi(Patch::original),
				ppatch->second.mask(Patch::original),
				true);
//		cv::imshow("covered_state",patches[lower_patches[i]].covered_state());
//		std::cerr << "patch " << lower_patches[i] << std::endl;
	}

	put_list.push_back(ID);

	max_id++;
	on_table.resize(max_id,false);
	on_table[ID] = true;
	return ID;
}


/*
 * @brief パッチをモデルから取り除く
 * */
void PatchModel::takePatch(size_t ID,std::vector<size_t>* taken_patch_ids){
	assert(on_table[ID]);



	std::map<size_t,Patch>::iterator ppatch;
	ppatch = patches.find(ID);
	assert(patches.end() != ppatch);
/*	/////////////////////
	cv::Rect ___roi = ppatch->second.roi(skl::Patch::original);
	std::cerr << "ROI: " << ___roi.x << ", " << ___roi.y << ", " << ___roi.width << ", " << ___roi.height << std::endl;
	exit(0);
	/////////////////////
*/
	// 自分より下にある物体のcoveredStateをfalseに戻す
	std::vector<size_t> lower_patches = layer.getAllBeneathPatch(ID,Patch::original);
	for(size_t i=0;i<lower_patches.size();i++){
		patches[lower_patches[i]].setCoveredState(
				ppatch->second.roi(Patch::original),
				ppatch->second.mask(Patch::original),
				false);
	}

	// IDより上にあるパッチを先に取り去る
	size_t upper_patch_id;
	while(UINT_MAX != (upper_patch_id = layer.getUpperPatch(ID,Patch::original))){
		takePatch(upper_patch_id,taken_patch_ids);
	}

	if(taken_patch_ids!=NULL){
		taken_patch_ids->push_back(ID);
	}

	// 自分の後ろの画像を更新用に作成
	cv::Mat hidden_roi = cv::Mat(hidden_image, ppatch->second.roi(Patch::dilate));
	cv::Mat hidden_mask_roi = cv::Mat(hidden_mask, ppatch->second.roi(Patch::dilate));
	// 既にhidden_maskがセットされている領域に対して、
	// ppatchとのブレンディングマスクを作る
	cv::Mat common_mask = ppatch->second.mask(Patch::dilate).clone();
	cv::Mat temp = cv::Mat(hidden_mask_roi.size(), hidden_mask_roi.type());
	cv::threshold(hidden_mask_roi, temp, 0.0, 1.0, CV_THRESH_BINARY);
	common_mask = 1.0 - common_mask;
/*
	cv::imshow("common_mask.inv",common_mask);
	cv::waitKey(-1);
*/
	common_mask = common_mask.mul(temp);

/*
 	cv::imshow("debug",common_mask);
	cv::waitKey(-1);
*/
	blending<cv::Vec3b,float>(hidden_roi,patches_underside[ppatch->first],common_mask,hidden_roi);
/*
	cv::imshow("debug",hidden_image);
	cv::waitKey(-1);
*/

	// 自分の後ろの画像に対する更新率を設定
	cv::max(hidden_mask_roi,ppatch->second.mask(Patch::dilate),hidden_mask_roi);

	//  自分のIDをlayerから取り除く
	layer.erase(ID);
	on_table[ID]=false;
	changed_bg = true;
}

void PatchModel::setObjectLabels(
		const cv::Mat& img,
		const cv::Mat& human,
		const cv::Mat& object_cand_labels,
		size_t object_cand_num,
		std::vector<size_t>* put_object_ids,
		std::vector<size_t>* taken_object_ids){
	assert(NULL != put_object_ids);
	assert(NULL != taken_object_ids);
	put_object_ids->clear();
	taken_object_ids->clear();

	// update hidden state
	std::list<size_t> hidden_patches;
	getHiddenPatches(human,&hidden_patches);
	updateHiddenState(hidden_patches);

	cv::Mat fg_edge;
	cv::Mat bg_edge;
	edge_difference(img,_latest_bg,fg_edge,bg_edge);

	// check whether reappeared objects are still there.
/*	for(std::list<size_t>::iterator iter = _reappeared_objects.begin();
			iter != _reappeared_objects.end(); iter++){
		if(!on_table[*iter]) continue;
		std::map<size_t,Patch>::const_iterator ppatch;
		ppatch = patches.find(*iter);
		if(patches.end() == ppatch){
			// *iter has already been taken when beneath object was removed.
			continue;
		}
		std::cout << "CHECK TAKEN OBJECT( " << ppatch->first << ") = ";
		if(checkTakenObject(ppatch->second,bg_edge)){
			takePatch(*iter,taken_object_ids);
		}
	}
*/
	std::vector<cv::Mat> cand_masks(object_cand_num);
	std::vector<cv::Rect> cand_rois(object_cand_num,cv::Rect(INT_MAX,INT_MAX,0,0));
	for(size_t l=0;l<object_cand_num;l++){
		cand_masks[l] = cv::Mat::zeros(object_cand_labels.size(),CV_8UC1);
	}

//	printLocation();
//	std::cerr << object_cand_num << std::endl;
	for(int y = 0; y < object_cand_labels.rows; y++){
		for(int x = 0; x < object_cand_labels.cols; x++){
			short label = object_cand_labels.at<short>(y,x);
			if(0==label) continue;
			label--;
			cand_masks[label].at<unsigned char>(y,x) = 255;
			cand_rois[label].x = cand_rois[label].x < x ? cand_rois[label].x : x;
			cand_rois[label].y = cand_rois[label].y < y ? cand_rois[label].y : y;
			cand_rois[label].width = cand_rois[label].width > x ? cand_rois[label].width : x;
			cand_rois[label].height= cand_rois[label].height> y ? cand_rois[label].height: y;
		}
	}
/*
	std::cerr << "object masks" << std::endl;
	for(size_t l = 0; l < object_cand_num; l++){
		std::cerr << l << "th cand" << std::endl;
		std::cerr << cand_rois[l].x << "," << cand_rois[l].y << "," << cand_rois[l].width << "," << cand_rois[l].height << std::endl;
		cv::imshow("cand_mask",cand_masks[l]);
		cv::waitKey(-1);
	}
*/
	for(size_t l=0;l < object_cand_num;l++){
		cand_rois[l].width -= cand_rois[l].x - 1;
		cand_rois[l].height-= cand_rois[l].y - 1;
	}

	std::vector<bool> was_taken_object(object_cand_num,false);
	for(size_t l = 0; l < object_cand_num; l++){
		for(int y = cand_rois[l].y;
				y < cand_rois[l].height + cand_rois[l].y; y++){
			for(int x = cand_rois[l].x;
					x < cand_rois[l].width + cand_rois[l].x; x++){
				if(cand_masks[l].at<unsigned char>(y,x)==0) continue;
				if(hidden_mask.at<float>(y,x)==0.0f) continue;
				was_taken_object[l] = true;
				break;
			}
			if(was_taken_object[l]) break;
		}
	}

	// put the rest candidates;
	for(size_t l=0;l<object_cand_num;l++){
		if(was_taken_object[l]) continue;
		size_t check_taken_patch = checkTakenObject(bg_edge,cand_rois[l]);
		if(check_taken_patch != UINT_MAX){
			takePatch(check_taken_patch,taken_object_ids);
			continue;
		}
		std::vector<cv::Point> edge_points;
		for(int y = cand_rois[l].y;
				y < cand_rois[l].height + cand_rois[l].y; y++){
			for(int x = cand_rois[l].x;
					x < cand_rois[l].width + cand_rois[l].x; x++){
				if(fg_edge.at<unsigned char>(y,x)==0) continue;
				edge_points.push_back(cv::Point(x,y));
			}
		}
		if(edge_points.empty()) continue;

		cand_rois[l] = fitRect(edge_points);
		size_t new_id = putPatch(img, fg_edge, cand_masks[l], cand_rois[l]);
		put_object_ids->push_back(new_id);
	}

	// 取られた物体のIDを_hidden_objects,reappeared_objectsなどから取り除く
	for(size_t i=0;i<taken_object_ids->size();i++){
		size_t ID = taken_object_ids->at(i);
		_hidden_objects.remove(ID);
		_newly_hidden_objects.remove(ID);
		_reappeared_objects.remove(ID);
	}

	update();
}

/*
 * @brief 変更をlatest_bgに反映させる
 * */
void PatchModel::update(){
	// taken patch に対する更新
	if(changed_bg){
		changed_bg = false;
//		cv::imshow("latest_bg",_latest_bg);
		blending<cv::Vec3b,float>(
				hidden_image,
				_latest_bg,
				hidden_mask,
				_latest_bg);
/*
		cv::imshow("hidden_image",hidden_image);
		cv::imshow("updated bg",_latest_bg);
		cv::waitKey(-1);
*/
		_updated_mask = hidden_mask.clone();
		hidden_image = cv::Scalar(0);
		hidden_mask = cv::Scalar(0);
	}
	else{
		_updated_mask = cv::Scalar(0);
	}

	// put_patch に対する更新
	for(size_t i=0;i<put_list.size();i++){
		std::map<size_t,Patch>::iterator ppatch;
		ppatch = patches.find(put_list[i]);
		assert(patches.end()!=ppatch);
		cv::Mat latest_bg_roi = cv::Mat(_latest_bg,ppatch->second.roi(Patch::dilate));

		blending<cv::Vec3b,float>(
				ppatch->second.image(Patch::dilate),
				latest_bg_roi,
				ppatch->second.mask(Patch::dilate),
				latest_bg_roi);
		cv::Mat _updated_mask_roi = cv::Mat(_updated_mask,ppatch->second.roi(Patch::dilate));

/*		cv::imshow("patch_image",ppatch->second.image(Patch::dilate));
		cv::imshow("patch_mask",ppatch->second.mask(Patch::dilate));
		cv::waitKey(-1);
*/
		_updated_mask_roi += ppatch->second.mask(Patch::dilate);
	}
/*
	printLocation();
	cv::imshow("updated_mask",_updated_mask);
	cv::waitKey(-1);
*/
	put_list.clear();
}

void PatchModel::latest_bg(const cv::Mat& bg){
	assert(bg.size() == _latest_bg.size());
	assert(bg.type() == _latest_bg.type());
	_latest_bg = bg;
}

const cv::Mat& PatchModel::latest_bg()const{
	return _latest_bg;
}

void PatchModel::save(
		const std::string& file_head,
		const std::string& ext)const{

	std::stringstream ss;

	std::ofstream fout;
	fout.open((file_head+"_state.txt").c_str());
	assert(fout);

	// パッチの状態をあらわすファイルを作成
	std::map<size_t,Patch>::const_iterator ppatch;
	for(ppatch = patches.begin();
			ppatch != patches.end();ppatch++){
		if(!on_table[ppatch->first]) continue;
		ss.str("");
		ss << file_head << "_p" << std::setw(5) << std::setfill('0') << ppatch->first;
		ppatch->second.save(ss.str()+"original"+ext,Patch::original);
		ppatch->second.save(ss.str()+"dilate"+ext,Patch::dilate);
		fout << ppatch->first << ": ";
		fout << layer.getOrder(ppatch->first) << "th patch" << std::endl;
	}

	fout.close();
}

const cv::Mat& PatchModel::base()const{
	return _base;
}

const Patch& PatchModel::operator[](size_t ID)const{
	std::map<size_t,Patch>::const_iterator pp;
	pp = patches.find(ID);
	assert(pp!=patches.end());
	return pp->second;
}


void PatchModel::getHiddenPatches(const cv::Mat& mask,std::list<size_t>* hidden_patch_ids){
	assert(1 == mask.channels());
	assert(NULL!=hidden_patch_ids);
	assert(mask.size() == _latest_bg.size());
	hidden_patch_ids->clear();

	// maskの中の4x4のブロックの数をカウントする
	cv::Rect mrect(INT_MAX, INT_MAX, 0, 0);

	for(int y = 0; y < mask.rows; y += PATCH_MODEL_BLOCK_SIZE){
		for(int x = 0; x < mask.cols; x += PATCH_MODEL_BLOCK_SIZE){
			if(0 == mask.at<unsigned char>(y,x)) continue;
			mrect.x = mrect.x < x ? mrect.x:x;
			mrect.y = mrect.y<y?mrect.y:y;
			mrect.width = mrect.width > x?mrect.width:x;
			mrect.height = mrect.height > y?mrect.height:y;
		}
	}
	if(mrect.x == INT_MAX) return;
	mrect.width -= mrect.x - 1;
	mrect.height -= mrect.y - 1;

	std::vector<size_t> current_hidden_patches;
	for(std::map<size_t,Patch>::const_iterator ppatch = patches.begin();
			ppatch != patches.end(); ppatch++){
		cv::Rect prect = ppatch->second.roi(Patch::original);
		if( !on_table[ppatch->first] ) continue;
		// 長方形が重なっていなければスキップ
		if( !( mrect && prect ) ) continue;

		// パッチの面積(点の数)
		size_t patch_area = ppatch->second.points().size();
		size_t count = 0;
		for(size_t i=0;i<patch_area;i++){
			cv::Point pt = ppatch->second.points()[i];
//			std::cerr << ppatch->first << ": (" << pt.x << ", " << pt.y << ")" << std::endl;
			if(0 == mask.at<unsigned char>(pt.y+prect.y,pt.x+prect.x))continue;
			count++;
		}
//		std::cerr << "count : patch_area = " << count << " : " << patch_area;
		double recall_rate = static_cast<double>(count)/patch_area;
		double min_recall_rate = HIDDEN_OBJECT_MIN_RECALL_RATE;
//		printLocation();
//		std::cerr << "hidden score: " << recall_rate << " <= " << min_recall_rate << std::endl;
		if(recall_rate > min_recall_rate){
			hidden_patch_ids->push_back(ppatch->first);
		}
	}
}

double PatchModel::calcCommonEdge(
		const CvRect& common_rect,
		const Patch& base,
		const Patch& patch){
//	Image _base_edge(base.getEdgeImage());
//	Image _patch_edge(patch.getEdgeImage());

	cv::Rect crect = common_rect;
	crect.width += crect.x;
	crect.height += crect.y;

	size_t common_edge_count = 0;

	cv::Rect brect = base.roi(Patch::original);
	cv::Rect prect = patch.roi(Patch::original);

	for(size_t i=0;i<patch.edge_count();i++){
		int x = patch.edge_points()[i].x + prect.x - brect.x;
		int y = patch.edge_points()[i].y + prect.y - brect.y;
		if(base.edge().at<unsigned char>(y,x) <= 0) continue;
		common_edge_count++;
	}

//	cvShowImage("patch_edge",_patch_edge.getIplImage());
//	cvShowImage("base_edge",_base_edge.getIplImage());
	assert(patch.edge_count()!=0);
	return static_cast<double>(common_edge_count) / patch.edge_count();
}

void PatchModel::getObjectSamplePoints(const cv::Mat& mask, std::vector<cv::Point>* points){
	points->clear();
	for(int y=0;y<mask.rows;y+=PATCH_MODEL_BLOCK_SIZE){
		for(int x=0;x<mask.cols;x+=PATCH_MODEL_BLOCK_SIZE){
			if(0==mask.at<unsigned char>(y,x)) continue;
			points->push_back(cv::Point(x,y));
		}
	}
}

void PatchModel::updateHiddenState(
		std::list<size_t>& __hidden_objects){
	__hidden_objects.sort();

	_newly_hidden_objects.clear();
	std::set_difference(
			__hidden_objects.begin(),__hidden_objects.end(),
			_hidden_objects.begin(),_hidden_objects.end(),
			std::inserter(_newly_hidden_objects,_newly_hidden_objects.end()));

	_reappeared_objects.clear();
	std::set_difference(
			_hidden_objects.begin(),_hidden_objects.end(),
			__hidden_objects.begin(),__hidden_objects.end(),
			std::inserter(_reappeared_objects,_reappeared_objects.end()));

	_hidden_objects = __hidden_objects;
}

size_t PatchModel::checkTakenObject(const cv::Mat& bg_edge,const cv::Rect& roi)const{
	std::map<size_t,Patch>::const_iterator pp;
	for(pp=patches.begin();pp!=patches.end();pp++){
		if(!on_table[pp->first]) continue;
		std::cout << "CHECK TAKEN OBJECT( " << pp->first << ")=";
		if(checkTakenObject(pp->second,bg_edge,&roi)){
			return pp->first;
		}
	}
	return UINT_MAX;
}

bool PatchModel::checkTakenObject(const Patch& patch, const cv::Mat& bg_edge,const cv::Rect* roi2)const{
	cv::Rect roi = patch.roi(Patch::original);
	if(  roi2 != NULL  &&  !( roi && *roi2 )  ){
		std::cerr << " not crossed." << std::endl;
		return false;
	}
	cv::Mat dick_bg_edge;
	cv::Size kernel_size(PATCH_MODLE_EDGE_DILATE,PATCH_MODLE_EDGE_DILATE);
	cv::blur(cv::Mat(bg_edge,roi),dick_bg_edge,kernel_size);
//	cv::threshold(dick_bg_edge,dick_bg_edge,0,255,CV_THRESH_BINARY);


	int count_lost_edge(0);
	int count_laying_edge(0);
	const std::vector<cv::Point>& edge_points(patch.edge_points());
//	cv::imshow("bg_edge",dick_bg_edge);
//	cv::imshow("patch_edge",patch.edge());
	for(size_t i=0;i<edge_points.size();i++){
		if(roi2!=NULL){
			int absx = edge_points[i].x + roi.x;
			int absy = edge_points[i].y + roi.y;
			if(absx < roi2->x || roi2->x+roi2->width <= absx 
					|| absy < roi2->y || roi2->y+roi2->height <= absy){
				count_laying_edge++;
				continue;
			}
		}

		if(0==dick_bg_edge.at<unsigned char>(edge_points[i].y,edge_points[i].x)){
			count_laying_edge++;
			continue;
		}
		else if(patch.isCovered(edge_points[i].x,edge_points[i].y)){
			count_laying_edge++;
			continue;
		}
		count_lost_edge++;
	}
	double score = static_cast<double>(count_lost_edge) / edge_points.size();

/*	std::cerr << __FILE__ << ": " << __LINE__ << std::endl;
	cv::imshow("patch_edge",patch.edge());
	cv::imshow("dick_edge",dick_bg_edge);
*/
	std::cout << score << std::endl;
	if(score > TAKEN_OBJECT_EDGE_CORELATION){
		return true;
	}
	return false;
}

bool PatchModel::erase(size_t ID){
	if(on_table[ID]){
		return false;
	}
	std::map<size_t,Patch>::iterator ppatch = patches.find(ID);
	if(patches.end()==ppatch) return false;
	patches.erase(ppatch);
	std::map<size_t,cv::Mat>::iterator punderside = patches_underside.find(ID);
	if(patches_underside.end()==punderside) return false;
	patches_underside.erase(punderside);
	return true;
}

void PatchModel::removePatchFromModel(size_t id){
	patches.erase(id);
	patches_underside.erase(id);
}