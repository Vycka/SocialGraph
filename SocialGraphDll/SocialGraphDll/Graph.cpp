#define _CRT_SECURE_NO_WARNINGS
#define VIDRENDER_MAX_FRAME_Q 60
#define VIDRENDER_BEGINFRAME 1000000
#include "Graph.h"
#include "Tools.h"
#include "Node.h"
#include "Edge.h"
#include "Config.h"
#include "GdiTools.h"
#include "Logger.h"
#include "InferenceHeuristic.h"
#include "AdjacencyInferenceHeuristic.h"
#include "BinarySequenceInferenceHeuristic.h"
#include "DirectAddressingInferenceHeuristic.h"
#include "IndirectAddressingInferenceHeuristic.h"
#include <sstream>
#include <iostream>
#include <set>
#include <queue>
#include <Math.h>

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

Graph::Graph(const Config *cfg,bool videoRendering)
{
	isVideoRenderingGraph = videoRendering;
	this->cfg = new Config(*cfg);
#ifdef COM_EXE
	this->cfg->logSave = false;	
#endif
	gt = NULL;

	lastFrame = 0;
	lastRender = 0;
	lastUpload = 0;
	minX = 0;
	maxX = 75; //big numbers here so node relocator places them nicely, before real mins/maxes be calculated
	minY = 0;
	maxY = 75;

	inferences.push_back(new AdjacencyInferenceHeuristic(this,cfg->hAdjacency));
	inferences.push_back(new BinarySequenceInferenceHeuristic(this,cfg->hBinary));
	inferences.push_back(new DirectAddressingInferenceHeuristic(this,cfg->hDirect));
	inferences.push_back(new IndirectAddressingInferenceHeuristic(this,cfg->hIndirect));

	QueryPerformanceFrequency((LARGE_INTEGER*)&qpcTicksPerMs);
	qpcTicksPerMs = qpcTicksPerMs / 1000;

	if (!videoRendering)
		initGraphForLogging();
	else
	{
		//overridinam kaikuriuos configus ir renderinam
		srand(12345); // kad nesikeistu perrenderinant randomas :)
		this->cfg->gCacheGdiTools = true;
		this->cfg->gSpringEmbedderIterations = this->cfg->vidSEIterationsPerFrame;
		gt = new GdiTools(this->cfg);
		this->cfg->logSave = false;
		vidRendFrame = VIDRENDER_BEGINFRAME;
		vidSecsPerFrame = 86400 / cfg->vidFramesPerDay;
		grq = new GraphRendererQueue;
		renderVideo();
	}
}

void Graph::initGraphForLogging()
{
	if (cfg->logSave)
	{
		logger = new Logger(cfg->logFile.c_str());
		logger->wPause();
	}
	else
		logger = NULL;

	
	loadFromFile(cfg->fGraphOutput.c_str());

	if (cfg->gCacheGdiTools)
		gt = new GdiTools(this->cfg);
	
	std::string mmsg = "/.signal SocialGraph addChan " + cfg->nChannel;
	execInMirc(&mmsg);

#ifndef COMP_EXE
	deleteUnusedNodes();
#endif
	if (cfg->logSave)
		logger->wResume();
}

Graph::~Graph(void)
{
	if (!isVideoRenderingGraph)
	{
		std::string mmsg = "/.signal SocialGraph delChan " + cfg->nChannel;
		execInMirc(&mmsg);

		dumpToFile(cfg->fGraphOutput.c_str());
	}
	else
		delete grq;

	if (cfg->logSave)
		logger->wPause();

	for (unsigned int x = 0;x < inferences.size();x++)
		delete inferences[x];

	clear();
	inferences.clear();

	if (cfg->gCacheGdiTools && gt != NULL)
		delete gt;

	if (cfg->logSave)
		delete logger;

	delete cfg;
}


void Graph::clear()
{
	if (cfg->logSave)
		logger->wEnd();
	for (std::map<std::string,Node*>::iterator i = nodes.begin();i != nodes.end();i++)
		delete i->second;
	for (unsigned int x = 0;x < edges.size();x++)
		delete edges[x];


	nodes.clear();
	edges.clear();
	ignoreNicks.clear();
	visibleNodes.clear();
}

Node *Graph::addNode(const std::string *nick,const std::string *lnick,double weight)
{
	//Node *fNode = nodes[*lnick]; //kazko tai buggina taip
	std::map<std::string,Node*>::iterator iNode = nodes.find(*lnick);
	if (iNode != nodes.end())
	{
		iNode->second->appWeight(weight);
		if (cfg->logSave) //logger node
			if (weight != 0.0)
				logger->wAddNode(iNode->second,weight);
		return iNode->second;
	}
	else
	{
		Node *n = new Node(nick,lnick,weight);
		nodes.insert(std::make_pair(*lnick,n));
		if (cfg->logSave) //logger node
			logger->wAddNode(n,weight);
		return n;
	}

}

Node* Graph::addNode(const std::string *nick,double weight)
{
	std::string lnick;
	strToLower(nick,&lnick);
	return addNode(nick,&lnick,weight);
}

void Graph::deleteNode(const std::string *lnick)
{

	std::map<std::string,Node*>::iterator iNode = nodes.find(*lnick);
	if (iNode == nodes.end())
	{
		return;
	}

	if (cfg->logSave)
		logger->wDelNode(iNode->second);

	//jei toks yra, naikinam visus edges susijusius su tuo node
	for (unsigned int x = 0;x < edges.size();x++)
		if (edges[x]->getSource() == iNode->second || edges[x]->getTarget() == iNode->second)
		{
			edges[x]->getSource()->appConEdges(-1);
			edges[x]->getTarget()->appConEdges(-1);
			delete edges[x];
			edges.erase(edges.begin()+x);
			x--;
		}
	//ant galo naikinam node
	delete iNode->second;
	nodes.erase(iNode);
}

Node *Graph::findNode(const std::string *lnick)
{
	std::map<std::string,Node*>::iterator iNode = nodes.find(*lnick);
	return (iNode != nodes.end() ? iNode->second : NULL);
}

void Graph::addEdge(const std::string *ln1, const std::string *ln2, double weight)
{
	if (*ln1 == *ln2 || weight == 0.0)
		return;

	//jei yra toks edge, padidinam weight, jei ner, kuriam/kraunam
	Edge *edge = findEdge(ln1,ln2);
	if (edge)
	{
		edge->appWeight(weight);
		edge->updateActivityTime();
		if (cfg->logSave)
			logger->wAddEdge(edge,weight);

	}
	else
	{
		//tokie tikrai yra cia tik siaip reikia surast ju adresus
		Node *source = findNode(ln1);
		Node *target = findNode(ln2);
		source->appConEdges(1);
		target->appConEdges(1);
		Edge *e = new Edge(source,target,weight);
		edges.push_back(e);
		if (cfg->logSave)
			logger->wAddEdge(e,weight);
	}
	updateFrame();	
}

void Graph::updateEdge(Edge *e, double weight)
{
	if (weight == 0.0)
		return;
	e->appWeight(weight);
	e->updateActivityTime();
	if (cfg->logSave)
		logger->wAddEdge(e,weight);
	updateFrame();
}

Edge* Graph::findEdge(const std::string *ln1, const std::string *ln2)
{
	for (unsigned int x = 0;x < edges.size();x++)
	{
		if (edges[x]->sameNicks(ln1,ln2))
		{
			return edges[x];
		}
	}
	return NULL;
}

void Graph::onMessage(const std::string *nick,const std::string *msg)
{
	std::string lnick;
	strToLower(nick,&lnick);

	if (findIgnore(&lnick))
		return;

	std::string lmsg;
	strToLower(msg,&lmsg);

	addNode(nick,&lnick,cfg->gNodeBumpWeight);

	for (unsigned int x = 0;x < inferences.size();x++)
		inferences[x]->infer(&lnick,&lmsg);
}

void Graph::onJoin(const std::string *nick)
{
	std::string lnick;
	strToLower(nick,&lnick);

	if (findIgnore(&lnick))
		return;
	addNode(nick,&lnick);
}

void Graph::onMessage(const char *nick, const char *msg)
{
	std::string nnick(nick);
	std::string nmsg(msg);
	onMessage(&nnick,&nmsg);
}

void Graph::printLists()
{
	std::cout << "----Nodes----\n";
	for (std::map<std::string,Node*>::iterator i = nodes.begin();i != nodes.end();i++)
		std::cout << i->first << " | " << *i->second->getNick() << " " << i->second->getWeight()
		<< " " << i->second->getX() << " " << i->second->getY() << std::endl;
	std::cout << "----Edges----\n";
	for (unsigned int x = 0;x < edges.size();x++)
		std::cout << *edges[x]->getSource()->getNick() << " " << *edges[x]->getTarget()->getNick()
		<< " " << edges[x]->getWeight() << " " << edges[x]->getActivityTime() << std::endl;
	//std::cout << "----Inferences----\n";
	//for (unsigned int x = 0;x < inferences.size();x++)
	//	std::cout << inferences[x] << "  " << inferences[x]->getName() << std::endl;
	updateVisibleNodeList();
	std::cout << "----Visible Nodes----\n";
	for (unsigned int x = 0;x < visibleNodes.size();x++)
		std::cout << *visibleNodes[x]->getNick() << " " << visibleNodes[x]->getWeight() << " "
		<< visibleNodes[x]->getX() << " " << visibleNodes[x]->getY() << std::endl;
	std::cout << "----Ignore Nicks----\n";
	for (std::set<std::string>::iterator i = ignoreNicks.begin();i != ignoreNicks.end();i++)
		std::cout << *i << std::endl;
}

void Graph::updateVisibleNodeList()
{
	std::set<Node*> uniqNodes;
	for (unsigned int x = 0;x < edges.size();x++)
	{
		uniqNodes.insert(edges[x]->getSource());
		uniqNodes.insert(edges[x]->getTarget());
	}
	visibleNodes.clear();
	for (std::set<Node*>::iterator iun = uniqNodes.begin();iun != uniqNodes.end();iun++)
		visibleNodes.push_back(*iun);
}

void Graph::loadFromFile(const char *fn)
{
	std::fstream f(fn,std::ios_base::in);
	if (!f.good())
		return;

	clear();
	int n = 0;
	int timeSaved;
	f >> lastFrame >> timeSaved >> n;
	for (int x = 0;x < n;x++)
	{
		Node *node = new Node(&f);
		nodes.insert(std::make_pair(*node->getLNick(),node));
		if (cfg->logSave)
			logger->wAddNode(node,node->getWeight());
	}

	f >> n;
	int tPassed = ((int)time(NULL) - timeSaved);
	for (int x = 0;x < n;x++)
	{
		std::string n1,n2,ln1,ln2;
		double weight;
		int secs;
		f >> n1 >> n2 >> weight >> secs;
		strToLower(&n1,&ln1);
		strToLower(&n2,&ln2);
		Node *node1 = findNode(&ln1);
		Node *node2 = findNode(&ln2);
		node1->appConEdges(1);
		node2->appConEdges(1);
		Edge *e = new Edge(node1,node2,weight,secs + tPassed);
		edges.push_back(e);
		if (cfg->logSave)
			logger->wAddEdge(e,weight);
	}

	f >> n;
	for (int x = 0;x < n;x++)
	{
		std::string n1;
		f >> n1;
		ignoreNicks.insert(n1);
	}
	if (!f.good()) //kazkas atsitiko blogo skaityme
	{
		lastFrame = 0;
		clear();
	}
	if (cfg->logSave)
		logger->wFrame(lastFrame);
	f.close();
}

void Graph::dumpToFile(const char *fn)
{

#ifdef COMP_EXE
	return;
#endif
	std::fstream f(fn,std::ios_base::out);
	f << lastFrame << ' ' << (int)time(NULL) <<  std::endl;
	f << nodes.size() << std::endl;
	for (std::map<std::string,Node*>::iterator i = nodes.begin();i != nodes.end();i++)
	{
		Node *n = i->second;
		f << *n->getNick() << "\t\t" << n->getX() << '\t' << n->getY() << '\t' << n->getWeight() << std::endl;
	}

	f << std::endl << edges.size() << std::endl;
	
	for (unsigned int x = 0;x < edges.size();x++)
	{	
		Edge *e = edges[x];
		f << *e->getSource()->getNick() << "\t" << *e->getTarget()->getNick() << "\t" << e->getWeight()
			<< "\t" << e->getActivityTime() << std::endl;
	}

	f << std::endl << ignoreNicks.size() << std::endl;

	for (std::set<std::string>::iterator i = ignoreNicks.begin();i != ignoreNicks.end();i++)
		f << *i << std::endl;

	f.close();
}

void Graph::decay(double d, int tNow)
{
	for (std::map<std::string,Node*>::iterator i = nodes.begin();i != nodes.end();i++)
	{
#ifdef COMP_EXE
		break;
#endif
		i->second->appWeight(-d);
		if (i->second->getWeight() < 0)
			i->second->setWeight(0);

	}
	for (unsigned int x = 0;x < edges.size();x++)
	{
		//tipo extra saugiklis kad senesni edges greiciau mazetu
		double newDecay = (d * ((tNow - edges[x]->getActivityTime()) / cfg->gEdgeDecayMultiplyIdleSecs)) + d;
#ifdef COMP_EXE
		std::cout << *edges[x]->getSource()->getNick() << "\t" << *edges[x]->getTarget()->getNick() << "\t"
			<< edges[x]->getWeight() << "\t" << edges[x]->getActivityTime() << "\t" << newDecay << std::endl;
		continue;
#endif
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

void Graph::deleteUnusedNodes()
{
	updateVisibleNodeList();
	std::list <Node*> nodesToErase;
	for (std::map<std::string,Node*>::iterator iNodes = nodes.begin();iNodes != nodes.end();iNodes++)
	{	
		if (iNodes->second->getWeight() > cfg->gTemporalDecayAmount)
			continue;
		bool found = false;
		for (unsigned int x = 0;x < visibleNodes.size();x++)
		{		
			if (visibleNodes[x] == iNodes->second)
			{
				found = true;
				break;
			}
		}
		if (!found) //jei nerado tokio naudojamo, desim prie nenaudojamu saraso
			nodesToErase.push_back(iNodes->second);
	}
	for (std::list<Node*>::iterator iErase = nodesToErase.begin(); iErase != nodesToErase.end();iErase++)
	{
		std::map<std::string,Node*>::iterator iNode = nodes.find(*(*iErase)->getLNick());
		if (cfg->logSave)
			logger->wDelNode(iNode->second);
		delete iNode->second;
		nodes.erase(iNode);
	}
	if (!this->isVideoRenderingGraph)
	{
		std::string mmsg = "/.signal SocialGraph getNicks " + cfg->nChannel;
		execInMirc(&mmsg);
	}
}

void Graph::addIgnore(const char *lnick)
{
	std::string ln(lnick);
	addIgnore(&ln);
}

void Graph::addIgnore(const std::string *lnick)
{

	ignoreNicks.insert(*lnick);
	deleteNode(lnick);
}

bool Graph::findIgnore(const std::string *lnick)
{
	return (ignoreNicks.find(*lnick) != ignoreNicks.end());
}

void Graph::deleteIgnore(const std::string *lnick)
{
	ignoreNicks.erase(*lnick);
}

void Graph::updateFrame()
{
	int timeNow = (int)time(NULL);

	if ((timeNow - lastRender) > (unsigned int)cfg->gMinPauseBeforeNextRender) //leidziam piest kas gMinPauseBeforeNextRender sec
	{	
		lastRender = timeNow;
		//RENDER IMAGE
		this->lastFrame++; //padidinam frame
		
		if (cfg->logSave) //loger stuff
			logger->wFrame(lastFrame);

		makeImage(); //surenderinam
		dumpToFile(cfg->fGraphOutput.c_str());
		char buff[15];
		_itoa(lastFrame,buff,10);

		//genFrame SIGNAL
		std::string mmsg = "/.signal SocialGraph genFrame " + cfg->nChannel + " " + std::string(buff);
		execInMirc(&mmsg);
#ifdef GRAPHNOTICES
		mmsg = "/echo -sg SocialGraph: Next Frame Generated: #";
		mmsg += buff + std::string(" Channel: ") + cfg->nChannel;
		execInMirc(&mmsg);
#endif
		//genFrame SIGNAL END

		//SAVE OLD FRAMES STUFF
		if (cfg->oSaveOldFrames)
			saveOldFrame();
		//SAVE OLD FRAMES END

		//FTP UPLOAD
		if (cfg->ftpUpload)
			if ((timeNow - lastUpload) > (unsigned int)cfg->gMinPauseBeforeNextUpload)
			{
				lastUpload = timeNow;
				upload();
			}
		//FTP UPLOAD END
	}
	decay(cfg->gTemporalDecayAmount);
}

void Graph::makeImage(int iterations, std::wstring *output,int tNow)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickBeforeRender);
	updateVisibleNodeList();
	doLayout(iterations);
	calcBounds();
	QueryPerformanceCounter((LARGE_INTEGER*)&qpcTickAfterRender);
	drawImage(output,tNow);
}

void Graph::makeImage()
{
	makeImage(cfg->gSpringEmbedderIterations,&cfg->fWImageOutput);
}
void Graph::doLayout(int gSpringEmbedderIterations)
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

void Graph::calcBounds()
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
		minX = 0;
		maxX = 0;
		minY = 1;
		maxY = 1;
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
	
	if (!isVideoRenderingGraph)
	{
		minX = tminX;
		maxX = tmaxX;
		minY = tminY;
		maxY = tmaxY;
	}
	else
	{
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
			minX -= dminX / 10;
			maxX -= dmaxX / 10;
			minY -= dminY / 10;
			maxY -= dmaxY / 10;
		}
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

void Graph::drawImage(std::wstring *fWPath,int szClock)
{
	if (!cfg->gCacheGdiTools)
		gt = new GdiTools(cfg);
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

	wcredits = L"Generated by algorithm based on Piespy http://www.jibble.org/piespy/ and rewrited to C++ DLL for mIRC";
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
	wcredits.assign(credits.begin(),credits.end());
	gt->g->DrawString(wcredits.c_str(),wcredits.size(),gt->fCredits,
		Gdiplus::PointF((float)borderSize,(float)(height + borderSize * 2 - 5 - 30)),gt->sbTitle);

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
		
		int alpha = 55 + (int)(200 * weight / maxWeight);
		Gdiplus::Pen p(Gdiplus::Color(alpha,cfg->iEdgeColor.r,cfg->iEdgeColor.g,cfg->iEdgeColor.b),((float)(alpha / 85) + 1)); 
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
		gt->g->DrawString(n->getWNick(),n->getNick()->size(),gt->fNick,
			Gdiplus::PointF((float)x1+newNodeRadius-1,(float)y1+newNodeRadius-1),gt->sbLabel);
		//some debug stuff
		/*if (this->isVideoRenderingGraph)
		{
			wchar_t cNodesW[10];
			_itow(n->getConEdges(),cNodesW,10);
			gt->g->DrawString(cNodesW,2,gt->fNick,Gdiplus::PointF((float)x1-15,(float)y1-6),gt->sbLabel);
		}*/
	}
	if (this->isVideoRenderingGraph)
	{
		GraphRendererFrame *frame = new GraphRendererFrame;
		frame->fName = *fWPath;
		frame->fbmp = gt->bmp->Clone(0,0,cfg->iOutputWidth,cfg->iOutputHeight,PixelFormat24bppRGB);
		while (grq->renderQueoe.size() >= VIDRENDER_MAX_FRAME_Q)
			Sleep(1);
		grq->renderQueoe.push(frame);
	}
	else
		gt->bmp->Save(fWPath->c_str(),&gt->encoderClsid,NULL);

	if (!cfg->gCacheGdiTools)
		delete gt;
}

void Graph::upload()
{
	char buff[16];

#ifdef GRAPHNOTICES
	std::string mmsg = "/echo -sg SocialGraph: Uploading Frame: #";
	mmsg += _itoa(lastFrame,buff,10) + std::string(" Channel: ") + cfg->nChannel;
	execInMirc(&mmsg);
#endif

	DWORD err = ftpUpload(cfg->ftpHost.c_str(),cfg->ftpPort,cfg->ftpUser.c_str(),cfg->ftpPass.c_str(),
		cfg->fImageOutput.c_str(),cfg->ftpDir.c_str(),cfg->ftpFile.c_str());
	if (err)
	{
		std::string s("/echo -sg SocialGraph: Error uploading to FTP - Channel: " + cfg->nChannel + ", Error Code: ");
		_itoa(err,buff,10);
		s += buff + std::string(", Extended Err: ") + ftpGetExtendedError(); 
		execInMirc(&s);
	}
}

void Graph::saveOldFrame()
{
			char buff[16];
			if (!CopyFile(cfg->fImageOutput.c_str(),std::string(cfg->oPathBegin+_itoa(lastFrame,buff,10)+cfg->oPathEnd).c_str(),true))
			{
#ifdef GRAPHNOTICES
				std::string mmsg = "/echo -sg SocialGraph: Error Saving old frame. Error Code: ";
				mmsg += _itoa(GetLastError(),buff,10);
				execInMirc(&mmsg);
#endif
			}
}


Config* Graph::getConfig()
{
	return cfg;
}
void Graph::renderVideo()
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
	Node *node1,*node2;
	Edge *e;
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
	int nextRender = timestamp + vidSecsPerFrame;
	while (ss.good())
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
			e = findEdge(&ln1,&ln2);
			if (e)
			{
				e->appWeight(weight);
				e->updateActivityTime(activity);
			}
			else
			{
				node1 = findNode(&ln1);
				node2 = findNode(&ln2);
				renderRelocateNode(node1);
				renderRelocateNode(node2);
				node1->appConEdges(1);
				node2->appConEdges(1);
				e = new Edge(node1,node2,weight,activity);
				edges.push_back(e);
			}
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
			{
				deleteUnusedNodes();
				for (std::map<std::string,Node*>::iterator i = nodes.begin();i != nodes.end();i++)
					i->second->setWeight(0);
				for (unsigned int x = 0;x < edges.size();x++)
					edges[x]->setWeight(0);
			}
			else if (!pauseRender) //if VID_CLEAR is not in pause condition, that means someone manualy called clear(), 
				clear();			//and we can assume that graph is reseted,but technicly i dont think, thats posible.
			break;
		case VID_SETFRAME:
			ss >> lastFrame;
			break;
		case VID_PAUSE:
			if (!pauseRender)
				pauseRender = true; 
			break;
		case VID_RESUME:
			pauseRender = false;
			break;
		case VID_INIT:
			break;
		default:
			std::stringstream ssmmsg;
			ssmmsg << "/echo -sg SocialGraph: Unknown Log Key: " << timestamp << " " << key;
			execInMirc(ssmmsg.str().c_str());
			ss.setstate(std::ios::badbit);
			break;
		}
		lastTime = timestamp;
	}
	//fd.close();
	grq->stopThreads = true;
	WaitForMultipleObjects(cfg->vidRendererThreads,grh,true,-1);
	for (int x = 0;x < cfg->vidRendererThreads;x++)
		CloseHandle(grh[x]);
	delete [] grh;
	execInMirc("/echo -sg SocialGraph: Video Rendering Finished");
}

void Graph::renderFrames(int &nextRender, int timestamp)
{
	static bool firstFrame = true;
	while (nextRender < timestamp)
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
			drawImage(&dir,nextRender);
			firstFrame = false;
		}
		else
			makeImage(cfg->vidSEIterationsPerFrame,&dir,nextRender);
		nextRender += vidSecsPerFrame;

	}
}

void Graph::renderRelocateNode(Node *n)
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