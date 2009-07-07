#pragma once
//#define HEURISTIC_NOTICES
#define GRAPHNOTICES
#include <map>
#include <set>
#include <vector>
#include <string>
#include <Windows.h>
#include <time.h>
#include <fstream>

class GdiTools;
class Config;
class Node;
class Edge;
class InferenceHeuristic;
class Logger;

struct GraphRendererQueue;

class Graph
{
public:
	Graph(const Config *cfg,bool videoRendering = false);
	~Graph(void);
	void initGraphForLogging();
	void clear();
	Node* addNode(const std::string *nick,const std::string *lnick,double weight = 0.0);
	Node* addNode(const std::string *nick,double weight = 0.0);
	void deleteNode(const std::string *lnick);
	Node* findNode(const std::string *lnick);
	void addEdge(const std::string *ln1, const std::string *ln2, double weight = 0);
	void updateEdge(Edge *e, double weight = 0);
	Edge* findEdge(const std::string *ln1, const std::string *ln2);
	void onMessage(const std::string *nick, const std::string *msg);
	void onMessage(const char *nick, const char *msg);
	void onJoin(const std::string *nick);
	void updateVisibleNodeList();
	void printLists();
	void dumpToFile(const char *fn);
	void loadFromFile(const char *fn);
	void decay(double d,int tNow = (int)time(NULL));
	void deleteUnusedNodes();
	void addIgnore(const char *lnick);
	void addIgnore(const std::string *lnick);
	bool findIgnore(const std::string *lnick);
	void deleteIgnore(const std::string *lnick);
	void updateFrame();
	void makeImage(int iterations, std::wstring *output,int tNow = (int)time(NULL));
	void makeImage();
	void doLayout(int gSpringEmbedderIterations);
	void calcBounds();
	void drawImage(std::wstring *fWPath,int szClock);
	void upload();
	void saveOldFrame();
	Config* getConfig();
	void renderVideo();
	void renderFrames(int &nextRender,int timestamp);
	void renderRelocateNode(Node *n);
protected:
	double minX,maxX,minY,maxY;
private:
	Config *cfg;
	std::map<std::string,Node*> nodes;
	std::vector<Edge*> edges;
	std::vector<Node*> visibleNodes;
	std::vector<InferenceHeuristic*> inferences;
	std::set<std::string> ignoreNicks;
	unsigned int lastFrame,lastRender,lastUpload;
	GdiTools *gt;
	double maxWeight;
	bool isVideoRenderingGraph;
	Logger *logger;
	__int64 qpcTicksPerMs,qpcTickBeforeRender,qpcTickAfterRender;
	int vidRendFrame;
	int vidSecsPerFrame;
	GraphRendererQueue *grq;
};
