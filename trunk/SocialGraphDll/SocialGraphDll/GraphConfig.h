#pragma once
#include "Config.h"

class GraphConfig :
	public Config
{
public:
	GraphConfig(const char *fn);
	~GraphConfig();

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
	std::wstring fWEncoderMime,fWImageOutput,nWChannel,nWTitle;
	std::string fImageOutput,fGraphOutput,nChannel;
	//old frame saving stuff
	std::string oPathBegin,oPathEnd;
	bool oSaveOldFrames;
	//logging/video
	bool logSave;
	int vidFramesPerDay,vidSEIterationsPerFrame,vidRendererThreads;
	std::wstring vidRenderPBegin,vidRenderPEnd;
	std::string logFile;
};