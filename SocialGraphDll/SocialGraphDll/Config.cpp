#include "Config.h"
#include "Tools.h"
#include <fstream>
#include <sstream>

Config::Config(const char *fn)
{
	//No try catch, in that way users sees all bad config variables at once :)
	badConfig = false;
	configFile = fn;
	read();
	if (!cfgList.size())
	{
		badConfig = true;
		std::string mmsg = "/.echo -sg SocialGraph: Config file is empty or not exists: ";
		mmsg += configFile;
		execInMirc(&mmsg);
		return;
	}
	rChannel = "<channel>";

	//names/filestuff
	nWChannel     = getWString("nChannel");
	nChannel      = getString("nChannel");
	rChannel      = nChannel.c_str();
	nWTitle       = getWString("nTitle");
	fImageOutput  = getString("fImageOutput");
	fWImageOutput = getWString("fImageOutput");
	fGraphOutput  = getString("fGraphData");
	fWEncoderMime = getWString("fEncoderMime");

	//image stuff
	iOutputWidth     = getInt("iOutputWidth");
	iOutputHeight    = getInt("iOutputHeight");
	iBackgroundColor = getCColor("iBackgroundColor");
	iChannelColor    = getCColor("iChannelColor");
	iLabelColor      = getCColor("iLabelColor");
	iTitleColor      = getCColor("iTitleColor");
	iNodeBorderColor = getCColor("iNodeBorderColor");
	iNodeColor       = getCColor("iNodeColor");
	iEdgeColor       = getCColor("iEdgeColor");
	iBorderColor     = getCColor("iBorderColor");

	//ftp
	ftpUpload = getBool("ftpUpload");
	ftpHost   = getString("ftpHost");
	ftpPort   = getInt("ftpPort");
	ftpUser   = getString("ftpUser");
	ftpPass   = getString("ftpPass");
	ftpDir    = getString("ftpDir");
	ftpFile   = getString("ftpFile");

	//graph
	gTemporalDecayAmount       = getDouble("gTemporalDecayAmount");
	gSpringEmbedderIterations  = getInt("gSpringEmbedderIterations");
	gK                         = getDouble("gK");
	gC                         = getDouble("gC");
	gMaxRepulsiveForceDistance = getDouble("gMaxRepulsiveForceDistance");
	gMaxNodeMovement           = getDouble("gMaxNodeMovement");
	gMinDiagramSize            = getDouble("gMinDiagramSize");
	gBorderSize                = getDouble("gBorderSize");
	gNodeRadius                = getDouble("gNodeRadius");
	gEdgeThreshold             = getDouble("gEdgeThreshold");
	gNodeBumpWeight            = getDouble("gNodeBumpWeight");
	gMinPauseBeforeNextRender  = getInt("gMinPauseBeforeNextRender");
	gMinPauseBeforeNextUpload  = getInt("gMinPauseBeforeNextUpload");
	gEdgeDecayMultiplyIdleSecs = getInt("gEdgeDecayMultiplyIdleSecs");
	gCacheGdiTools             = getInt("gCacheGdiTools");

	//inference heuristics
	hAdjacency  = getDouble("hAdjacency");
	hBinary     = getDouble("hBinary");
	hDirect     = getDouble("hDirect");
	hIndirect   = getDouble("hIndirect");

	//old frames stuff
	oPathBegin     = getString("oPathBegin");
	oPathEnd       = getString("oPathEnd");
	oSaveOldFrames = getBool("oSaveOldFrames");

	//logging/video
	logSave                 = getBool("logSave");
	logFile                 = getString("logFile");
	vidRenderPBegin         = getWString("vidRenderPBegin");
	vidRenderPEnd           = getWString("vidRenderPEnd");
	vidFramesPerDay         = getInt("vidFramesPerDay");
	vidSEIterationsPerFrame = getInt("vidSEIterationsPerFrame");

	//...

	//clear list 
	cfgList.clear();
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
	std::string srcStr = "<channel>";
	replaceString(&des,&des,&srcStr,&rChannel);
	return des;
}
std::wstring Config::getWString(const char *cfgName)
{
	std::string des;
	std::string buff = getParam(cfgName);
	des = getInQuotes(&buff);
	std::string srcStr = "<channel>";
	replaceString(&des,&des,&srcStr,&rChannel);
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