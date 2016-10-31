/*!
 * @file PatchModelBiBackground.cpp
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/10
 * @date Last Change: 2012/Sep/27.
 */
#include "PatchModelBiBackground.h"
#include "skl.h"
#include <highgui.h>

using namespace skl;


/*!
 * @brief デフォルトコンストラクタ
 */
PatchModelBiBackground::PatchModelBiBackground(){

}

/*!
 * @brief 背景画像を引数に取るコンストラクタ
 */
PatchModelBiBackground::PatchModelBiBackground(const cv::Mat& base){
	this->base(base);
}

/*!
 * @brief デストラクタ
 */
PatchModelBiBackground::~PatchModelBiBackground(){

}
void PatchModelBiBackground::setObjectLabels(
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
	for(std::list<size_t>::iterator iter = _reappeared_objects.begin();
			iter != _reappeared_objects.end(); iter++){
		if(!on_table[*iter]) continue;
		std::map<size_t,Patch>::const_iterator ppatch;
		ppatch = patches.find(*iter);
		if(patches.end() == ppatch){
			// *iter has already been taken when beneath object was removed.
			continue;
		}
		std::cout << "CHECK TAKEN OBJECT( " << ppatch->first << ")=";
		if(checkTakenObject(ppatch->second,bg_edge)){
			takePatch(*iter,taken_object_ids);
		}
	}

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

size_t PatchModelBiBackground::putPatch(const cv::Mat& img, const cv::Mat& fg_edge, const cv::Mat& mask, const cv::Rect& roi){
	size_t ID = max_id;
	patches[ID] = Patch(mask, img, fg_edge, roi);
	std::map<size_t,Patch>::iterator ppatch = patches.find(ID);

	patches_underside[ID] = cv::Mat(_latest_bg,ppatch->second.roi(Patch::dilate)).clone();
	patches_underside2[ID] = cv::Mat(_latest_bg2,ppatch->second.roi(Patch::dilate)).clone();

	// 重なったパッチのvisivility maskを更新する
	// パッチの上下関係を更新する
	layer.push(ID);
	std::vector<size_t> lower_patches = layer.getAllBeneathPatch(ID,Patch::original);
	for(size_t i=0;i<lower_patches.size();i++){
		patches[lower_patches[i]].setCoveredState(
				ppatch->second.roi(Patch::original),
				ppatch->second.mask(Patch::original),
				true);
	}

	put_list.push_back(ID);

	max_id++;
	on_table.resize(max_id,false);
	on_table[ID] = true;
	return ID;
}

void PatchModelBiBackground::takePatch(size_t ID,std::vector<size_t>* taken_patch_ids){
	assert(on_table[ID]);

	std::map<size_t,Patch>::iterator ppatch;
	ppatch = patches.find(ID);
	assert(patches.end() != ppatch);

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
//	cv::imshow("hidden_image",hidden_image);
//	cv::imshow("hidden_mask",hidden_mask);
	cv::Mat hidden_roi = cv::Mat(hidden_image, ppatch->second.roi(Patch::dilate));
	cv::Mat hidden2_roi = cv::Mat(hidden_image2,ppatch->second.roi(Patch::dilate));
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
//	cv::imshow("patch underside",patches_underside[ppatch->first]);
//	cv::waitKey(-1);
	blending<cv::Vec3b,float>(hidden_roi,patches_underside[ppatch->first],common_mask,hidden_roi);
//	cv::imshow("hidden_image after",hidden_image);
	blending<cv::Vec3b,float>(hidden2_roi,patches_underside2[ppatch->first],common_mask,hidden2_roi);

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


void PatchModelBiBackground::update(){
	// taken patch に対する更新
	if(changed_bg){
		changed_bg = false;
//		cv::imshow("latest_bg",_latest_bg);
//		cv::imshow("hidden_image",hidden_image);
//		cv::imshow("hidden_mask",hidden_mask);

		blending<cv::Vec3b,float>(
				hidden_image,
				_latest_bg,
				hidden_mask,
				_latest_bg);
		blending<cv::Vec3b,float>(
				hidden_image2,
				_latest_bg2,
				hidden_mask,
				_latest_bg2);
//		cv::imshow("latest_bg after",_latest_bg);

		_updated_mask = hidden_mask.clone();
		hidden_image = cv::Scalar(0);
		hidden_image2 = cv::Scalar(0);
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

		cv::Mat latest_bg2_roi = cv::Mat(_latest_bg2,ppatch->second.roi(Patch::dilate));
		blending<cv::Vec3b,float>(
				ppatch->second.image(Patch::dilate),
				latest_bg2_roi,
				ppatch->second.mask(Patch::dilate),
				latest_bg2_roi);

		cv::Mat _updated_mask_roi = cv::Mat(_updated_mask,ppatch->second.roi(Patch::dilate));
//		cv::imshow("patch_image",ppatch->second.image(Patch::dilate));
//		cv::imshow("patch_mask",ppatch->second.mask(Patch::dilate));
		_updated_mask_roi += ppatch->second.mask(Patch::dilate);
	}
/*
	printLocation();
	cv::imshow("updated_mask",_updated_mask);
	cv::waitKey(-1);
*/
	put_list.clear();

}


void PatchModelBiBackground::base(const cv::Mat& base_bg){
	PatchModel::base(base_bg);
	_latest_bg2 = base_bg.clone();
	hidden_image2 = cv::Mat::zeros(base_bg.size(),CV_8UC3);
}

void PatchModelBiBackground::latest_bg2(const cv::Mat& bg){
	assert(bg.size() == _latest_bg2.size());
	assert(bg.type() == _latest_bg2.type());
	_latest_bg2 = bg;
}

bool PatchModelBiBackground::erase(size_t ID){
	if(!PatchModel::erase(ID)){
		return false;
	}
	std::map<size_t,cv::Mat>::iterator punderside = patches_underside2.find(ID);
	if(patches_underside2.end()==punderside) return false;
	patches_underside2.erase(punderside);
	return true;
}


void PatchModelBiBackground::removePatchFromModel(size_t id){
	PatchModel::removePatchFromModel(id);
	patches_underside2.erase(id);
}