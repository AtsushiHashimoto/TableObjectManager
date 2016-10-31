/*!
 * @file PatchModel.h
 * @author a_hasimoto
 * @date Last Change:2012/Sep/28.
 */
#ifndef __PATCH_MODEL_H__
#define __PATCH_MODEL_H__

#include <list>
#include <set>
#include <map>
#include "sklcvutils.h"

#define PATCH_MODEL_BLOCK_SIZE 4

#define PATCH_EDGE_CANNY_THRESH1 16
#define PATCH_EDGE_CANNY_THRESH2 32
#define PATCH_MODLE_EDGE_DILATE 2

#define TAKEN_OBJECT_EDGE_CORELATION 0.3
#define HIDDEN_OBJECT_MIN_RECALL_RATE 0.0

namespace skl{

#define PATCH_DILATE 32

	class Patch{
		public:
			enum Type{
				original=0,
				dilate=1
			};
		static cv::Mat blur_mask(const cv::Mat& mask, size_t blur_width);
		public:
			Patch();
			Patch(const cv::Mat& mask, const cv::Mat& img,  const cv::Mat& fg_edge, const cv::Rect& roi);
			Patch(const Patch& other);
			~Patch();
			void set(const cv::Mat& mask, const cv::Mat& img, const cv::Mat& fg_edge, const cv::Rect& roi);

			void setCoveredState(const cv::Rect& rect,const cv::Mat& mask,bool isCovered);
			void setCoveredState(int x,int y,bool isCovered);
			inline bool isCovered(int x,int y)const{return _covered_state.at<float>(y,x)!=0.0;}

			void save(const std::string& filename, Type type, const std::string& edge_filename="")const;

			/*** Accessor ***/
			float maskValue(int x,int y, Type type=original)const;
			const unsigned char* operator()(int x,int y, Type type=original)const;
			unsigned char* operator()(int x,int y,Type type=original);

			inline const cv::Rect& roi(Type type=original)const{return _roi[type];}
			cv::Mat print_image(Type type=original)const;
			cv::Mat print_mask(Type type=original)const;
			inline cv::Mat& image(Type type=original){return _image[type];}
			inline const cv::Mat& image(Type type=original)const{return _image[type];}
			inline const cv::Mat& mask(Type type=original)const{return _mask[type];}

			inline const cv::Mat& edge()const{return _edge;}
			void edge(const cv::Mat& __edge);
			inline size_t edge_count()const{return _edge_points.size();};
			inline const std::vector<cv::Point>& points()const{return _points;}
			inline const std::vector<cv::Point>& edge_points()const{return _edge_points;}
			inline const cv::Mat& covered_state()const{return _covered_state;}
		protected:
			cv::Mat _mask[2];
			cv::Mat _image[2];
			cv::Mat _edge;
			cv::Rect _roi[2];
			cv::Mat _covered_state;
			cv::Size base_size;
			std::vector<cv::Point> _points; // relative (_image[original] base) coordinate, rough resolution (PATCH_MODEL_BLOCK_SIZExPATCH_MODEL_BLOCK_SIZE)
			std::vector<cv::Point> _edge_points; // relative (_image[original] base) coordinalte
		private:
			void cvtG2L(int* x,int* y, Type type)const;
			bool isIn(int local_x,int local_y, Type type)const;
	};

	class PatchLayer{
		public:
			PatchLayer(std::map<size_t,Patch>* patches=NULL);
			~PatchLayer();
			void push(size_t ID);
			virtual bool erase(size_t ID);

			// 重なっているパッチのうち、1つだけ上のものを返す
			// なければUINT_MAXを返す
			size_t getUpperPatch(size_t ID, Patch::Type)const;
			std::vector<size_t> getAllBeneathPatch(size_t ID,Patch::Type type);
			size_t getOrder(size_t ID)const;
			size_t getTopLayer()const;
			size_t size()const;
			size_t getLayer(size_t i,bool from_top = true)const;
		protected:
			std::list<size_t> layer_order;
			std::map<size_t,Patch>* patches;
		private:
			static bool isOverlayed(const cv::Rect& common_rect,const Patch& p1,const Patch& p2, Patch::Type type, cv::Mat& mask);
			static bool isOverlayed(const cv::Rect& common_rect,const Patch& p1,const Patch& p2, Patch::Type type);
	};

	class PatchModel{
			friend class PatchLayer;
			friend class PatchTracker;
		public:
			PatchModel();
			PatchModel(const cv::Mat& base);
			~PatchModel();
			Patch& operator()(size_t ID);

			virtual void setObjectLabels(
					const cv::Mat& img,
					const cv::Mat& human_label,
					const cv::Mat& object_cand_labels,
					size_t object_cand_num,
					std::vector<size_t>* put_object_ids,
					std::vector<size_t>* taken_object_ids);

			void save(const std::string& file_head,const std::string& ext)const;
			const Patch& operator[](size_t ID)const;
			virtual bool erase(size_t ID);

			/**** Accessor ****/
			virtual void base(const cv::Mat& __bg);
			const cv::Mat& base()const;

			void latest_bg(const cv::Mat& bg);
			const cv::Mat& latest_bg()const;

			inline const std::list<size_t>& hidden_objects()const{return _hidden_objects;}
			inline const std::list<size_t>& newly_hidden_objects()const{return _newly_hidden_objects;}
			inline const std::list<size_t>& reappeared_objects()const{return _reappeared_objects;}
			inline const cv::Mat& updated_mask(){return _updated_mask;}
		
			void removePatchFromModel(size_t id);

		protected:
			std::map<size_t,Patch> patches;
			std::map<size_t,cv::Mat> patches_underside;
			cv::Mat _latest_bg;
			cv::Mat _base;
			PatchLayer layer;

			virtual size_t putPatch(const cv::Mat& img, const cv::Mat& fg_edge, const cv::Mat& mask, const cv::Rect& roi);
			virtual void takePatch(size_t ID,std::vector<size_t>* taken_patch_ids);
			void getHiddenPatches(const cv::Mat& human_mask, std::list<size_t>* hidden_patch_ids);

			static double calcCommonEdge(
					const CvRect& common_rect,
					const Patch& patch_1,
					const Patch& patch_2);

			static void getObjectSamplePoints(const cv::Mat& mask, std::vector<cv::Point>* points);

			bool checkTakenObject(const Patch& patch, const cv::Mat& bg_edge,const cv::Rect* roi2=NULL)const;
			size_t checkTakenObject(const cv::Mat& bg_edge,const cv::Rect& roi)const;
			virtual void update();

			void updateHiddenState(std::list<size_t>& __hidden_object);
			size_t max_id;
			std::vector<size_t> put_list;
			cv::Mat hidden_image;
			cv::Mat hidden_mask;
			cv::Mat _updated_mask;
			bool changed_bg;
			bool changed_fg;
			std::list<size_t> _hidden_objects;
			std::list<size_t> _newly_hidden_objects;
			std::list<size_t> _reappeared_objects;
			std::vector<bool> on_table;

		private:
	};

}

#endif // __PATCH_MODEL_H__
