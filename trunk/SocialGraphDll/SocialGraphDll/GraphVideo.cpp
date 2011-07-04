#define _CRT_SECURE_NO_WARNINGS
#define MAX_THREADS 16
#define VIDRENDER_MAX_FRAME_Q 5
#define VIDRENDER_BEGINFRAME 1000000
//border size is multiplied to compensate incremental min/max xy calculation, so nodes won't appear outside the image
#define VIDRENDER_BORDER_MUL_W 2.5
#define VIDRENDER_BORDER_MUL_H 1.7
#include "GraphVideo.h"
#include "Tools.h"
#include "Math.h"
#include "GdiTools.h"
#include "Logger.h"
#include "Node.h"
#include "Edge.h"
#include "GraphConfig.h"
#include <queue>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <list>
#include <map>

struct GvEdgeChatDot
{
	GvEdgeChatDot(int timeAdded = 0,float percentMoved = 0.0) : timeAdded(timeAdded), percentMoved(percentMoved) {};
	int timeAdded;
	double percentMoved;
};

struct GvEdgeData
{
	int sourceFrames,targetFrames;
	Gdiplus::Color sourceInactiveColor,targetInactiveColor;
	std::vector<GvEdgeChatDot> sourceChatDots;
	std::vector<GvEdgeChatDot> targetChatDots;
};

struct GvNodeData
{
	GvNodeData() : disconnectedStillVisible(false), lastKnownRadius(5), fadeOutSteps(0), fadeOutEnabled(false) {};
	bool disconnectedStillVisible;
	float lastKnownRadius;
	int fadeOutSteps;
	bool fadeOutEnabled;
	inline void fadeOutBegin() { fadeOutSteps = 0; fadeOutEnabled = true; };
	inline void fadeOutCancel() { fadeOutSteps = 0; fadeOutEnabled = false; };
};

struct GraphRendererFrame
{
	std::wstring fName;
	Gdiplus::Bitmap *fbmp;
};

struct GraphRendererQueue
{
	GraphRendererQueue() : framesRendered(0), stopThreads(false), lastThreadadded(0) {
	for (int x = 0; x < MAX_THREADS; x++)
		for (int y = 0; y < VIDRENDER_MAX_FRAME_Q; y++)
			renderQueoe[x][y] = NULL;
	};
	int framesRendered, lastThreadadded;
	bool stopThreads;
	GraphRendererFrame *renderQueoe[MAX_THREADS][VIDRENDER_MAX_FRAME_Q];
	CLSID encoderClsid;
};

struct GraphRendererThreadSync
{
	int threadId;
	GraphRendererQueue *grq;
};

DWORD WINAPI graphRenderSaveStillQueoe(LPVOID lp)
{
	GraphRendererThreadSync *grts = (GraphRendererThreadSync*)lp;
	GraphRendererQueue* gr = grts->grq;
	while (true)
	{
		bool isThereFrameToRender = false;
		int framePos = 0;
		for (int x = 0; x < VIDRENDER_MAX_FRAME_Q; x++)
			if (gr->renderQueoe[grts->threadId][x])
			{
				framePos = x;
				isThereFrameToRender = true;
			}
		if (!isThereFrameToRender)
		{
			//execInMirc("/echo -sg SocialGraph: Renderer Queue is Empty!");		
			if (gr->stopThreads)
			{
				std::stringstream ss;
				ss << "/echo -sg SocialGraph: Video Renderer Thread Exited - " << grts->threadId + 1;
				execInMirc(ss.str().c_str());
				return 0;
			}
			Sleep(1);
		}
		else
		{

			GraphRendererFrame *frame = gr->renderQueoe[grts->threadId][framePos];
			frame->fbmp->Save(frame->fName.c_str(),&gr->encoderClsid);

			delete frame->fbmp;
			delete frame;
			gr->renderQueoe[grts->threadId][framePos] = NULL;

			if ((++gr->framesRendered % 250) == 0)
			{
				char frameNr[20];
				_itoa(gr->framesRendered,frameNr,10);
				std::string mmsg = "/.echo -sg SocialGraph: Video Frames Rendered: ";
				mmsg += frameNr;
				execInMirc(&mmsg);			
			}
		}
	}
	return 0;
}

GraphVideo::GraphVideo(GraphConfig *config) : Graph(config,true)
{
	//overridinam kaikuriuos configus ir renderinam
	srand32(12345); // kad nesikeistu perrenderinant randomas :)
	
	cfg->gCacheGdiTools = true;
	
	//TODO: those overrides need to be cleaned up one day (Hopefully before stable v2.0 release)
	cfg->gSpringEmbedderIterations = cfg->vidSEIterationsPerFrame;
	cfg->gC = cfg->vidC;
	cfg->gMaxNodeMovement = cfg->vidMaxNodeMovement;
	cfg->iOutputWidth  = cfg->vidOutputWidth;
	cfg->iOutputHeight = cfg->vidOutputHeight;
	cfg->iNickFontSize = cfg->vidNickFontSize;
	cfg->gBorderSize   = cfg->vidBorderSize;
	cfg->gNodeRadius   = cfg->vidNodeRadius;
	if (cfg->vidRendererThreads > 16 || cfg->vidRendererThreads < 1)
	{
		cfg->vidRendererThreads = 4;
		execInMirc("/echo -sg SocialGraph: Bad vidRendererThreads value, defaulting it to 4");
	}

	gt = new GdiTools(cfg);
	cfg->logSave = false;
	vidRendFrame = VIDRENDER_BEGINFRAME;
	vidSecsPerFrame = 86400.0 / cfg->vidFramesPerDay;
	firstFrameRendered = false;
	nodeDeg = 0;
	grq = new GraphRendererQueue;
	grh = new HANDLE[cfg->vidRendererThreads];
	grts = new GraphRendererThreadSync[cfg->vidRendererThreads];
	grq->encoderClsid = gt->encoderClsid;
	
	nodeCoordCalcWidth = (int)(cfg->iOutputWidth - cfg->gBorderSize * VIDRENDER_BORDER_MUL_W * 2);
	nodeCoordCalcHeight = (int)(cfg->iOutputHeight - cfg->gBorderSize * VIDRENDER_BORDER_MUL_H * 2);
	nodeCoordCalcBorderX = (int)(cfg->gBorderSize * VIDRENDER_BORDER_MUL_W);
	nodeCoordCalcBorderY = (int)(cfg->gBorderSize * VIDRENDER_BORDER_MUL_H);

	xyAR = cfg->iOutputWidth / cfg->iOutputHeight;
	xyDivX = cfg->vidXYDivRatio;
	xyDivY = xyDivX / xyAR;
	xyDivX2 = xyDivX * 4;
	xyDivY2 = xyDivY * 4;
	xyDivX3 = xyDivX / 4;
	xyDivY3 = xyDivY / 4;
}

GraphVideo::~GraphVideo()
{
	delete grq;
	delete [] grh;
	delete [] grts;
}

void GraphVideo::renderVideo()
{
	int beginTime = (int)time(0);
	execInMirc("/.echo -sg SocialGraph: Starting Video Frames Rendering...");
	//initialize multithreated renderer stuff
	
	for (int x = 0;x < cfg->vidRendererThreads;x++)
	{
		grts[x].threadId = x;
		grts[x].grq = grq;
		grh[x] = CreateThread(NULL,NULL,graphRenderSaveStillQueoe,&grts[x],NULL,NULL);
		if (grh[x] == NULL)
		{
			execInMirc("/.echo SocialGraph: Unable to create renderer threads. Halting!");
			grq->stopThreads = true;
			WaitForMultipleObjects(x,grh,true,-1);
			for (int y = 0; y < x;y++)
				CloseHandle(grh[y]);
			return;
		}
		std::stringstream ss;
		ss << "/echo -sg SocialGraph: Video Renderer Thread Created: " << grts[x].threadId + 1; 
		execInMirc(ss.str().c_str());
	}
	int timestamp,lastTime,key,activity;
	std::string n1,ln1,ln2;
	double weight;
	pauseRender = false;
	std::fstream flog(cfg->logFile.c_str(),std::ios_base::in);
	std::string buffer;
	getline(flog,buffer,'\0');
	flog.close();
	std::stringstream ss(buffer);
	//read until first keyframe (VID_INIT)
	do
	{
		ss >> timestamp >> key;
	} while (key != VID_INIT && ss.good());
	lastTime = timestamp;
	double nextRender = (cfg->vidBeginRenderTime > timestamp ? cfg->vidBeginRenderTime : timestamp + 1);
	if (cfg->vidEndRenderTime <= 0)
		cfg->vidEndRenderTime = 0x7FFFFFFF;
	while (ss.good() && timestamp < cfg->vidEndRenderTime)
	{
		Node *n = NULL;
		ss >> timestamp >> key;
		switch (key)
		{
		case VID_ADDNODE:
			if (!pauseRender)
				renderFrames(nextRender,lastTime);	
			ss >> n1 >> weight;
			n = addNode(&n1,weight);
			if (!n->getUserData())
				n->setUserData(new GvNodeData);
			if (!pauseRender)
				renderFrames(nextRender,timestamp);
			break;
		case VID_ADDEDGE:
			if (!pauseRender)
				renderFrames(nextRender,lastTime);		
			ss >> ln1 >> ln2 >> weight >> activity;
			if (timestamp < activity)
			{
				std::stringstream ssEig;
				ssEig << "/echo @SocialGraph Time Continuity Error: " << ss.str();
				execInMirc(ssEig.str().c_str());
			}
			addEdge(&ln1,&ln2,weight,activity);
			if (!pauseRender)
			{
				renderFrames(nextRender,timestamp);
				decay(cfg->gTemporalDecayAmount,timestamp);
			}
			break;
		case VID_DELETENODE:
			ss >> ln1;
			deleteNode(&ln1);
			break;
		case VID_CLEAR:
			if (pauseRender)
				deleteUnusedNodes();
			else if (!pauseRender) //if VID_CLEAR is not in pause condition, that means someone manualy called clear(), 
				clear();			//and we can assume that graph is reseted,but technicly i dont think, thats posible.
			break;
		case VID_SETFRAME:
			ss >> lastFrame;
			break;
		case VID_PAUSE:
			pauseRender = true; 
			break;
		case VID_RESUME:
			pauseRender = false;
			decay(0,timestamp); //remove empty edges
			renderFrames(nextRender,timestamp);
			break;
		case VID_INIT:
			break;
		default:
			std::stringstream ssmmsg;
			ssmmsg << "/echo -sg SocialGraph: Unknown Log Key: " << timestamp << " " << key << " Seeking till the new line!";
			execInMirc(ssmmsg.str().c_str());
			ss.ignore(0x7FFFFFFF,'\n');
			//ss.setstate(std::ios::badbit);
			break;
		}
		lastTime = timestamp;
	}

	grq->stopThreads = true;
	WaitForMultipleObjects(cfg->vidRendererThreads,grh,true,-1);
	for (int x = 0;x < cfg->vidRendererThreads;x++)
		CloseHandle(grh[x]);
	execInMirc("/echo -sg SocialGraph: Video Rendering Finished");
	
	int diffTime = (int)time(0) - beginTime;
	ss.str("");
	ss << "/echo -sg SocialGraph: Frames rendered: " << grq->framesRendered-1 << " // Time took: " << diffTime << "seconds.";
	execInMirc(ss.str().c_str());
}

void GraphVideo::renderFrames(double &nextRender, int timestamp)
{
	while (nextRender <= timestamp)
	{
		wchar_t frameNr[20];
		std::wstring dir = cfg->vidRenderPBegin;
		_itow(vidRendFrame++,frameNr,10);
		dir += frameNr;
		dir += cfg->vidRenderPEnd;
		if (!firstFrameRendered)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickBeforeRender);
			updateVisibleNodeList();
			//doLayout(iterations); first time dont do layout :P, so you will get a nice circle
			calcBounds();
			mergeVisibleAndDisconnectedNodes();
			QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickAfterRender);
			drawImage(&dir,(int)nextRender);
		}
		else
			makeImage(cfg->vidSEIterationsPerFrame,&dir,(int)nextRender);
		nextRender += vidSecsPerFrame;

	}
}

void GraphVideo::relocateNode(Node *n)
{
	double radian;
	const static double pirad = 3.14159265 / 180;
	if (n->getConEdges() == 0)
	{
		double cx = ((minX + maxX) / 2);
		double cy = ((minY + maxY) / 2);
		double rx = abs(maxX - minX) / 4;
		double ry = abs(maxY - minY) / 4;
		radian = pirad * nodeDeg;
		cx += cos(radian) * rx;
		cy += sin(radian) * ry;
		nodeDeg = (nodeDeg + 25);
		n->setX(cx);
		n->setY(cy);
	}
}

void GraphVideo::drawImage(std::wstring *fWPath,int szClock)
{
	std::wstring wcredits;
	//filling background
	gt->g->FillRectangle(gt->sbBackground,*gt->rBackground);
	//frame
	gt->g->DrawRectangle(gt->pBorder,*gt->rBackground);


	gt->g->DrawString(cfg->nWChannel.c_str(),cfg->nWChannel.size(),gt->fChannel, *gt->pChannel ,gt->sbChannel);
	
	gt->g->DrawString(cfg->nWTitle.c_str(),cfg->nWTitle.size(),gt->fTitle,*gt->pTitle,gt->sbTitle);
	
	Gdiplus::StringFormat sfL,sfR;
	sfL.SetAlignment(Gdiplus::StringAlignmentFar);
	sfR.SetAlignment(Gdiplus::StringAlignmentNear);

	wcredits = L"Generated by SocialGraph DLL for mIRC http://code.google.com/p/socialgraphdll/";
	//TODO fix credits font
	gt->g->DrawString(wcredits.c_str(),wcredits.size(),gt->fVidNick,Gdiplus::PointF((float)(cfg->gBorderSize / 2),(float)(cfg->iOutputHeight - 25)),gt->sbTitle);
	//std::stringstream ss;
	//ss << "XXYY: " << minX << " " << maxX << " " << minY<< " " << maxY;
	//std::string tmp = ss.str();
	//std::wstring wcredits2;
	//wcredits2.assign(tmp.begin(),tmp.end());
	//gt->g->DrawString(wcredits2.c_str(),wcredits2.size(),gt->fVidNick,Gdiplus::PointF((float)(borderSize),(float)(cfg->iOutputHeight - 40)),gt->sbTitle);

	int eclWR = cfg->iOutputWidth - 105, eclWC = cfg->iOutputWidth - 135, eclWL = cfg->iOutputWidth - 136, eclClock = cfg->iOutputWidth - 243;
	double eclH = 130;
	for (std::list<GraphEdgeChangeList>::iterator i = edgeChangeList.begin(); i != edgeChangeList.end(); i++)
	{

		//TODO: Will need to make separate font/brush for these..

		GraphEdgeChangeList *l = &(*i);
		std::string stNow = ctimeToTimeStr(l->tNow);
		std::wstring wstNow;
		wstNow.assign(stNow.begin(),stNow.end());
		gt->g->DrawString(l->n1.c_str(),l->n1.size(),gt->fCredits,Gdiplus::PointF((float)eclWL,(float)eclH),&sfL,gt->sbLabel);
		gt->g->DrawString(l->n2.c_str(),l->n2.size(),gt->fCredits,Gdiplus::PointF((float)eclWR,(float)eclH),&sfR,gt->sbLabel);
		gt->g->DrawString(wstNow.c_str(),wstNow.size(),gt->fCredits,Gdiplus::PointF((float)eclClock,(float)eclH),&sfL,gt->sbTitle);
		Gdiplus::Color c;
		if ((*i).isAppearing)
			c = Gdiplus::Color(cfg->iEdgeColor.a,cfg->iEdgeColor.r,cfg->iEdgeColor.g,cfg->iEdgeColor.b);
		else
			c = Gdiplus::Color(cfg->iEdgeColorChangeInactive.a,cfg->iEdgeColorChangeInactive.r,cfg->iEdgeColorChangeInactive.g,cfg->iEdgeColorChangeInactive.b);

		Gdiplus::Pen p(c,3);
		int lineHeight = (int)(eclH + 8);
		gt->g->DrawLine(&p,eclWC+2,lineHeight,eclWC+26,lineHeight);

		//eclH += cfg->iNickFontSize + 3;
		eclH += 15;
	}


	std::string tlTime = ctimeToTimeStr(szClock);
	std::string tlDate = ctimeToDateStr(szClock);
	std::wstring wtlTime, wtlDate;
	wtlTime.assign(tlTime.begin(),tlTime.end());
	wtlDate.assign(tlDate.begin(),tlDate.end());

	gt->g->DrawString(wtlTime.c_str(),wtlTime.size()-3,gt->fVidTimelapseTime,Gdiplus::PointF((float)cfg->iOutputWidth-325,(float)40),gt->sbChannel);
	gt->g->DrawString(wtlDate.c_str(),wtlDate.size(),gt->fVidTimelapseDate,Gdiplus::PointF((float)cfg->iOutputWidth-220,(float)10),gt->sbChannel);

	short int edgeInactivityMaxDiffR = cfg->iEdgeColor.r - cfg->iEdgeColorChangeInactive.r;
	short int edgeInactivityMaxDiffG = cfg->iEdgeColor.g - cfg->iEdgeColorChangeInactive.g;
	short int edgeInactivityMaxDiffB = cfg->iEdgeColor.b - cfg->iEdgeColorChangeInactive.b;

	for (std::vector<Edge*>::iterator ei = edges.begin(); ei != edges.end(); ei++)
	{
		if ((*ei)->getWeight() < cfg->gEdgeThreshold)
			continue;
		Node *nodeA = (*ei)->getSource();
		Node *nodeB = (*ei)->getTarget();
		double weight = (*ei)->getWeight();
		float x1 = (float)getNodeFinalCoordX(nodeA);
		float y1 = (float)getNodeFinalCoordY(nodeA);
		float x2 = (float)getNodeFinalCoordX(nodeB);
		float y2 = (float)getNodeFinalCoordY(nodeB);
		float lenX = abs(x1 - x2);
		float lenY = abs(y1 - y2);
		float ratioXYYX = (lenX > lenY ? lenY / lenX : lenX / lenY);
		

		int eiDiffR = 0,eiDiffG = 0,eiDiffB = 0,eiDiffA = 0;
		int edgeInactivity = szClock - (*ei)->getActivityTime();
		double edgeInactivityMinMaxDiff = cfg->gEdgeColorChangeInactivityMax - cfg->gEdgeColorChangeInactivityMin;
		double eiMul = 0.0;
		if (edgeInactivity >= cfg->gEdgeColorChangeInactivityMin)
		{
			if (edgeInactivity > cfg->gEdgeColorChangeInactivityMax)
				edgeInactivity = cfg->gEdgeColorChangeInactivityMax;
			edgeInactivity-= cfg->gEdgeColorChangeInactivityMin;
			eiMul = edgeInactivity / edgeInactivityMinMaxDiff;
			eiDiffR = (int)(edgeInactivityMaxDiffR * eiMul);
			eiDiffG = (int)(edgeInactivityMaxDiffG * eiMul);
			eiDiffB = (int)(edgeInactivityMaxDiffB * eiMul);
		}


		//edge alpha calculation
		const static double maxWeightStrength = 255;
		const static double weightToAlphaDiv = maxWeightStrength / (maxWeightStrength - cfg->iEdgeActiveMinAlpha);

		double weightStrength = maxWeightStrength * (weight / maxWeight);
		double alphaFinal = (weightStrength / weightToAlphaDiv) + cfg->iEdgeActiveMinAlpha;
		if (alphaFinal > cfg->iEdgeColor.a)
			alphaFinal = cfg->iEdgeColor.a;
		alphaFinal -= (int)((alphaFinal - cfg->iEdgeColorChangeInactive.a) * eiMul);
		if (alphaFinal < cfg->iEdgeColorChangeInactive.a)
			alphaFinal = cfg->iEdgeColorChangeInactive.a;

		float finalThickness = (float)((weightStrength / 75) + 2); // 255/75=3.4

		Gdiplus::Pen p(Gdiplus::Color((int)alphaFinal,cfg->iEdgeColor.r-eiDiffR,cfg->iEdgeColor.g-eiDiffG,cfg->iEdgeColor.b-eiDiffB),finalThickness); 
 		gt->g->DrawLine(&p,x1,y1,x2,y2);


		GvEdgeData *ged = (GvEdgeData*)(*ei)->getUserData();
		float cdRadius = (float)cfg->vidEdgeChatDotRadius;
		float cdHalfRadius = cdRadius / 2;
		//const static float thicknessCorrectionTable[6] = {1, 1, 1, 2, 4, 5}; //workaround
		float thicknessCorrection = finalThickness / 2;  //TODO: simplify all this in the future....
		double edgeLength = sqrt(pow((double)x1-x2,2)+pow((double)y1-y2,2));

		//std::wstringstream wFT;
		//wFT << finalThickness << " || " << thicknessCorrection;
		//gt->g->DrawString(wFT.str().c_str(),wFT.str().size(),gt->fNick,Gdiplus::PointF((x1+x2) / 2, (y1+y2) / 2),gt->sbChatDotColor);

		for (unsigned int x = 0; x < ged->sourceChatDots.size(); x++)
		{
			GvEdgeChatDot *gecd = &ged->sourceChatDots[x];

			double xRange = x2 - x1;
			double m = (y2 - y1) / (xRange);
			
			gecd->percentMoved += (float)((100.0 / edgeLength) * cfg->vidEdgeChatDotSpeedPixelsPerFrame);
			if (gecd->percentMoved >= 100)
			{
				ged->sourceChatDots.erase(ged->sourceChatDots.begin() + x);
				x--;
				continue;
			}

			double fx = (xRange / 100.0) * gecd->percentMoved;
			float dotPosX = (float)((fx + x1));
			float dotPosY = (float)((m * fx) + y1);

			double dotPercentVector = gecd->percentMoved + ((100.0 / edgeLength) * thicknessCorrection);
			double vfx = (xRange / 100.0) * dotPercentVector;
			float vDotPosX = (float)((m * vfx) + y1) - dotPosY;
			float vDotPosY = (float)(((vfx + x1)) - dotPosX) * -1;

			//gt->g->DrawLine(gt->pBorder,dotPosX,dotPosY,dotPosX + vDotPosX,dotPosY + vDotPosY);
			gt->g->FillEllipse(gt->sbChatDotColor,dotPosX - cdHalfRadius + vDotPosX,dotPosY - cdHalfRadius + vDotPosY,cdRadius,cdRadius);
		}
		
		for (unsigned int x = 0; x < ged->targetChatDots.size(); x++)
		{
			GvEdgeChatDot *gecd = &ged->targetChatDots[x];

			double xRange = x1 - x2;
			double m = (y1 - y2) / (xRange);
			gecd->percentMoved += (100.0 / edgeLength) * cfg->vidEdgeChatDotSpeedPixelsPerFrame;

			if (gecd->percentMoved >= 100)
			{
				ged->targetChatDots.erase(ged->targetChatDots.begin() + x);
				x--;
				continue;
			}

			double fx = (xRange / 100.0) * gecd->percentMoved;
			float dotPosX = (float)((fx + x2));
			float dotPosY = (float)((m * fx) + y2);

			double dotPercentVector = gecd->percentMoved + ((100.0 / edgeLength) * thicknessCorrection);
			double vfx = (xRange / 100.0) * dotPercentVector;
			float vDotPosX = (float)((m * vfx) + y2) - dotPosY;
			float vDotPosY = (float)(((vfx + x2)) - dotPosX) * -1;


			gt->g->FillEllipse(gt->sbChatDotColor,dotPosX - cdHalfRadius + vDotPosX,dotPosY - cdHalfRadius + vDotPosY,cdRadius,cdRadius);
			//gt->g->DrawLine(gt->pBorder,dotPosX,dotPosY,dotPosX + vDotPosX,dotPosY + vDotPosY);
		}
	}
	int nodeRadius = (int)cfg->gNodeRadius;
	for (unsigned int x = 0;x < visibleNodes.size();x++)
	{
		Node *n = visibleNodes[x];
		GvNodeData* nData = ((GvNodeData*)n->getUserData());
		

		int x1 = getNodeFinalCoordX(n);
		int y1 = getNodeFinalCoordY(n);

		nData->lastKnownRadius = (float)(log((n->getWeight() + 1) / 10) + nodeRadius);

		if (nData->disconnectedStillVisible)
		{
			if (nData->fadeOutEnabled)
			{
				nData->fadeOutSteps++;

				CColor ccLabel(cfg->iLabelColor.a / nData->fadeOutSteps,cfg->iLabelColor.r,cfg->iLabelColor.g,cfg->iLabelColor.b);
				CColor ccNodeBorder(cfg->iNodeBorderColor.a / nData->fadeOutSteps,cfg->iNodeBorderColor.r,cfg->iNodeBorderColor.g,cfg->iNodeBorderColor.b);

				Gdiplus::Pen fadeOutpNodeBorder(ccNodeBorder.argb(),1.0);
				Gdiplus::SolidBrush fadeOutsbLabel(ccLabel.argb());

				//lol 1x
				gt->g->DrawEllipse(&fadeOutpNodeBorder,x1 -nData->lastKnownRadius,y1 - nData->lastKnownRadius,nData->lastKnownRadius*2,nData->lastKnownRadius*2);
				gt->g->DrawString(n->getWNick(),n->getNick()->size(),gt->fVidNick,Gdiplus::PointF((float)x1+nData->lastKnownRadius-1,(float)y1+nData->lastKnownRadius-1),&fadeOutsbLabel);
			}
			else
			{
				//lol 2x
				gt->g->DrawEllipse(gt->pNodeBorder,x1 -nData->lastKnownRadius,y1 - nData->lastKnownRadius,nData->lastKnownRadius*2,nData->lastKnownRadius*2);
				gt->g->DrawString(n->getWNick(),n->getNick()->size(),gt->fVidNick,Gdiplus::PointF((float)x1+nData->lastKnownRadius-1,(float)y1+nData->lastKnownRadius-1),gt->sbLabel);
			}

		}
		else
		{
			gt->g->FillEllipse(gt->sbNode,x1 - nData->lastKnownRadius,y1 - nData->lastKnownRadius,nData->lastKnownRadius*2,nData->lastKnownRadius*2);
			//lol 3x
			gt->g->DrawEllipse(gt->pNodeBorder,x1 -nData->lastKnownRadius,y1 - nData->lastKnownRadius,nData->lastKnownRadius*2,nData->lastKnownRadius*2);
			gt->g->DrawString(n->getWNick(),n->getNick()->size(),gt->fVidNick,Gdiplus::PointF((float)x1+nData->lastKnownRadius-1,(float)y1+nData->lastKnownRadius-1),gt->sbLabel);
		}
		//need better solution here one day :/
	}

	GraphRendererFrame *frame = new GraphRendererFrame;
	frame->fName = *fWPath;
	frame->fbmp = gt->bmp->Clone(0,0,cfg->iOutputWidth,cfg->iOutputHeight,PixelFormat24bppRGB);
	
	int writeThread = 0, writePos = 0;
	bool posFound = false;
	do
	{
		posFound = false;
		while (!posFound)
		{
			grq->lastThreadadded = (grq->lastThreadadded + 1) % cfg->vidRendererThreads;
			for (int x = 0; x < VIDRENDER_MAX_FRAME_Q; x++)
				if (!grq->renderQueoe[grq->lastThreadadded][x])
				{
					writeThread = grq->lastThreadadded;
					writePos = x;
					posFound = true;
					break;
				}
		}
	} while (!posFound);
	SuspendThread(grh[writeThread]);
	grq->renderQueoe[writeThread][writePos] = frame;
	ResumeThread(grh[writeThread]);
	if (!firstFrameRendered)
		firstFrameRendered = true;
}

void GraphVideo::calcBounds()
{
	double tminX,tminY,tmaxX,tmaxY;

	if (visibleNodes.size() > 0)
	{
		tminX = visibleNodes[0]->getX();
		tmaxX = visibleNodes[0]->getX();
		tminY = visibleNodes[0]->getY();
		tmaxY = visibleNodes[0]->getY();
	}
	else
	{
		tminX = 0;
		tmaxX = 0;
		tminY = 1;
		tmaxY = 1;
	}
	maxWeight = 0;

	for (unsigned int x = 0; x < visibleNodes.size();x++)	
	{
		if (visibleNodes[x]->getX() > tmaxX)
			tmaxX = visibleNodes[x]->getX();

		if (visibleNodes[x]->getX() < tminX)
			tminX = visibleNodes[x]->getX();

		if (visibleNodes[x]->getY() > tmaxY)
			tmaxY = visibleNodes[x]->getY();

		if (visibleNodes[x]->getY() < tminY)
			tminY = visibleNodes[x]->getY();
	}

	if (!firstFrameRendered)
	{
		minX = tminX;
		maxX = tmaxX;
		minY = tminY;
		maxY = tmaxY;
	}
	else
	{
		
		double dminX = minX - tminX;
		double dmaxX = maxX - tmaxX;
		double dminY = minY - tminY;
		double dmaxY = maxY - tmaxY;
		minX -= dminX / xyDivX;
		maxX -= dmaxX / xyDivX;
		minY -= dminY / xyDivY;
		maxY -= dmaxY / xyDivY;
	}

	
	// Increase size if too small.
	double minSize = cfg->gMinDiagramSize;
	if (maxX - minX < minSize)
	{
		double midX = (maxX + minX) / 2;
		double deltaMinX = minX - (midX - (minSize / 2));
		double deltaMaxX = maxX - (midX + (minSize / 2));
		minX -= deltaMinX / xyDivX2;
		maxX -= deltaMaxX / xyDivX2; 
	}
	if (maxY - minY < minSize)
	{
		double midY = (maxY + minY) / 2;
		double deltaMinY = minY - (midY - (minSize / 2));
		double deltaMaxY = maxY - (midY + (minSize / 2));
		minY -= deltaMinY / xyDivY2;
		maxY -= deltaMaxY / xyDivY2;
	}

	for (unsigned int x = 0;x < edges.size();x++)
		if (edges[x]->getWeight() > maxWeight)
			maxWeight = edges[x]->getWeight();

	if (maxWeight < cfg->gMinMaxWeight)
		maxWeight = cfg->gMinMaxWeight;
	

	// Jibble the boundaries to maintain the aspect ratio.
	double xyRatio = ((maxX - minX) / (maxY - minY)) / (cfg->iOutputWidth / cfg->iOutputHeight);
	if (xyRatio > 1)
	{
		// diagram is wider than it is high.
		double dy = maxY - minY;
		dy = dy * xyRatio - dy;
		double deltaMinY = minY - (minY - dy / 2);
		double deltaMaxY = maxY - (maxY + dy / 2);
		minY -= deltaMinY / xyDivY3;
		maxY -= deltaMaxY / xyDivY3;
	}
	else if (xyRatio < 1)
	{
		// Diagram is higher than it is wide.
		double dx = maxX - minX;
		dx = dx / xyRatio - dx;
		double deltaMinX = minX - (minX - dx / 2);
		double deltaMaxX = maxX - (maxX + dx / 2);
		minX -= deltaMinX / xyDivX3;
		maxX -= deltaMaxX / xyDivX3; 
	}
}

void GraphVideo::makeImage(int iterations, std::wstring *output,int tNow)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickBeforeRender);
	updateVisibleNodeList();
	doLayout(iterations);
	calcBounds();

	for (unsigned int x = 0; x < visibleDisconnectedNodes.size(); x++)
	{
		Node *n = visibleDisconnectedNodes[x];
		GvNodeData *nData = (GvNodeData*)n->getUserData();
		if (!isNodeInFrame(n) || nData->fadeOutSteps > cfg->vidDisconnectedFadeOutFrames)
		{
			
			nData->disconnectedStillVisible = false;
			nData->fadeOutCancel();
			visibleDisconnectedNodes.erase(visibleDisconnectedNodes.begin() + x);
			x--;
		}
		else if (!isNodeWithinBorder(n) && !nData->fadeOutEnabled)
		{
			nData->fadeOutBegin();
		}
	}
	mergeVisibleAndDisconnectedNodes();
	QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickAfterRender);
	drawImage(output,tNow);
}

void GraphVideo::doLayout(int gSpringEmbedderIterations)
{
	double k = cfg->gK;
	double c = cfg->gC;
	double cDC = cfg->vidCDisconnected;
	// Repulsive forces between nodes that are further apart than this are ignored.
	double maxRepulsiveForceDistance = cfg->gMaxRepulsiveForceDistance;
				
		// For each iteration...
	for (int it = 0; it < gSpringEmbedderIterations; it++) {
			
			// Calculate forces acting on nodes due to node-node repulsions...
		for (std::vector<Node*>::iterator ai = visibleNodes.begin(); ai != visibleNodes.end(); ai++)
		{
			for (std::vector<Node*>::iterator bi = ai + 1; bi != visibleNodes.end(); bi++)
			{
				Node *nodeA = *ai;
				Node *nodeB = *bi;
					
				double deltaX = nodeB->getX() - nodeA->getX();
				double deltaY = nodeB->getY() - nodeA->getY();
					
				double distanceSquared = deltaX * deltaX + deltaY * deltaY;
										
				if (distanceSquared < 0.01)
					distanceSquared = (rand32(2000) / 10000) + 0.1;
				
				double distance = sqrt(distanceSquared);
				
				if (distance < maxRepulsiveForceDistance) {
					double repulsiveForce = (k * k / distance);
					
					nodeB->setFX(nodeB->getFX() + (repulsiveForce * deltaX / distance));
					nodeB->setFY(nodeB->getFY() + (repulsiveForce * deltaY / distance));
					nodeA->setFX(nodeA->getFX() - (repulsiveForce * deltaX / distance));
					nodeA->setFY(nodeA->getFY() - (repulsiveForce * deltaY / distance));
				}
			}
		}
		

		//Repeat calculations for visible but disconnected nodes
		for (std::vector<Node*>::iterator ai = visibleNodes.begin(); ai != visibleNodes.end(); ai++)
		{
			for (std::vector<Node*>::iterator bi = visibleDisconnectedNodes.begin(); bi != visibleDisconnectedNodes.end(); bi++)
			{
				Node *nodeA = *ai;
				Node *nodeB = *bi;
				
				if (nodeA == nodeB)
				{
					execInMirc("/echo @SocialGraph: Logical error: Identical nodes in the Visible/Disconnected loop!");
					continue;
				}

				double deltaX = nodeB->getX() - nodeA->getX();
				double deltaY = nodeB->getY() - nodeA->getY();
					
				double distanceSquared = deltaX * deltaX + deltaY * deltaY;
										
				if (distanceSquared < 0.01)
					distanceSquared = (rand32(2000) / 10000) + 0.1;
				
				double distance = sqrt(distanceSquared);
				
				double repulsiveForce = (k * k / distance);
					
				nodeB->setFX(nodeB->getFX() + (repulsiveForce * deltaX / distance));
				nodeB->setFY(nodeB->getFY() + (repulsiveForce * deltaY / distance));
			}
		}

		// Calculate forces acting on nodes due to edge attractions.
		for (unsigned int e = 0; e < edges.size(); e++)
		{
			Edge *edge = edges[e];
			Node *nodeA = edge->getSource();
			Node *nodeB = edge->getTarget();
			
			double deltaX = nodeB->getX() - nodeA->getX();
			double deltaY = nodeB->getY() - nodeA->getY();
			
			double distanceSquared = deltaX * deltaX + deltaY * deltaY;
	
			// Avoid division by zero error or Nodes flying off to
			// infinity.  Pretend there is an arbitrary distance between
			// the Nodes.
			if (distanceSquared < 0.01)
				distanceSquared = (rand32(2000) / 10000) + 0.1;
				
			double distance = sqrt(distanceSquared);
				
			if (distance >  maxRepulsiveForceDistance)
				distance = maxRepulsiveForceDistance;

			distanceSquared = distance * distance;
				
			double attractiveForce = (distanceSquared - k * k) / k;
				
			// Make edges stronger if people know each other.
			double weight = edge->getWeight();
			if (weight < 1) 
				weight = 1;
	
			attractiveForce *= (log(weight) * 0.5) + 1;
			
			nodeB->setFX(nodeB->getFX() - attractiveForce * deltaX / distance);
			nodeB->setFY(nodeB->getFY() - attractiveForce * deltaY / distance);
			nodeA->setFX(nodeA->getFX() + attractiveForce * deltaX / distance);
			nodeA->setFY(nodeA->getFY() + attractiveForce * deltaY / distance);
				
		}
			
		// Now move each node to its new location...
		for (std::vector<Node*>::iterator it = visibleNodes.begin(); it != visibleNodes.end(); it++)
		{
			Node *node = *it;
			
			double xMovement = c * node->getFX();
			double yMovement = c * node->getFY();
			
			// Limit movement values to stop nodes flying into oblivion.
			double max = cfg->gMaxNodeMovement;
			if (xMovement > max) 
				xMovement = max;
			
			else if (xMovement < -max) 
				xMovement = -max;
			
			if (yMovement > max) 
				yMovement = max;
			
			else if (yMovement < -max)
				yMovement = -max;
			
			
			node->setX(node->getX() + xMovement);
			node->setY(node->getY() + yMovement);
			
			// Reset the forces
			node->setFX(0);
			node->setFY(0);
		}
		
		//move visible disconnected nodes to the new location
		for (std::vector<Node*>::iterator it = visibleDisconnectedNodes.begin(); it != visibleDisconnectedNodes.end(); it++)
		{
			Node *node = *it;
			
			double xMovement = cDC * node->getFX();
			double yMovement = cDC * node->getFY();
			
			// Limit movement values to stop nodes flying into oblivion.
			double max = cfg->vidMaxNodeMovementDisconnected;
			if (xMovement > max) 
				xMovement = max;
			
			else if (xMovement < -max) 
				xMovement = -max;
			
			if (yMovement > max) 
				yMovement = max;
			
			else if (yMovement < -max)
				yMovement = -max;
			
			
			node->setX(node->getX() + xMovement);
			node->setY(node->getY() + yMovement);
			
			// Reset the forces
			node->setFX(0);
			node->setFY(0);
		}
	}
}

void GraphVideo::deleteUnusedNodes()
{
	updateVisibleNodeList();
	std::list <Node*> nodesToErase;
	for (std::map<std::string,Node*>::iterator iNodes = nodes.begin();iNodes != nodes.end();iNodes++)
	{	
		//method called only in pausedRender, so after this one nodes weight will be updated from next LOG_INIT
		if (iNodes->second->getWeight() > cfg->gTemporalDecayAmount || ((GvNodeData*)iNodes->second->getUserData())->disconnectedStillVisible)
		{
			iNodes->second->setWeight(0);
			continue;
		}
		bool found = false;
		for (unsigned int x = 0;x < visibleNodes.size();x++)
		{		
			if (visibleNodes[x] == iNodes->second)
			{
				found = true;
				iNodes->second->setWeight(0);
				break;
			}
		}
		if (!found) //jei nerado tokio naudojamo, desim prie nenaudojamu saraso
			nodesToErase.push_back(iNodes->second);
	}
	for (std::list<Node*>::iterator iErase = nodesToErase.begin(); iErase != nodesToErase.end();iErase++)
	{
		std::map<std::string,Node*>::iterator iNode = nodes.find(*(*iErase)->getLNick());
		delete iNode->second;
		nodes.erase(iNode);

	}
	for (unsigned int x = 0;x < edges.size();x++)
	{
		edges[x]->setWeight(0);
		edges[x]->setChangedInPause(true);
	}
}

void GraphVideo::addEdge(const std::string *ln1, const std::string *ln2, double weight,int activity)
{	
	Edge *e = findEdge(ln1,ln2);
	bool isInputFromSource = true;
	double wLast = 0.0;
	if (e)
	{
		wLast = e->getWeight();
		e->appWeight(weight);
		e->updateActivityTime(activity);
		if (!e->getChangedInPause())
			e->updateActivityTimeForNick(*ln1,activity);
		if (*e->getSource()->getLNick() != *ln1)
			isInputFromSource = false;
	}
	else	
	{
		Node *node1 = findNode(ln1);
		Node *node2 = findNode(ln2);

		if (!node1)
		{
			std::string mmsg("/echo -sg SocialGraph: missing nick in log: ");
			mmsg += *ln1;
			execInMirc(&mmsg);
			node1 = addNode(ln1,ln1,0);
			relocateNode(node1);
		}
		if (!node2)
		{
			std::string mmsg("/echo -sg SocialGraph: Missing nick in log: ");
			mmsg += *ln2;
			execInMirc(&mmsg);
			node2 = addNode(ln2,ln2,0);
			relocateNode(node2);
		}
		if (((GvNodeData*)node1->getUserData())->disconnectedStillVisible)
		{
			((GvNodeData*)node1->getUserData())->disconnectedStillVisible = false;
			deleteDisconnectedVisibleNode(node1);
		}
		else
			relocateNode(node1);
		

		if (((GvNodeData*)node2->getUserData())->disconnectedStillVisible)
		{
			((GvNodeData*)node2->getUserData())->disconnectedStillVisible = false;
			deleteDisconnectedVisibleNode(node2);
		}
		else
			relocateNode(node2);

		//if (!isNodeInFrame(node1))
		//	relocateNode(node1);

		//if (!isNodeInFrame(node2))
		//	relocateNode(node2);

		node1->appConEdges(1);
		node2->appConEdges(1);
		e = new Edge(node1,node2,weight,activity);
		e->updateActivityTimeForSource(activity);
		e->setUserData(new GvEdgeData);
		edges.push_back(e);
	}
	if (cfg->vidEdgeChatDotEnabled && !pauseRender && activity >= cfg->vidBeginRenderTime)
	{
		GvEdgeData *ged = (GvEdgeData*)e->getUserData();
		if (isInputFromSource)
		{
			if (ged->sourceChatDots.empty() || ged->sourceChatDots.rbegin()->percentMoved != 0) //dont add extra dots if they get overlapped pixel-by-pixel anyway
				ged->sourceChatDots.push_back(GvEdgeChatDot(activity));
		}
		else
		{
			if (ged->targetChatDots.empty() || ged->targetChatDots.rbegin()->percentMoved != 0)
				ged->targetChatDots.push_back(GvEdgeChatDot(activity));
		}

	}
	if (wLast <= cfg->gEdgeThreshold && e->getWeight() > cfg->gEdgeThreshold)
	{
		if (e->getChangedInPause())
		{
			e->setChangedInPause(false);
			return;
		}
		GraphEdgeChangeList gecl(activity,e->getSource()->getWNick(),e->getTarget()->getWNick(),true);
		addEdgeChangeList(gecl);
	}
}

void GraphVideo::decay(double d, int tNow)
{
	for (std::map<std::string,Node*>::iterator i = nodes.begin();i != nodes.end();i++)
	{
		i->second->appWeight(-d);
		if (i->second->getWeight() < 0)
			i->second->setWeight(0);

	}
	for (unsigned int x = 0;x < edges.size();x++)
	{
		//tipo extra saugiklis kad senesni edges greiciau mazetu
		double mul = ((tNow - edges[x]->getActivityTime()) / cfg->gEdgeDecayMultiplyIdleSecs);
		mul *= mul;
		double newDecay = (d * mul) + d;
		Edge *e = edges[x];
		double wLast = e->getWeight();
		e->appWeight(-newDecay);
		if (wLast > cfg->gEdgeThreshold && e->getWeight() <= cfg->gEdgeThreshold)
		{
			GraphEdgeChangeList gecl(tNow,e->getSource()->getWNick(),e->getTarget()->getWNick(),false);
			addEdgeChangeList(gecl);
		}

		if (e->getWeight() <= 0)
		{
			e->getSource()->appConEdges(-1);
			e->getTarget()->appConEdges(-1);
			if (tNow >= cfg->vidBeginRenderTime)
			{

				//Here's the only place where disconnectedStillVisible can become true.
				//cancel fadeout here if it was active

				if (e->getSource()->getConEdges() == 0)
				{
					Node *n = e->getSource();
					GvNodeData *nData = (GvNodeData*)n->getUserData();
					visibleDisconnectedNodes.push_back(n);
					nData->disconnectedStillVisible = true;
					nData->fadeOutCancel();

				}
				if (e->getTarget()->getConEdges() == 0)
				{
					Node *n = e->getTarget();
					GvNodeData *nData = (GvNodeData*)n->getUserData();
					visibleDisconnectedNodes.push_back(n);
					nData->disconnectedStillVisible = true;
					nData->fadeOutCancel();
				}
			}
			delete e;
			edges.erase(edges.begin()+x);
			x--;
		}
	}
}

void GraphVideo::deleteDisconnectedVisibleNode(Node *n)
{
	for (std::vector<Node*>::iterator i = visibleDisconnectedNodes.begin(); i != visibleDisconnectedNodes.end(); i++)
		if (n == *i)
		{
			visibleDisconnectedNodes.erase(i);
			return;
		}
}

void GraphVideo::mergeVisibleAndDisconnectedNodes()
{
	visibleNodes.insert(visibleNodes.end(),visibleDisconnectedNodes.begin(),visibleDisconnectedNodes.end());
}

void GraphVideo::clear()
{
	for (std::map<std::string,Node*>::iterator i = nodes.begin();i != nodes.end();i++)
		delete i->second;
	for (unsigned int x = 0;x < edges.size();x++)
		delete edges[x];


	nodes.clear();
	edges.clear();
	visibleNodes.clear();
	visibleDisconnectedNodes.clear();
	//std::stringstream ss;
	//ss << "/echo @SocialGraph Video: called clear():" << this->lastFrame;
	//execInMirc(ss.str().c_str());
}

void GraphVideo::deleteNode(const std::string *lnick)
{

	std::map<std::string,Node*>::iterator iNode = nodes.find(*lnick);
	if (iNode == nodes.end())
		return;

	if (((GvNodeData*)iNode->second->getUserData())->disconnectedStillVisible)
		return;


	//if node has any connected edges, set edges weight to 0 and let decay() handle the rest.
	//Otherwise node might get deleted without becoming disconnected and flying off
	//TODO: TEST THIS HOTFIX!!!
	if (iNode->second->getConEdges())
	{
		for (std::vector<Edge*>::iterator i = edges.begin(); i != edges.end(); i++)
		{
			Edge *e = *i;
			if (e->getSource() == iNode->second || e->getTarget() == iNode->second)
				e->setWeight(0);
		}
		return;
	}


	//delete node
	delete iNode->second;
	nodes.erase(iNode);
}

bool GraphVideo::isNodeInFrame(Node *n)
{
	int x = getNodeFinalCoordX(n);
	int y = getNodeFinalCoordY(n);
	GvNodeData* nData = ((GvNodeData*)n->getUserData());
	if (x + nData->lastKnownRadius > 0  && x < cfg->iOutputWidth && y + nData->lastKnownRadius > 0 && y < cfg->iOutputHeight)
		return true;
	return false;
}

bool GraphVideo::isNodeWithinBorder(Node *n)
{
	//return false;
	int x = getNodeFinalCoordX(n);
	int y = getNodeFinalCoordY(n);
	GvNodeData* nData = ((GvNodeData*)n->getUserData());
	if (x + nData->lastKnownRadius > cfg->gBorderSize && x < cfg->iOutputWidth - cfg->gBorderSize &&
		y + nData->lastKnownRadius > cfg->gBorderSize && y < cfg->iOutputHeight - cfg->gBorderSize)
		return true;
	return false;
}