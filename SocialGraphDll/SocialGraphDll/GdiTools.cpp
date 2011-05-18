#include "GdiTools.h"
#include "GraphConfig.h"
#include "Tools.h"

GdiTools::GdiTools(GraphConfig *cfg)
{
	bmp = new Gdiplus::Bitmap(cfg->iOutputWidth,cfg->iOutputHeight,PixelFormat24bppRGB);
	g = new Gdiplus::Graphics(bmp);
	cBackground = new Gdiplus::Color(cfg->iBackgroundColor.a,cfg->iBackgroundColor.r,cfg->iBackgroundColor.g,cfg->iBackgroundColor.b);
	cChannel = new Gdiplus::Color(cfg->iChannelColor.a,cfg->iChannelColor.r,cfg->iChannelColor.g,cfg->iChannelColor.b);
	cLabel = new Gdiplus::Color(cfg->iLabelColor.a,cfg->iLabelColor.r,cfg->iLabelColor.g,cfg->iLabelColor.b);
	cTitle = new Gdiplus::Color(cfg->iTitleColor.a,cfg->iTitleColor.r,cfg->iTitleColor.g,cfg->iTitleColor.b);
	cNodeBorder = new Gdiplus::Color(cfg->iNodeBorderColor.a,cfg->iNodeBorderColor.r,cfg->iNodeBorderColor.g,cfg->iNodeBorderColor.b);
	cNode = new Gdiplus::Color(cfg->iNodeColor.a,cfg->iNodeColor.r,cfg->iNodeColor.g,cfg->iNodeColor.b);
	cEdge = new Gdiplus::Color(cfg->iEdgeColor.a,cfg->iEdgeColor.r,cfg->iEdgeColor.g,cfg->iEdgeColor.b);
	cBorder = new Gdiplus::Color(cfg->iBorderColor.a,cfg->iBorderColor.r,cfg->iBorderColor.g,cfg->iBorderColor.b);
	sbBackground = new Gdiplus::SolidBrush(*cBackground);
	rBackground = new Gdiplus::Rect(0,0,cfg->iOutputWidth - 1,cfg->iOutputHeight -1);
	pBorder = new Gdiplus::Pen(*cBorder,1.0);
	fChannel = new Gdiplus::Font(L"SansSerif",64, Gdiplus::FontStyleBold);
	pChannel = new Gdiplus::PointF((Gdiplus::REAL)(cfg->gBorderSize + 20.0),80.0f);
	sbChannel = new Gdiplus::SolidBrush(*cChannel);
	fTitle = new Gdiplus::Font(L"SansSerif",18, Gdiplus::FontStyleBold);
	pTitle = new Gdiplus::PointF((Gdiplus::REAL)cfg->gBorderSize,(Gdiplus::REAL)(cfg->gBorderSize - cfg->gNodeRadius - 15));
	sbTitle = new Gdiplus::SolidBrush(*cTitle);
	sbNode = new Gdiplus::SolidBrush(*cNode);
	fNick = new Gdiplus::Font(L"Tahoma",(float)cfg->iNickFontSize);
	fCredits = new Gdiplus::Font(L"SansSerif",10);
	sbLabel = new Gdiplus::SolidBrush(*cLabel);
	pNodeBorder = new Gdiplus::Pen(*cNodeBorder,2);
	GetEncoderClsid(cfg->fWEncoderMime.c_str(),&encoderClsid);
	g->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	fVidTimelapseTime = new Gdiplus::Font(L"Lucida Console", 72, Gdiplus::FontStyleBold);
	fVidTimelapseDate = new Gdiplus::Font(L"Lucida Console", 24, Gdiplus::FontStyleRegular);
	fVidNick = new Gdiplus::Font(L"Comic Sans MS",(float)cfg->iNickFontSize);
	//Courier New
	//VID TIMELAPSE stuff
	//TODO: Hardcoded stuff needs to go to the config one deay
}

GdiTools::~GdiTools(void)
{
	delete bmp;
	delete g;
	delete cBackground;
	delete cChannel;
	delete cLabel;
	delete cTitle;
	delete cNodeBorder;
	delete cNode;
	delete cEdge;
	delete cBorder;
	delete sbBackground;
	delete rBackground;
	delete pBorder;
	delete fChannel;
	delete pChannel;
	delete sbChannel;
	delete fTitle;
	delete pTitle;
	delete sbTitle;
	delete sbNode;
	delete fNick;
	delete fCredits;
	delete sbLabel;
	delete pNodeBorder;
	delete fVidTimelapseTime;
	delete fVidTimelapseDate;
	delete fVidNick;
}
