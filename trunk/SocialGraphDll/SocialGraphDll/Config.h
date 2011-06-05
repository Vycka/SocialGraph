#pragma once
#include <string>
#include <vector>
#include <map>



class Graph;
class GdiTools;

struct CColor
{
	unsigned long argb()
	{
		return *((unsigned long*)(this));
	}
	CColor(unsigned char a = 0, unsigned char r = 0, unsigned char g = 0, unsigned char b = 0) : a(a), r(r), g(g), b(b) {};
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

	std::string getParam(const char *key);
	void replaceParam(std::string *param);


	std::string configFile;
	std::vector<CReplace> replaceTable;
	bool badConfig;
	std::map<std::string,std::string> cfgList;
};