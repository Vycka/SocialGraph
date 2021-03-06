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
	iEdgeColorInactive = getCColor("iEdgeColorInactive");
	iEdgeChangeListColor = getCColor("iEdgeChangeListColor");
	iEdgeChangeListColorInactive = getCColor("iEdgeChangeListColorInactive");
	iChatDotColor    = getCColor("iChatDotColor");
	iNodeBorderWidth = getDouble("iNodeBorderWidth", "2.0");

	//font/sizes/misc stuff
	iNickFont           = getWString("iNickFont");
	iNickFontSize       = getDouble("iNickFontSize");
	iEdgeActiveMinAlpha = getInt("iEdgeActiveMinAlpha");
	
	//ftp
	ftpUpload = getBool("ftpUpload");
	ftpHost   = getString("ftpHost");
	ftpPort   = getInt("ftpPort");
	ftpUser   = getString("ftpUser");
	ftpPass   = getString("ftpPass");
	ftpDir    = getString("ftpDir");
	ftpFile   = getString("ftpFile");

	//graph
	//gEdgeNickListDrawInserts   = getBool("gEdgeNickListDrawInserts");
	//gEdgeNickListDrawRemoves   = getBool("gEdgeNickListDrawRemoves");
	gEdgeChangeListEnabled       = getBool("gEdgeChangeListEnabled");
	gEdgeChangeListDrawLogSize   = getInt("gEdgeChangeListDrawLogSize");
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
	gNodeBumpWeight            = getDouble("gNodeBumpWeight");
	gMinEdgeWidth              = getInt("gMinEdgeWidth","2");
	gMaxEdgeWidth              = getInt("gMaxEdgeWidth","4");
	gMinPauseBeforeNextRender  = getInt("gMinPauseBeforeNextRender");
	gMinPauseBeforeNextUpload  = getInt("gMinPauseBeforeNextUpload");
	gEdgeDecayMultiplyIdleSecs = getInt("gEdgeDecayMultiplyIdleSecs");
	gCacheGdiTools             = getInt("gCacheGdiTools");
	gMinMaxWeight              = getDouble("gMinMaxWeight");
	gPreserveAlpha             = getBool("gPreserveAlpha");
	gDrawUnconnectedNodes      = getBool("gDrawUnconnectedNodes","0");
	gDrawTimeStamp             = getBool("gDrawTimeStamp","1");
	gStrechToImageSize         = getBool("gStrechToImageSize","1");

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
	vidFramesPerDay         = getDouble("vidFramesPerDay");
	vidSEIterationsPerFrame = getInt("vidSEIterationsPerFrame");
	vidRendererThreads      = getInt("vidRendererThreads");
	vidBeginRenderTime      = getInt("vidBeginRenderTime");
	vidEndRenderTime        = getInt("vidEndRenderTime");
	vidNickFont             = getWString("vidNickFont");
	vidXYDivRatio           = getDouble("vidXYDivRatio");
	vidEdgeChatDotEnabled   = getBool("vidEdgeChatDotEnabled");
	vidEdgeChatDotSpeedPixelsPerFrame  = getDouble("vidEdgeChatDotSpeedPixelsPerFrame");
	vidEdgeChatDotRadius    = getDouble("vidEdgeChatDotRadius");
	vidCDisconnected        = getDouble("vidCDisconnected");
	vidMaxNodeMovementDisconnected = getDouble("vidMaxNodeMovementDisconnected");
	vidDisconnectedFadeOutFrames = getInt("vidDisconnectedFadeOutFrames");
	vidMaxNodeMovement      = getDouble("vidDisconnectedFadeOutFrames");
	vidC                    = getDouble("vidC");
	vidOutputWidth          = getInt("vidOutputWidth");
	vidOutputHeight         = getInt("vidOutputHeight");
	vidNickFontSize         = getInt("vidNickFontSize");
	vidNodeRadius           = getInt("vidNodeRadius");
	vidBorderSize           = getInt("vidBorderSize");
	vidPreserveAlpha        = getBool("vidPreserveAlpha");
	vidEdgeFramesToDie      = getInt("vidEdgeFramesToDie","30");
	//...

	//clear list 
	cfgList.clear();
}

GraphConfig::~GraphConfig()
{
}
