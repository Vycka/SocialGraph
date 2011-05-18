#define _CRT_SECURE_NO_WARNINGS
#define VIDRENDER_MAX_FRAME_Q 60
#define VIDRENDER_BEGINFRAME 1000000
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

#include <list>
#include <map>


struct GraphRendererFrame
{
	std::wstring fName;
	Gdiplus::Bitmap *fbmp;
};

struct GraphRendererQueue
{
	GraphRendererQueue() : framesRendered(0), stopThreads(false), queueInUse(false) {};
	int framesRendered;
	bool stopThreads,queueInUse;
	std::queue<GraphRendererFrame*> renderQueoe;
	CLSID encoderClsid;
};

DWORD WINAPI graphRenderSaveStillQueoe(LPVOID lp)
{
	execInMirc("/echo -sg SocialGraph: Video Renderer Thread Created");
	GraphRendererQueue* gr = (GraphRendererQueue*)lp;
	while (true)
	{
		if (gr->renderQueoe.empty() || gr->queueInUse)
		{
			//execInMirc("/echo -sg SocialGraph: Renderer Queue is Empty!");		
			if (gr->stopThreads)
			{
				execInMirc("/echo -sg SocialGraph: Video Renderer Thread Exited");
				return 0;
			}
			Sleep(1);
		}
		else
		{
			gr->queueInUse = true;
			GraphRendererFrame *frame = gr->renderQueoe.front();
			gr->renderQueoe.pop();
			gr->queueInUse = false;
			frame->fbmp->Save(frame->fName.c_str(),&gr->encoderClsid);

			delete frame->fbmp;
			delete frame;

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

GraphVideo::GraphVideo(GraphConfig *cfg) : Graph(cfg,true)
{
	//overridinam kaikuriuos configus ir renderinam
	srand(12345); // kad nesikeistu perrenderinant randomas :)
	this->cfg->gCacheGdiTools = true;
	this->cfg->gSpringEmbedderIterations = this->cfg->vidSEIterationsPerFrame;
	gt = new GdiTools(this->cfg);
	this->cfg->logSave = false;
	vidRendFrame = VIDRENDER_BEGINFRAME;
	vidSecsPerFrame = 86400.0 / cfg->vidFramesPerDay;
	grq = new GraphRendererQueue;
	renderVideo();
}

GraphVideo::~GraphVideo()
{
	delete grq;
}

void GraphVideo::renderVideo()
{
	execInMirc("/.echo -sg SocialGraph: Starting Video Frames Rendering...");
	//initialize multithreated renderer stuff
	grq->encoderClsid = gt->encoderClsid;
	HANDLE *grh = new HANDLE[cfg->vidRendererThreads];
	for (int x = 0;x < cfg->vidRendererThreads;x++)
	{
		grh[x] = CreateThread(NULL,NULL,graphRenderSaveStillQueoe,grq,NULL,NULL);
		if (grh[x] == NULL)
		{
			execInMirc("/.echo SocialGraph: Unable to create renderer threads. Halting!");
			grq->stopThreads = true;
			for (int y = 0; y < x;y++)
				CloseHandle(grh[y]);
			delete [] grh;
			return;
		}
	}
	int timestamp,lastTime,key,activity;
	std::string n1,ln1,ln2;
	double weight;
	bool pauseRender = false;
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
		cfg->vidBeginRenderTime = 0x7FFFFFFF;
	while (ss.good() && timestamp < cfg->vidBeginRenderTime)
	{
		ss >> timestamp >> key;
		switch (key)
		{
		case VID_ADDNODE:
			if (!pauseRender)
				renderFrames(nextRender,lastTime);	
			ss >> n1 >> weight;
			addNode(&n1,weight);	
			if (!pauseRender)
				renderFrames(nextRender,timestamp);
			break;
		case VID_ADDEDGE:
			if (!pauseRender)
				renderFrames(nextRender,lastTime);			
			ss >> ln1 >> ln2 >> weight >> activity;
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
	delete [] grh;
	execInMirc("/echo -sg SocialGraph: Video Rendering Finished");
}

void GraphVideo::renderFrames(double &nextRender, int timestamp)
{
	static bool firstFrame = true;
	while (nextRender <= timestamp)
	{
		wchar_t frameNr[20];
		std::wstring dir = cfg->vidRenderPBegin;
		_itow(vidRendFrame++,frameNr,10);
		dir += frameNr;
		dir += cfg->vidRenderPEnd;
		if (firstFrame)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickBeforeRender);
			updateVisibleNodeList();
			//doLayout(iterations); first time dont do layout :P, so you will get a nice circle
			calcBounds();
			QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickAfterRender);
			drawImage(&dir,(int)nextRender);
			firstFrame = false;
		}
		else
			makeImage(cfg->vidSEIterationsPerFrame,&dir,(int)nextRender);
		nextRender += vidSecsPerFrame;

	}
}

void GraphVideo::renderRelocateNode(Node *n)
{
	static int nodeDeg = 0;
	double radian;
	const static double pirad = 3.14159265 / 180;
	if (n->getConEdges() == 0)
	{
		double cx = ((minX + maxX) / 2);
		double cy = ((minY + maxY) / 2);
		double rx = abs(maxX - minX) / 20;
		double ry = abs(maxY - minY) / 20;
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
	//std::cout << "XXYY: " << minX << " " << maxX << " " << minY<< " " << maxY << std::endl; 
	//backgroundo spalva
	gt->g->FillRectangle(gt->sbBackground,*gt->rBackground);
	//remas
	gt->g->DrawRectangle(gt->pBorder,*gt->rBackground);

	double width = cfg->iOutputWidth - cfg->gBorderSize * 3;
	double height = cfg->iOutputHeight - cfg->gBorderSize * 2;
	double borderSize = cfg->gBorderSize;
	gt->g->DrawString(cfg->nWChannel.c_str(),cfg->nWChannel.size(),gt->fChannel, *gt->pChannel ,gt->sbChannel);
	
	gt->g->DrawString(cfg->nWTitle.c_str(),cfg->nWTitle.size(),gt->fTitle,*gt->pTitle,gt->sbTitle);

	wcredits = L"Generated by SocialGraph DLL for mIRC http://code.google.com/p/socialgraphdll/. It is based on PieSpy.";
	gt->g->DrawString(wcredits.c_str(),wcredits.size(),gt->fCredits,
		Gdiplus::PointF((float)borderSize,(float)(height + borderSize * 2 - 5 - 15)),gt->sbTitle);
	
	char buff[64];
	_itoa(this->lastFrame,buff,10);
	std::string credits = "Frame #";
	credits += buff;
	credits += " was drawn on ";
	//gaunam laika norimam formate
	//laiko funkcijom naudojam _s variantus, tada dirbsim su mano kintamaisiais
	//o ne velniai zino su kokiais, ir tikiuos nebeliks JOBANO MEMORY LEAKO
	tm newTime;
	time_t clock = szClock;
	localtime_s(&newTime,(time_t*)&clock);
	asctime_s(buff,64,&newTime);
	credits += buff;
	credits.erase(credits.size()-1);
	credits += "... ";
	_itoa((int)cfg->gSpringEmbedderIterations,buff,10);
	credits += buff;
	credits += " Spring embedder iterations done in ";
	int msRendered = (int)((qpcTickAfterRender - qpcTickBeforeRender) / qpcTicksPerMs);
	_itoa(msRendered,buff,10);
	credits += buff;
	credits += "ms.";

	std::string tlTime = ctimeToTimeStr(szClock);
	std::string tlDate = ctimeToDateStr(szClock);
	std::wstring wtlTime, wtlDate;
	wtlTime.assign(tlTime.begin(),tlTime.end());
	wtlDate.assign(tlDate.begin(),tlDate.end());

	gt->g->DrawString(wtlTime.c_str(),wtlTime.size()-3,gt->fVidTimelapseTime,Gdiplus::PointF((float)cfg->iOutputWidth-325,(float)40),gt->sbChannel);
	gt->g->DrawString(wtlDate.c_str(),wtlDate.size(),gt->fVidTimelapseDate,Gdiplus::PointF((float)cfg->iOutputWidth-220,(float)10),gt->sbChannel);

	wcredits.assign(credits.begin(),credits.end());
	gt->g->DrawString(wcredits.c_str(),wcredits.size(),gt->fCredits,
		Gdiplus::PointF((float)borderSize,(float)(height + borderSize * 2 - 5 - 30)),gt->sbTitle);

	short int edgeInactivityMaxDiffR = cfg->iEdgeColor.r - cfg->iEdgeColorChangeInactive.r;
	short int edgeInactivityMaxDiffG = cfg->iEdgeColor.g - cfg->iEdgeColorChangeInactive.g;
	short int edgeInactivityMaxDiffB = cfg->iEdgeColor.b - cfg->iEdgeColorChangeInactive.b;
	short int edgeInactivityMaxDiffA = cfg->iEdgeColor.a - cfg->iEdgeColorChangeInactive.a;

	for (unsigned int x = 0;x < edges.size();x++)
	{
		if (edges[x]->getWeight() < cfg->gEdgeThreshold)
			continue;
		Node *nodeA = edges[x]->getSource();
		Node *nodeB = edges[x]->getTarget();
		double weight = edges[x]->getWeight();
		int x1 = (int) ((width * (nodeA->getX() - minX) / (maxX - minX)) + borderSize);
		int y1 = (int) ((height * (nodeA->getY() - minY) / (maxY - minY)) + borderSize);
		int x2 = (int) ((width * (nodeB->getX() - minX) / (maxX - minX)) + borderSize);
		int y2 = (int) ((height * (nodeB->getY() - minY) / (maxY - minY)) + borderSize);
		

		int eiDiffR = 0,eiDiffG = 0,eiDiffB = 0,eiDiffA = 0;
		int edgeInactivity = szClock - edges[x]->getActivityTime();
		double edgeInactivityMinMaxDiff = cfg->gEdgeColorChangeInactivityMax - cfg->gEdgeColorChangeInactivityMin;
		if (edgeInactivity >= cfg->gEdgeColorChangeInactivityMin)
		{
			if (edgeInactivity > cfg->gEdgeColorChangeInactivityMax)
				edgeInactivity = cfg->gEdgeColorChangeInactivityMax;
			edgeInactivity-= cfg->gEdgeColorChangeInactivityMin;
			double eiMul = edgeInactivity / edgeInactivityMinMaxDiff;
			eiDiffR = (int)(edgeInactivityMaxDiffR * eiMul);
			eiDiffG = (int)(edgeInactivityMaxDiffG * eiMul);
			eiDiffB = (int)(edgeInactivityMaxDiffB * eiMul);
			eiDiffA = (int)(edgeInactivityMaxDiffA * eiMul);
		}

		int alpha = 70 + (int)(185.0 * (weight / maxWeight));
		int alphaFinal = alpha-eiDiffA;
		if (alphaFinal < 20)
			alphaFinal = 20;
		Gdiplus::Pen p(Gdiplus::Color(alphaFinal,cfg->iEdgeColor.r-eiDiffR,cfg->iEdgeColor.g-eiDiffG,cfg->iEdgeColor.b-eiDiffB),((float)((alpha == 255 ? 254 : alpha) / 85) + 2)); 
 		gt->g->DrawLine(&p,x1,y1,x2,y2);
	}
	int nodeRadius = (int)cfg->gNodeRadius;
	for (unsigned int x = 0;x < visibleNodes.size();x++)
	{
		Node *n = visibleNodes[x];
		int x1 = (int) ((width * (n->getX() - minX) / (maxX - minX)) + borderSize);
		int y1 = (int) ((height * (n->getY() - minY) / (maxY - minY)) + borderSize);
		int newNodeRadius = (int) (log((n->getWeight() + 1) / 10) + nodeRadius);
		gt->g->FillEllipse(gt->sbNode,x1 - newNodeRadius,y1 - newNodeRadius,newNodeRadius*2,newNodeRadius*2);
		gt->g->DrawEllipse(gt->pNodeBorder,x1 - newNodeRadius,y1 - newNodeRadius,newNodeRadius*2,newNodeRadius*2);
		gt->g->DrawString(n->getWNick(),n->getNick()->size(),gt->fVidNick,
			Gdiplus::PointF((float)x1+newNodeRadius-1,(float)y1+newNodeRadius-1),gt->sbLabel);
	}

		GraphRendererFrame *frame = new GraphRendererFrame;
		frame->fName = *fWPath;
		frame->fbmp = gt->bmp->Clone(0,0,cfg->iOutputWidth,cfg->iOutputHeight,PixelFormat24bppRGB);
		while (grq->renderQueoe.size() >= VIDRENDER_MAX_FRAME_Q)
			Sleep(1);
		grq->renderQueoe.push(frame);
}

void GraphVideo::calcBounds()
{
	double tminX,tminY,tmaxX,tmaxY;
	static bool renderFirstTime = true;
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

	if (renderFirstTime)
	{
		minX = tminX;
		maxX = tmaxX;
		minY = tminY;
		maxY = tmaxY;
		renderFirstTime = false;
	}
	else
	{
		
		double dminX = minX - tminX;
		double dmaxX = maxX - tmaxX;
		double dminY = minY - tminY;
		double dmaxY = maxY - tmaxY;
		static double xyAR = cfg->iOutputWidth / cfg->iOutputHeight;
		static double xyDivX = 40.0;
		static double xyDivY = xyDivX / xyAR;
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
		minX = midX - (minSize / 2);
		maxX = midX + (minSize / 2);
	}
	if (maxY - minY < minSize)
	{
		double midY = (maxY + minY) / 2;
		minY = midY - (minSize / 2);
		maxY = midY + (minSize / 2);
	}

	for (unsigned int x = 0;x < edges.size();x++)
		if (edges[x]->getWeight() > maxWeight)
			maxWeight = edges[x]->getWeight();

		
	// Jibble the boundaries to maintain the aspect ratio.
	double xyRatio = ((maxX - minX) / (maxY - minY)) / (cfg->iOutputWidth / cfg->iOutputHeight);
	if (xyRatio > 1)
	{
		// diagram is wider than it is high.
		double dy = maxY - minY;
		dy = dy * xyRatio - dy;
		minY = minY - dy / 2;
		maxY = maxY + dy / 2;
	}
	else if (xyRatio < 1)
	{
		// Diagram is higher than it is wide.
		double dx = maxX - minX;
		dx = dx / xyRatio - dx;
		minX = minX - dx / 2;
		maxX = maxX + dx / 2;
	}
}

void GraphVideo::makeImage(int iterations, std::wstring *output,int tNow)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickBeforeRender);
	updateVisibleNodeList();
	doLayout(iterations);
	calcBounds();
	QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickAfterRender);
	drawImage(output,tNow);
}

void GraphVideo::doLayout(int gSpringEmbedderIterations)
{
	double k = cfg->gK;
	double c = cfg->gC;
	// Repulsive forces between nodes that are further apart than this are ignored.
	double maxRepulsiveForceDistance = cfg->gMaxRepulsiveForceDistance;
				
		// For each iteration...
	for (int it = 0; it < gSpringEmbedderIterations; it++) {
			
			// Calculate forces acting on nodes due to node-node repulsions...
		for (unsigned int a = 0; a < visibleNodes.size(); a++)
		{
			for (unsigned int b = a + 1; b < visibleNodes.size(); b++)
			{
				Node *nodeA = visibleNodes[a];
				Node *nodeB = visibleNodes[b];
					
				double deltaX = nodeB->getX() - nodeA->getX();
				double deltaY = nodeB->getY() - nodeA->getY();
					
				double distanceSquared = deltaX * deltaX + deltaY * deltaY;
										
				if (distanceSquared < 0.01)
				{
					deltaX = (rand() % 1000) / 10000 + 0.1;
					deltaY = (rand() % 1000) / 10000 + 0.1;
					distanceSquared = deltaX * deltaX + deltaY * deltaY;
				}
				
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
			{
				deltaX = (rand() % 1000) / 10000 + 0.1;
				deltaY = (rand() % 1000) / 10000 + 0.1;
				distanceSquared = deltaX * deltaX + deltaY * deltaY;
			}
				
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
		for (unsigned int a = 0; a < visibleNodes.size(); a++)
		{
			Node *node = visibleNodes[a];
			
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
			
	}
}

void GraphVideo::deleteUnusedNodes()
{
	updateVisibleNodeList();
	std::list <Node*> nodesToErase;
	for (std::map<std::string,Node*>::iterator iNodes = nodes.begin();iNodes != nodes.end();iNodes++)
	{	
		if (iNodes->second->getWeight() > cfg->gTemporalDecayAmount)
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
		edges[x]->setWeight(0);
}

void GraphVideo::addEdge(const std::string *ln1, const std::string *ln2, double weight,int activity)
{	
	Edge *e = findEdge(ln1,ln2);
	if (e)
	{
		e->appWeight(weight);
		e->updateActivityTime(activity);
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
		}
		if (!node2)
		{
			std::string mmsg("/echo -sg SocialGraph: Missing nick in log: ");
			mmsg += *ln2;
			execInMirc(&mmsg);
			node2 = addNode(ln2,ln2,0);
		}
		renderRelocateNode(node1);
		renderRelocateNode(node2);
		node1->appConEdges(1);
		node2->appConEdges(1);
		e = new Edge(node1,node2,weight,activity);
		edges.push_back(e);
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
		edges[x]->appWeight(-newDecay);
		if (edges[x]->getWeight() <= 0)
		{
			edges[x]->getSource()->appConEdges(-1);
			edges[x]->getTarget()->appConEdges(-1);
			delete edges[x];
			edges.erase(edges.begin()+x);
			x--;
		}
	}
}