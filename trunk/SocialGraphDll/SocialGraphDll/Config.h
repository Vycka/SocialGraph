#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "Tools.h"



class Graph;
class GdiTools;

struct CColor
{
	unsigned long argb()
	{
		return *((unsigned long*)(this));
	}
	CColor(unsigned char a = 0, unsigned char r = 0, unsigned char g = 0, unsigned char b = 0) : a(a), r(r), g(g), b(b) {};
	CColor(unsigned long argb) { *((unsigned long*)(this)) = argb; };
	//RGB A for now //TODO Needs to be converted to ARGB in the future.
	CColor(const std::string &str) { std::stringstream ss(str); ss >> r >> g >> b >> a; };
	unsigned char b,g,r,a; //do not change b,g,r,a variable declaration order or it will mess up argb() function
};

struct CReplace {
	CReplace (std::string f,std::string t) : from(f), to(t) {};
	std::string from,to;
};

class Config
{
public:
	Config(const char *fn);
	~Config(void);
	bool isBadConfig() { return badConfig; };

protected:
	void read();

	int getInt(const char *cfgName);
	bool getBool(const char *cfgName);
	double getDouble(const char *cfgName);
	std::string getString(const char *cfgName);
	std::wstring getWString(const char *cfgName);
	CColor getCColor(const char *cfgName);

	inline int getInt(const char *variable, const std::string &defaultValue) { return paramToInt(getParam(variable,defaultValue)); };
	inline bool getBool(const char *variable, const std::string &defaultValue) { return paramToBool(getParam(variable,defaultValue)); };
	inline double getDouble(const char *variable, const std::string &defaultValue) { return paramToDouble(getParam(variable,defaultValue)); };
	inline std::string getString(const char *variable, const std::string &defaultValue) { return paramToString(getParam(variable,defaultValue)); };
	inline std::wstring getWString(const char *variable, const std::string &defaultValue) { return paramToWString(getParam(variable,defaultValue)); };
	inline CColor getCColor(const char *variable, const std::string &defaultValue) { return paramToCColor(getParam(variable,defaultValue)); };

	//getParam - variable must exist, if it's missing, error will be thrown and badConfig will be marked as true.
	const std::string& getParam(const char *key);
	//getParam - if config variable doesn't exist, it will be replaced with defaultParam, badConfig won't be marked.
	const std::string& getParam(const char *key,const std::string &defaultValue);
	std::string& replaceParam(std::string &param);


	std::string configFile;
	std::vector<CReplace> replaceTable;
	bool badConfig;
	std::map<std::string,std::string> cfgList;
private:

	inline bool paramToBool(const std::string &param) { return (param[0] == '1' ? true : false ); };
	inline int paramToInt(const std::string &param) { return atoi(param.c_str()); };
	inline double paramToDouble(const std::string &param) { return atof(param.c_str()); };
	inline std::string paramToString(const std::string &param) { return replaceParam(getInQuotes(param)); };
	inline std::wstring paramToWString(const std::string &param) { return strToWStr(replaceParam(getInQuotes(param))); };
	inline CColor paramToCColor(const std::string &param) { return CColor(param); };
};