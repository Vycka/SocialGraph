#include "Config.h"
#include "Tools.h"
#include <fstream>
#include <sstream>

Config::Config(const char *fn)
{
	badConfig = false;
	configFile = fn;
	read();
	if (!cfgList.size())
	{
		badConfig = true;
		std::string mmsg = "/.echo -sg SocialGraph: Config file is empty or not exists: ";
		mmsg += configFile;
		execInMirc(&mmsg);
	}
}


Config::~Config(void)
{
}

int Config::getInt(const char *cfgName)
{
	int des;
	std::string buff = getParam(cfgName);
	std::stringstream ss(buff);
	ss >> des;
	return des;
}

bool Config::getBool(const char *cfgName)
{
	bool des;
	std::string buff = getParam(cfgName);
	std::stringstream ss(buff);
	ss >> des;
	return des;
}
double Config::getDouble(const char *cfgName)
{
	double des;
	std::string buff = getParam(cfgName);
	std::stringstream ss(buff);
	ss >> des;
	return des;
}
std::string Config::getString(const char *cfgName)
{
	std::string des;
	std::string buff = getParam(cfgName);
	des = getInQuotes(&buff);
	replaceParam(&des);
	return des;
}
std::wstring Config::getWString(const char *cfgName)
{
	std::string des;
	std::string buff = getParam(cfgName);
	des = getInQuotes(&buff);
	replaceParam(&des);
	std::wstring wdes;
	wdes.assign(des.begin(),des.end());
	return wdes;
}
CColor Config::getCColor(const char *cfgName)
{
	CColor des;
	int r,g,b,a;
	std::string buff = getParam(cfgName);
	std::stringstream ss(buff);
	ss >> r >> g >> b >> a;
	des.r = r;
	des.g = g;
	des.b = b;
	des.a = a;
	return des;
}

void Config::read()
{
	std::fstream f(configFile.c_str(),std::ios_base::in);
	std::string buff;
	while (f.good())
	{
		getline(f,buff,'\n');
		if (buff.empty())
			continue;
		std::stringstream ss(buff);
		std::string c,p;
		ss >> c;
		if (c[0] == '/' || c[0] == '#' || c[0] == '-') //eilutes prasidedancias su / - # skaitysim kaip komentarus
			continue;
		getline(ss,p,'\0');
		cfgList.insert(std::make_pair(c,p));
	}
	f.close();
}

std::string Config::getParam(const char *key)
{
	std::map <std::string,std::string>::iterator cfgIter = cfgList.find(std::string(key));
	if (cfgIter == cfgList.end())
	{
		std::string err = "/echo -sg SocialGraph: Bad/Missing config variable: " + std::string(key) + "\r\nConfig File: " + this->configFile;
		execInMirc(&err);
		badConfig = true;
		return std::string("\" \"");
	}
	else
		return cfgIter->second;
}

void Config::replaceParam(std::string *param)
{
	for (unsigned int x = 0;x < replaceTable.size();x++)
		replaceString(param,param,&replaceTable[x].from,&replaceTable[x].to);
}