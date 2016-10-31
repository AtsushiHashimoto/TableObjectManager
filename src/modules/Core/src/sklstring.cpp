#include "sklstring.h"
#include <cassert>
#include <iostream>
#include <fstream>

#include <boost/regex.hpp>

namespace skl{
	std::string strip(const std::string& str){
		if(str.empty()) return str;
		std::string dest;
		bool flag = false;
		size_t begin(0),end(str.size()-1);
		for(;begin<str.size();begin++){
			if(isgraph(str[begin])>0){
				break;
			}
		}
		if(begin == str.size()) return "";

		for(;end>=0;end--){
			if(isgraph(str[end])>0){
				break;
			}
		}

		dest = str.substr(begin,end-begin+1);
		return dest;
	}

	std::vector<std::string> split(const std::string& str, const std::string& deliminator,int length){
		std::vector<std::string> buf;
		std::string temp = str;
		size_t idx;

		// for push_back out of "while"
		length--;

		while((idx=temp.find(deliminator)) != temp.npos && length != 0){
			buf.push_back(temp.substr(0,idx));
			temp = temp.substr(idx+deliminator.size());
			length--;
		}

		buf.push_back(temp);

		return buf;
	}

	std::vector<std::string> split_strip(const std::string& str, const std::string& deliminator, int length){
		std::vector<std::string> buf = split(str,deliminator,length);
		for(size_t i=0;i<buf.size();i++){
			buf[i] = strip(buf[i]);
		}
		return buf;
	}

	std::string replace(const std::string& src,const std::string& strFrom,const std::string& strTo){
		std::string result(src);
		size_t pos(result.find(strFrom));
		while(pos!=std::string::npos){
			result.replace(pos,strFrom.length(),strTo);
			pos = result.find(strFrom);
		}
		return result;
	}

	bool parse_conffile(const std::string& filename, std::map<std::string,std::string>& param_map,const std::string& deliminator){
		std::ifstream fin;
		fin.open(filename.c_str());
		if(!fin){
			return false;
		}
		bool result = parse_conffile(fin,param_map,deliminator);
		fin.close();
		return result;
	}
	bool parse_conffile(std::istream& in,std::map<std::string,std::string>& param_map,const std::string& deliminator){
		std::vector<std::string> buf;
		std::string str;
		while(in && std::getline(in,str)){
			if(str.empty()) continue;
			str = strip(str);
			buf = split(str,"#",2);
			if(buf[0].empty()) continue;
			buf = split_strip(buf[0],deliminator,2);
			if(buf.size()<2 || buf[0].empty()){
#ifdef _DEBUG
				std::cerr << "WARNING: invalid format '" << str << "'." << std::endl;
#endif
				return false;
			}
			param_map[buf[0]] = buf[1];
		}
		return true;
	}

	/*****  Definition for Template Functions  *****/
	template<> bool convert(const std::string& src, std::string* dest){
		*dest = src;
		return true;
	}

	template<> bool convert(const std::string& src, bool* dest){
		const std::string* match = std::find(true_strings, true_strings + true_string_num, src);
		if(match != true_strings + true_string_num){
			*dest = true;
			return true;
		}

		match = std::find( false_strings, false_strings + false_string_num, src);
		if(match != false_strings + false_string_num){
			*dest = false;
			return true;
		}
		return false;	
	}



	std::string DirName(const std::string& filepath){
#ifdef _WIN32
		size_t index = filepath.rfind("\\");
#else
		size_t index = filepath.rfind("/");
#endif
		if(index == filepath.npos){
			return "";
		}
		return filepath.substr(0,index);
	}
	std::string ExtName(const std::string& filepath){
		std::string basename = BaseName(filepath);
		size_t index = basename.rfind(".");
		if(index == basename.npos) return "";
		return basename.substr(index);
	}
	std::string BaseName(const std::string& filepath, const std::string& extname){
#ifdef _WIN32
		size_t index = filepath.rfind("\\");
#else
		size_t index = filepath.rfind("/");
#endif
		std::string basename = filepath.substr(index+1);
		if(extname=="") return basename;
		index = basename.rfind(extname);
		return basename.substr(0,index);
	}

	bool isIPAddress(const std::string& str){
		std::vector<std::string> buf;
		buf = skl::split_strip(str,".",4);
		if(buf.size()!=4)return false;
		for(size_t i=0;i<4;i++){
			if(!std::all_of(buf[i].begin(),buf[i].end(), ::isdigit)) return false;
			int val;
			if(!convert(buf[i],&val)) return false;
			if(val<0 || 255<val) return false;
		}
		return true;
	}


	bool excusePath(std::string& path,const std::string& WinDrive){
		boost::regex reg_win_full_path("\\w:\\\\[^\\/:;,*?<>]+");
		boost::regex reg_win_path("[^\\/:;,*?<>]+");
		boost::regex reg_linux_path("[^\\\\:;,*?<>]+");
#ifdef _WIN32
		if(boost::regex_match(path, reg_win_path)){
			return true;
		}
		if(boost::regex_match(path, reg_win_full_path)){
			return true;
		}

		if(!boost::regex_match(path, reg_linux_path)){
			return false;
		}
		for(size_t i=0;i<path.size();i++)
			if(path[i]=='/') path[i] = '\\';
			if(path[0]=='\\'){
			path = WinDrive + path;
		}
		return true;
#else
		if(boost::regex_match(path, reg_linux_path)){
			return true;
		}
		if(!(boost::regex_match(path, reg_win_path) || boost::regex_match(path,reg_win_full_path))){
			return false;
		}
		if(boost::regex_match(path,reg_win_full_path)){
			path = path.substr(2,std::string::npos);
		}
		for(size_t i=0;i<path.size();i++)
			if(path[i]=='\\') path[i] = '/';
		return true;
#endif
}

}