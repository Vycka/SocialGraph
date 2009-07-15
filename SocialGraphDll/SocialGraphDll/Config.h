#pragma once
#include <string>
#include <vector>
#include <map>


class Graph;
class GdiTools;

struct CColor
{
	unsigned char r,g,b,a;
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

	std::string getParam(const char *key);
	void replaceParam(std::string *param);


	std::string configFile;
	std::vector<CReplace> replaceTable;
	bool badConfig;
	std::map<std::string,std::string> cfgList;
};