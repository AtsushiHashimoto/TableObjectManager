#include <boost/tokenizer.hpp>
#include <fstream>

#include "sklboostutils.h"

#ifdef _DEBUG
#define DEBUG_BOOST_UTILS
#include <iostream>
#endif


bool csv_parse(const std::string& filename, std::vector<std::vector<std::string> >* csv_data){
	std::ifstream fin;
	fin.open(filename.c_str());
	if(!fin){
#ifdef DEBUG_BOOST_UTILS
		std::cerr << "Warning: failed to open '" << filename << "'." << std::endl;
#endif
		return false;
	}
	bool result = csv_parse(fin,csv_data);
	fin.close();
	return result;
}

bool csv_parse(std::istream& in, std::vector<std::vector<std::string> >* csv_data){
	if(!in) return false;
	csv_data->clear();
	std::string buf;
	typedef boost::escaped_list_separator<char> esc_sep;
	esc_sep sep("\\",",","\"");
	typedef boost::tokenizer<esc_sep > tokenizer;
	while(in && std::getline(in,buf)){
		tokenizer tokens(buf,sep);
		std::vector<std::string> temp;
		for(tokenizer::iterator tok_iter = tokens.begin();
				tok_iter != tokens.end(); tok_iter++){
			temp.push_back(*tok_iter);
		}
		csv_data->push_back(temp);
	}
	return true;
}

