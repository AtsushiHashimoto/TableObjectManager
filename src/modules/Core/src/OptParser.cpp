/*!
 * @file OptParser.cpp
 * @author a_hasimoto
 * @date Date Created: 2011/Dec/27
 * @date Last Change: 2011/Dec/29.
 */
#include "OptParser.h"
#include <set>
#include <vector>
#include <iostream>
#include <cassert>
using namespace skl;

OptParserAtomInterface* OptParserAtomInterface::atom_top = NULL;

/*!
 * @brief デフォルトコンストラクタ
 */
OptParser::OptParser(const std::string& conf_deliminator):conf_deliminator(conf_deliminator){
	option_values["help"] = "";
	short_long_map["h"] = "help";
	explanations["help"] = make_usage("h","help","","show this usage.");

	option_values["conf"] = "";
	explanations["conf"] = make_usage("","conf","<FILE>","load default values from configure file. format is <var_name>:<value> for each column.");
}

/*!
 * @brief デストラクタ
 */
OptParser::~OptParser(){}
void OptParser::on(
		const std::string& long_form,
		const std::string& expression,
		const std::string& explanation){
	on("",long_form,expression,explanation);
}
void OptParser::on(
		const std::string& short_form,
		const std::string& long_form,
		const std::string& expression,
		const std::string& explanation){
	assert(!long_form.empty());

	std::string var_name,short_name;
	parse_long_form(long_form,&var_name);
	if(option_values.end()!=option_values.find(var_name)){
		std::cerr << "ERROR: duplicate option '" << var_name << "'." << std::endl;
		std::cerr << "\t Possibly " << var_name << " is reserved option." << std::endl;
		assert(false);
	}

	if(!short_form.empty()){
		parse_short_form(short_form,&short_name);
		if(short_long_map.end()!=short_long_map.find(short_name)){
			std::cerr << "WARNING: short option form '" << short_form << "' has been overwritten." << std::endl;
		}
		short_long_map[short_name] = var_name;
	}

	option_values[var_name] = "";

	explanations[var_name] = make_usage(short_name,var_name,expression,explanation);
}

std::string OptParser::make_usage(
		const std::string& short_name,
		const std::string& var_name,
		const std::string& expression,
		const std::string& explanation){
	std::string _usage = "";
	_usage += "--" + var_name + " " + expression;
	_usage += "\t: " + explanation;
	if(!short_name.empty()){
		_usage += "\n" + std::string(var_name.size(),' ') + "-" + short_name;
	}
	return _usage;
}


void OptParser::parse_short_form(const std::string& form,std::string* var_name){
	std::string short_form = strip(form);
	assert(short_form[0]=='-');
	assert(short_form.size()==2);
	assert(0==isdigit(short_form[1]));
	*var_name = get_short_form(short_form);
}

void OptParser::parse_long_form(const std::string& form,std::string* var_name){
	std::string long_form = strip(form);
	assert(long_form.substr(0,2)=="--");
	assert(long_form.size()>3);
	*var_name = get_long_form(long_form);
}

std::string OptParser::get_short_form(const std::string& str){
	return str.substr(1,1);
}
std::string OptParser::get_long_form(const std::string& str){
	return str.substr(2,std::string::npos);
}

int OptParser::parse_error(const std::string& str)const{
	std::cerr << "ERROR: invalid option '" << str << "'." << std::endl;
	usage();
	return -1;
}

int OptParser::checkType(const std::string& str)const{
	assert(!str.empty());

	if(str[0]!='-'){
		return 0;
	}
	else{
		if(str.size()==1){
			return parse_error(str);
		}
		if(isdigit(str[1])!=0) return 0;

		if(str[1]!='-'){
			if(str.size()!=2){
				return parse_error(str);
			}
			return 1;
		}
		if(str.size()==2){
			return parse_error(str);
		}
		return 2;
	}
}

void OptParser::usage()const{
	bool top = true;
	for(std::map<std::string,std::string>::const_iterator iter = explanations.begin();
			iter!=explanations.end();iter++){
		if(top){
			top = false;
		}
		else{
			std::cerr << std::endl;
		}
		std::cerr << iter->second << std::endl;
	}
}

bool OptParser::help()const{
	return _help;
}

std::vector<std::string> OptParser::parse(int argc,char* argv[]){
	std::vector<std::string> args(argc);
	for(int i=0; i<argc;i++){
		args[i] = argv[i];
	}
	return parse(args);
}

std::vector<std::string> OptParser::parse(const std::vector<std::string>& argv){
	int _switch = 0;
	std::string var_name,form;
	std::vector<std::string> args;

	std::set<std::string> directed_options;
	std::set<std::string>::iterator p_dopt;
	size_t argc = argv.size();

	int i=0;
	while(i!=argc){
		int type = checkType(argv[i]);
		if( type == 0 ){
			if(_switch == 1 || _switch == 2){
				_switch = 3;
			}
			else{
				_switch = 0;
			}
		}
		else{
			_switch = type;
		}

		switch(_switch){
			case 0:
				args.push_back(argv[i]);
				break;
			case 1:
				form = get_short_form(argv[i]);
				if(short_long_map.end()==short_long_map.find(form)){
					parse_error(argv[i]);
					assert(false);
				}
				var_name = short_long_map[form];
				p_dopt = directed_options.find(var_name);
				if(directed_options.end()!=p_dopt){
					std::cerr << "WARNING: option '" << var_name << "' is directed twice." << std::endl;
				}
				else{
					directed_options.insert(var_name);
				}

				option_values[var_name] = "ON";
				break;
			case 2:
				form = get_long_form(argv[i]);
				if(option_values.end()==option_values.find(form)){
					parse_error(argv[i]);
					assert(false);
				}
				var_name = form;
				p_dopt = directed_options.find(var_name);
				if(directed_options.end()!=p_dopt){
					std::cerr << "WARNING: option '" << var_name << "' is directed twice." << std::endl;
				}
				else{
					directed_options.insert(var_name);
				}
				option_values[var_name] = "ON";
				break;
			case 3:
				option_values[var_name] = argv[i];
				break;
			default:
				std::cerr << "ERROR: invalid option " << argv[i] << "." << std::endl;
				assert(false);
		}
		i++;
	}
	_help = false;
	std::string help_buf;
	get("help", &help_buf);
	convert(help_buf,&_help);
	std::string conffile;
	get("conf", &conffile);


	if(!conffile.empty()){
		std::map<std::string,std::string> conf_params;
		assert(parse_conffile(conffile,conf_params,conf_deliminator));


		for(std::map<std::string,std::string>::iterator iter = conf_params.begin();
				iter != conf_params.end(); iter++){
			p_dopt = directed_options.find(iter->first);

			// skip options directed by argv
			if(directed_options.end()!=p_dopt) continue;
			std::map<std::string,std::string>::iterator p_opt_val;
			p_opt_val = option_values.find(iter->first);

			if(option_values.end()==p_opt_val){
				std::cerr << "WARNING: '" << iter->first << "' is not an active option in the current context." << std::endl;
			}
			else{
				p_opt_val->second = iter->second;
			}

		}
	}

	return args;
}

bool OptParser::get(const std::string& var_name, std::string* var)const{
	std::map<std::string, std::string>::const_iterator pp = option_values.find(var_name);
	if(option_values.end()==pp) return false;
	if(""==pp->second) return true;
	*var = pp->second;
	return true;
}
