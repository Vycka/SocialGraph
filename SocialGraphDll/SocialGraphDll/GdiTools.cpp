#include "GdiTools.h"
#include "GraphConfig.h"
#include "Tools.h"

GdiTools::GdiTools(GraphConfig *cfg)
{
	//TODO: Make PixelFormat customizable from config file.
	bmp = new Gdiplus::Bitmap(cfg->iOutputWidth,cfg->iOutputHeight,PixelFormat32bppPARGB);
	g = new Gdiplus::Graphics(bmp);
	cBackground = new Gdiplus::Color(cfg->iBackgroundColor.argb());
	cChannel = new Gdiplus::Color(cfg->iChannelColor.argb());
	cLabel = new Gdiplus::Color(cfg->iLabelColor.argb());
	cTitle = new Gdiplus::Color(cfg->iTitleColor.argb());
	cNodeBorder = new Gdiplus::Color(cfg->iNodeBorderColor.argb());
	cNode = new Gdiplus::Color(cfg->iNodeColor.argb());
	cEdge = new Gdiplus::Color(cfg->iEdgeColor.argb());
	cBorder = new Gdiplus::Color(cfg->iBorderColor.argb());
	sbBackground = new Gdiplus::SolidBrush(*cBackground);
	rBackground = new Gdiplus::Rect(0,0,cfg->iOutputWidth - 1,cfg->iOutputHeight -1);
	pBorder = new Gdiplus::Pen(*cBorder,1.0);
	fChannel = new Gdiplus::Font(L"SansSerif",64, Gdiplus::FontStyleBold);
	pChannel = new Gdiplus::PointF((float)(cfg->gBorderSize + 20.0),(float)cfg->gBorderSize + 30);
	sbChannel = new Gdiplus::SolidBrush(*cChannel);
	fTitle = new Gdiplus::Font(L"SansSerif",18, Gdiplus::FontStyleBold);
	pTitle = new Gdiplus::PointF((float)cfg->gBorderSize,(Gdiplus::REAL)(cfg->gBorderSize));
	sbTitle = new Gdiplus::SolidBrush(*cTitle);
	sbNode = new Gdiplus::SolidBrush(*cNode);

	fNick = new Gdiplus::Font(cfg->iNickFont.c_str(),(float)cfg->iNickFontSize);

	fCredits = new Gdiplus::Font(L"SansSerif",10);
	sbLabel = new Gdiplus::SolidBrush(*cLabel);
	pNodeBorder = new Gdiplus::Pen(*cNodeBorder,2);
	GetEncoderClsid(cfg->fWEncoderMime.c_str(),&encoderClsid);
	g->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

	fVidTimelapseTime = new Gdiplus::Font(L"Lucida Console", 72, Gdiplus::FontStyleBold);
	fVidTimelapseDate = new Gdiplus::Font(L"Lucida Console", 24, Gdiplus::FontStyleRegular);
	
	//cEdgeListTimeColor = new Gdiplus::Color(cfg->iEdgeNickListTimeColor.argb());
	//cEdgeListLineAdd = new Gdiplus::Color(cfg->iEdgeNickListLineAddColor.argb());
	//cEdgeListLineDel = new Gdiplus::Color(cfg->iEdgeNickListLineDelColor.argb());
	cChatDotColor = new Gdiplus::Color(cfg->iChatDotColor.argb());
	sbChatDotColor = new Gdiplus::SolidBrush(*cChatDotColor);
	fVidNick = new Gdiplus::Font(cfg->vidNickFont.c_str(),(float)cfg->iNickFontSize);

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
	delete cChatDotColor;
	delete sbChatDotColor;
}
