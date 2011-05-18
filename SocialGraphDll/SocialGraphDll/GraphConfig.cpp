#include "GraphConfig.h"

GraphConfig::GraphConfig(const char *fn) : Config(fn)
{
	if (badConfig)
		return;
	//names/filestuff
	nWChannel     = getWString("nChannel");
	nChannel      = getString("nChannel");
	replaceTable.push_back(CReplace("<channel>",nChannel));
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
	iEdgeColorChangeInactive = getCColor("iEdgeColorChangeInactive");

	//font/sizes stuff
	iNickFontSize = getDouble("iNickFontSize");

	//ftp
	ftpUpload = getBool("ftpUpload");
	ftpHost   = getString("ftpHost");
	ftpPort   = getInt("ftpPort");
	ftpUser   = getString("ftpUser");
	ftpPass   = getString("ftpPass");
	ftpDir    = getString("ftpDir");
	ftpFile   = getString("ftpFile");

	//graph
	gEdgeColorChangeInactivityMin = getInt("gEdgeColorChangeInactivityMin");
	gEdgeColorChangeInactivityMax = getInt("gEdgeColorChangeInactivityMax");
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
	vidRendererThreads      = getInt("vidRendererThreads");
	vidBeginRenderTime      = getInt("vidBeginRenderTime");
	vidEndRenderTime        = getInt("vidEndRenderTime");

	//...

	//clear list 
	cfgList.clear();
}

GraphConfig::~GraphConfig()
{
}
