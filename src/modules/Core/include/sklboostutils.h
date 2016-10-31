#ifndef __SKL_BOOST_UTILS_H__
#define __SKL_BOOST_UTILS_H__

// C++,STL
#include <string>
#include <vector>

bool csv_parse(const std::string& filename, std::vector<std::vector<std::string> >* csv_data);
bool csv_parse(std::istream& in, std::vector<std::vector<std::string> >* csv_data);

#endif // __SKL_BOOST_UTILS_H__
