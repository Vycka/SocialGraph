#pragma once
#include <Windows.h>
#include <Gdiplus.h>

class GraphConfig;

class GdiTools
{
public:
	GdiTools(GraphConfig *cfg);
	~GdiTools(void);

	CLSID encoderClsid;

	Gdiplus::Bitmap *bmp;
	Gdiplus::Graphics *g;

	Gdiplus::Color *cBackground,*cChannel,*cLabel,*cTitle,*cNodeBorder,*cNode,*cEdge,*cBorder;

	Gdiplus::SolidBrush *sbBackground;
	Gdiplus::Rect *rBackground;

	Gdiplus::Pen *pBorder,*pNodeBorder;

	Gdiplus::Font *fChannel,*fTitle,*fNick,*fCredits;
	Gdiplus::PointF *pChannel,*pTitle;
	Gdiplus::SolidBrush *sbChannel,*sbTitle,*sbNode,*sbLabel;
};
