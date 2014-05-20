#pragma once
#include <Windows.h>
#include <Gdiplus.h>

class GraphConfig;

class GdiTools
{
public:
	GdiTools(const GraphConfig &cfg);
	~GdiTools(void);

	CLSID encoderClsid;

	Gdiplus::Bitmap *bmp;
	Gdiplus::Graphics *g;

	Gdiplus::Color *cBackground,*cChannel,*cLabel,*cTitle,*cNodeBorder,*cNode,*cEdge,*cBorder,*cChatDotColor;

	Gdiplus::SolidBrush *sbBackground,*sbChatDotColor;
	Gdiplus::Rect *rBackground;

	Gdiplus::Pen *pBorder,*pNodeBorder;

	Gdiplus::Font *fChannel,*fTitle,*fNick,*fCredits,*fVidTimelapseTime,*fVidTimelapseDate,*fVidNick;
	Gdiplus::PointF *pChannel,*pTitle;
	Gdiplus::SolidBrush *sbChannel,*sbTitle,*sbNode, *sbSecondaryNode,*sbLabel;
};
