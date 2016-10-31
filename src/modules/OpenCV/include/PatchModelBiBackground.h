/*!
 * @file PatchModelBiBackground.h
 * @author a_hasimoto
 * @date Date Created: 2012/Jan/10
 * @date Last Change:2012/Jan/11.
 */
#ifndef __SKL_PATCH_MODEL_BI_BACKGROUND_H__
#define __SKL_PATCH_MODEL_BI_BACKGROUND_H__


//#include "../OpenCV/include/PatchModel.h"
#include "PatchModel.h"

namespace skl{

	/*!
	 * @brief 複数の背景を持つパッチモデル
	 */
	class PatchModelBiBackground: public PatchModel{

		public:
			PatchModelBiBackground();
			PatchModelBiBackground(const cv::Mat& base);
			virtual ~PatchModelBiBackground();
			void setObjectLabels(
					const cv::Mat& img,
					const cv::Mat& human_label,
					const cv::Mat& object_cand_labels,
					size_t object_cand_num,
					std::vector<size_t>* put_object_ids,
					std::vector<size_t>* taken_object_ids);

			void removePatchFromModel(size_t id);

			/*** Accessor ***/
			void base(const cv::Mat& base_bg);
			void latest_bg2(const cv::Mat& bg);
			inline const cv::Mat& latest_bg2()const{return _latest_bg2;};
			bool erase(size_t ID);
		protected:
			std::map<size_t,cv::Mat> patches_underside2;
			cv::Mat _latest_bg2;
			cv::Mat hidden_image2;

			virtual size_t putPatch(const cv::Mat& img, const cv::Mat& fg_edge, const cv::Mat& mask, const cv::Rect& roi);
			virtual void takePatch(size_t ID,std::vector<size_t>* taken_patch_ids);
			virtual void update();

		private:
	};

} // skl

#endif // __SKL_PATCH_MODEL_BI_BACKGROUND_H__

