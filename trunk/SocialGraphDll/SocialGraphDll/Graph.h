#pragma once
//#define HEURISTIC_NOTICES
#define GRAPHNOTICES
#include <map>
#include <set>
#include <vector>
#include <list>
#include <string>
#include <Windows.h>
#include <time.h>
#include "Node.h"
#include "EdgeChangeListRecord.h"


class GdiTools;
class GraphConfig;
class InferenceHeuristic;
class Logger;
class Edge;
class GraphDataFileHandler;


//TODO CLASS GraphEdgeChangeList


class Graph
{
public:
	Graph(const GraphConfig &cfg,bool videoRendering = false);
	~Graph(void);
	void initGraphForLogging();
	virtual void clear();
	Node* addNode(const std::string *nick,const std::string *lnick,double weight = 0.0);
	Node* addNode(const std::string *nick,double weight = 0.0);
	virtual void deleteNode(const std::string *lnick);
	Node* findNode(const std::string *lnick);
	void addEdge(const std::string *ln1, const std::string *ln2, double weight = 0);
	void updateEdge(Edge *e, double weight,const Node *inputFrom);
	Edge* findEdge(const std::string &ln1, const std::string &ln2);
	void onMessage(const std::string *nick, const std::string *msg);
	void onMessage(const char *nick, const char *msg);
	void onJoin(const std::string *nick);
	void updateVisibleNodeList();
	void printLists();
	void saveToFile(const char *fn);
	//saveToFileEx Only for debug pusposes to bypass COMP_EXE without editing saveToFile function.
	bool saveToFileEx(const char *fn);
	void loadFromFile(const char *fn);
	virtual void decay(double d,int tNow = (int)time(NULL));
	virtual void deleteUnusedNodes();
	void addIgnore(const char *lnick);
	void addIgnore(const std::string *lnick);
	bool findIgnore(const std::string *lnick);
	void deleteIgnore(const std::string *lnick);
	void updateFrame();
	virtual void makeImage(int iterations, std::wstring *output,int tNow = (int)time(NULL));
	void makeImage();
	virtual void doLayout(int gSpringEmbedderIterations);
	virtual void calcBounds();
	virtual void drawImage(std::wstring *fWPath,int szClock);
	void upload();
	void saveOldFrame();
	GraphConfig* getConfig();
	void reloadConfig(const GraphConfig &newConfig);
	double calculateDecay(const Edge &e,const double &d, const int &tNow);
	//inline bool isNodeInFrame(Node *n, double extraBorderX = 0.0, double extraBorderY = 0.0) { return (n->getX() >= minX - extraBorderX && n->getY() >= minY - extraBorderY && n->getX() <= maxX + extraBorderX && n->getY() <= maxY + extraBorderY ? true : false); };

	friend GraphDataFileHandler;
protected:
	void addEdgeChangeList(int timeBegin, Edge *edge);
	void addEdgeChangeList(int timeBegin, int timeLast,const std::string &n1,const std::wstring &wn1, const std::string &n2,const std::wstring &wn2, Edge *e);

	double minX,maxX,minY,maxY;
	GdiTools *gt;
	GraphConfig *cfg;
	std::map<std::string,Node*> nodes;
	std::vector<Edge*> edges;
	unsigned int lastFrame;
	__int64 qpcTicksPerMs,qpcTickBeforeRender,qpcTickAfterRender;
	double maxWeight;
	std::vector<Node*> visibleNodes;
	std::list<EdgeChangeListRecord*> edgeChangeList;
private:
	//opens/appends log file, writes VID_PAUSE command, populates list based on current edges,nodes lists, and writes VID_RESUME command.
	void loggerStart(const std::string &fname);
	//writes VID_PAUSE, VID_CLEAR headers closes the file and deletes logger object.
	void loggerStop();

	std::vector<InferenceHeuristic*> inferences;
	std::set<std::string> ignoreNicks;
	unsigned int lastRender,lastUpload;
	bool isVideoRenderingGraph;
	Logger *logger;
	GraphDataFileHandler *graphDataFileHandle;
};
