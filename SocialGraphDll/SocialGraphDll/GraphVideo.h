#pragma once
#include "graph.h"

struct GraphRendererThreadSync;
struct GraphRendererQueue;

class GraphVideo : public Graph
{
public:
	GraphVideo(const GraphConfig &config);
	~GraphVideo();
	void addEdge(const std::string *ln1, const std::string *ln2, double weight,int activity);
	void renderVideo();
	void renderFrames(double &nextRender,int timestamp);
	void relocateNode(Node *n);
	void drawImage(int szClock);
	//does all calculations for next frame (updates layout, deltes faded out nodes/edges...) calls drawImage()
	void makeImage(int iterations,const std::wstring &outputPath,int tNow = (int)time(NULL));
	void doLayout(int gSpringEmbedderIterations, int tNow);
	void calcBounds();
	void deleteUnusedNodes();
	void decay(double d, int tNow);
	void deleteDisconnectedVisibleNode(Node *n);
	void mergeVisibleAndDisconnectedNodes();
	void clear();
	void deleteNode(const std::string *lnick);
	inline int getNodeFinalCoordX(Node *n) { return (int)((nodeCoordCalcWidth * (n->getX() - minX) / (maxX - minX)) + nodeCoordCalcBorderX); };
	inline int getNodeFinalCoordY(Node *n) { return (int)((nodeCoordCalcHeight * (n->getY() - minY) / (maxY - minY)) + nodeCoordCalcBorderY); };
	bool isNodeInFrame(Node *n);
	bool isNodeWithinBorder(Node *n);
	inline void setCancelRendering(bool val) { cancelRendering = val; };
private:
	//Copies current gt->g bitmap to multithreaded save queue.
	//Warning: If queue is full, function becomes blocking until there is enough space in queue
	void copyCurrentBitmapToSaveQueue(const std::wstring &fWPath);
	int vidRendFrame;
	std::vector<Node*> visibleDisconnectedNodes;
	double vidSecsPerFrame;
	GraphRendererQueue *grq;
	HANDLE *grh;
	GraphRendererThreadSync *grts;
	bool pauseRender;
	bool firstFrameRendered;
	bool cancelRendering;
	int nodeDeg;
	int nodeCoordCalcWidth, nodeCoordCalcHeight;
	int nodeCoordCalcBorderX, nodeCoordCalcBorderY;
	double xyAR;
	double xyDivX;
	double xyDivY;
	double xyDivX2;
	double xyDivY2;
	double xyDivX3;
	double xyDivY3;
	int vidBitmapPixelFormat;
	int fadeOutNodeParts,fadeOutEdgeParts;
	int *fadeSumTable;
};
