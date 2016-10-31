/*!
 * @file OptParser.h
 * @author a_hasimoto
 * @date Date Created: 2011/Dec/27
 * @date Last Change:2013/Nov/25.
 */
#ifndef __SKL_OPT_PARSER_H__
#define __SKL_OPT_PARSER_H__

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cassert>
#include "sklstring.h"

namespace skl{

/*!
 * @brief Parse Option from CommandLine
 */
class OptParser{

	public:
		OptParser(const std::string& conf_deliminator=":");
		virtual ~OptParser();
		void on(const std::string& long_form, const std::string& expression, const std::string& explanation);
		void on(const std::string& short_form, const std::string& long_form,const std::string& expression, const std::string& explanation);
		bool get(const std::string& var_name, std::string* var)const;
		std::vector<std::string> parse(int argc,char* argv[]);
		std::vector<std::string> parse(const std::vector<std::string>& args);
		void usage()const;
		bool help()const;
	protected:
		std::map<std::string,std::string> explanations;
		std::map<std::string, std::string> option_values;
		std::map<std::string, std::string> short_long_map;

		std::string conf_deliminator;

		int checkType(const std::string& str)const;
		static void parse_short_form(const std::string& form,std::string* var_name);
		static void parse_long_form(const std::string& form,std::string* var_name);

		static std::string get_short_form(const std::string& str);
		static std::string get_long_form(const std::string& str);
		static std::string make_usage(
				const std::string& short_name,
				const std::string& var_name,
				const std::string& expression,
				const std::string& explanation);
		bool _help;
	private:
		int parse_error(const std::string& str)const;
};

/*
 * @brief parser for indivisual valiables (interface)
 * */
class OptParserAtomInterface{
	public:
		OptParserAtomInterface(){
			next = atom_top;
			atom_top = this;
		}
		virtual ~OptParserAtomInterface(){};
		virtual void on(OptParser* parser)=0;
		virtual void get(OptParser* parser)=0;
		OptParserAtomInterface* next;
		static OptParserAtomInterface* atom_top;
	protected:
};

template<class T> class OptParserAtom : public OptParserAtomInterface{
	public:
		OptParserAtom(
			const std::string& short_form,
			const std::string& var_name,
			const std::string& expression,
			const std::string& explanation,
			T* dest);
		~OptParserAtom();
		void on(OptParser* parser);
		void get(OptParser* parser);
	protected:
		std::string short_form;
		std::string var_name;
		std::string expression;
		std::string explanation;
		T* dest;
};

template<class T> OptParserAtom<T>::OptParserAtom(
			const std::string& short_form,
			const std::string& var_name,
			const std::string& expression,
			const std::string& explanation,
			T* dest):short_form(short_form),var_name(var_name),expression(expression),explanation(explanation),dest(dest){}
template<class T> OptParserAtom<T>::~OptParserAtom(){}

template<class T> void OptParserAtom<T>::on(OptParser* parser){
	parser->on(short_form, "--" + var_name, expression, explanation);
}

template<class T> void OptParserAtom<T>::get(OptParser* parser){
	std::string buf;
	parser->get(var_name,&buf);
	if(buf.empty()) return;
	convert<T>(buf,dest);
}

template<class T, class Container=std::vector<T> > class OptParserAtomContainer : public OptParserAtomInterface{
	public:
		OptParserAtomContainer(
				const std::string& short_form,
				const std::string& var_name,
				const std::string& default_value,
				const std::string& expression,
				const std::string& explanation,
				Container* dest,
				const std::string& deliminator=":",
				int length=-1):
			short_form(short_form),var_name(var_name),default_value(default_value),
			expression(expression),explanation(explanation),
			dest(dest),deliminator(deliminator),length(length){set_default();}
		~OptParserAtomContainer(){}
		void on(OptParser* parser);
		void get(OptParser* parser);
		void set_default();
	protected:
		std::string short_form;
		std::string var_name;
		std::string default_value;
		std::string expression;
		std::string explanation;
		Container* dest;
		std::string deliminator;
		int length;
};

template<class T,class Container> void OptParserAtomContainer<T,Container>::on(OptParser* parser){
	parser->on(short_form, "--" + var_name, expression, explanation);
}

template<class T,class Container> void OptParserAtomContainer<T,Container>::set_default(){
	if(default_value.empty()) return;
	convert2container<T,Container>(default_value,dest,deliminator,length);
}

template<class T,class Container> void OptParserAtomContainer<T,Container>::get(OptParser* parser){
	std::string buf;
	parser->get(var_name,&buf);
	if(buf.empty()){
		return;
	}
	convert2container<T,Container>(buf,dest,deliminator,length);
}

// opt_on_boolは非推奨(初期値がfalseであるとわかりにくいため)
#define opt_on_bool(VAR, SHORT_FORM,EXPLANATION)\
	opt_on(bool, VAR, false, SHORT_FORM, "<BOOL>", EXPLANATION)

#define opt_on(TYPE, VAR, DEFAULT_VAL, SHORT_FORM, EXPRESSION, EXPLANATION)\
	TYPE VAR( DEFAULT_VAL );\
generate_atomic_parser(TYPE,VAR,SHORT_FORM, EXPRESSION, EXPLANATION);\

#define opt_on_container(CONTAINER_TYPE, ELEM_TYPE, VAR, DEFAULT_VAL, SHORT_FORM, EXPRESSION, EXPLANATION, DELIMINATOR, LENGTH)\
	CONTAINER_TYPE< ELEM_TYPE > VAR;\
generate_atomic_parser_container(CONTAINER_TYPE, ELEM_TYPE,VAR,DEFAULT_VAL, SHORT_FORM, EXPRESSION, EXPLANATION, DELIMINATOR, LENGTH);\

//#ifdef __linux__

#define opt_parse(PARSER,ARGC, ARGV, ARGS)\
skl::OptParserAtomInterface* __opt_parser_func_list__ = skl::OptParserAtomInterface::atom_top;\
while(__opt_parser_func_list__!=NULL){\
	__opt_parser_func_list__->on(& PARSER);\
	__opt_parser_func_list__ = __opt_parser_func_list__->next;\
}\
ARGS = PARSER.parse(ARGC,ARGV);\
__opt_parser_func_list__ = skl::OptParserAtomInterface::atom_top;\
while(__opt_parser_func_list__!=NULL){\
	__opt_parser_func_list__->get(& PARSER);\
	__opt_parser_func_list__ = __opt_parser_func_list__->next;\
}
/*#elif _WIN32
#define opt_parse(PARSER,ARGC, ARGV, ARGS)\
std::vector<std::string> __SKL_OPT_PARSER_ARGV__(ARGC);\
for(int i = 0; i < ARGC; i++){\
	char __SKL_OPT_PARSER_TEMP_BUF__[256];\
	sprintf_s(__SKL_OPT_PARSER_TEMP_BUF__,"%S", ARGV[i]);\
	__SKL_OPT_PARSER_ARGV__[i] = __SKL_OPT_PARSER_TEMP_BUF__;\
}\
skl::OptParserAtomInterface* __opt_parser_func_list__ = skl::OptParserAtomInterface::atom_top;\
while(__opt_parser_func_list__!=NULL){\
	__opt_parser_func_list__->on(& PARSER);\
	__opt_parser_func_list__ = __opt_parser_func_list__->next;\
}\
ARGS = PARSER.parse(__SKL_OPT_PARSER_ARGV__);\
__opt_parser_func_list__ = skl::OptParserAtomInterface::atom_top;\
while(__opt_parser_func_list__!=NULL){\
	__opt_parser_func_list__->get(& PARSER);\
	__opt_parser_func_list__ = __opt_parser_func_list__->next;\
}
#endif
*/

#define generate_atomic_parser(TYPE,VAR, SHORT_FORM, EXPRESSION, EXPLANATION)\
skl::OptParserAtom<TYPE> __opt_parser_atom_##VAR(SHORT_FORM, #VAR, EXPRESSION, EXPLANATION, &VAR)

#define generate_atomic_parser_container(CONTAINER_TYPE,ELEM_TYPE, VAR, DEFAULT_VAL, SHORT_FORM, EXPRESSION, EXPLANATION, DELIMINATOR, LENGTH)\
skl::OptParserAtomContainer < ELEM_TYPE, CONTAINER_TYPE < ELEM_TYPE > > __opt_parser_atom_container_##VAR(SHORT_FORM, #VAR, DEFAULT_VAL, EXPRESSION, EXPLANATION, &VAR, DELIMINATOR, LENGTH)


} // skl

#endif //__SKL_OPT_PARSER_H__
