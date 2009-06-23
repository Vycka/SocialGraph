#pragma once
#include <string>
#include <map>

class Graph;
class GdiTools;

struct CColor
{
	unsigned char r,g,b,a;
};

class Config
{
public:
	Config(const char *fn);
	~Config(void);
	void read(const char *fn);


	int getInt(const char *cfgName);
	bool getBool(const char *cfgName);
	double getDouble(const char *cfgName);
	std::string getString(const char *cfgName);
	std::wstring getWString(const char *cfgName);
	CColor getCColor(const char *cfgName);

	std::string getParam(const char *key);
	bool isBadConfig() { return badConfig; };

	//friend GdiTools;
	//friend Graph;
//private:
	//img config
	int iOutputWidth,iOutputHeight;
	CColor iBackgroundColor,iChannelColor,
		iLabelColor,iTitleColor,iNodeBorderColor,iNodeColor,
		iEdgeColor,iBorderColor;
	//graph config
	double gTemporalDecayAmount,gK,gC,gMaxRepulsiveForceDistance,
		gMaxNodeMovement,gMinDiagramSize,gBorderSize,
		gNodeRadius,gEdgeThreshold,gNodeBumpWeight;
	int gMinPauseBeforeNextRender,gEdgeDecayMultiplyIdleSecs,gMinPauseBeforeNextUpload,gCacheGdiTools,gSpringEmbedderIterations;
	//ftp stuff
	std::string ftpHost,ftpUser,ftpPass,ftpDir,ftpFile;
	int ftpPort;
	bool ftpUpload;
	//Heuristic Weightings
	double hAdjacency,hBinary,hDirect,hIndirect;
	//names outputs.. etc..
	std::wstring fWEncoderMime,fWImageOutput,nWChannel;
	std::string fImageOutput,fGraphOutput,nChannel;
	//old frame saving stuff
	std::string oPathBegin,oPathEnd;
	bool oSaveOldFrames;
	//logging/video
	bool logSave;
	int vidFramesPerDay,vidSEIterationsPerFrame;
	std::wstring vidRenderPBegin,vidRenderPEnd;
	std::string logFile;

private:
	std::string configFile;
	std::map<std::string,std::string> cfgList;
	bool badConfig;
};