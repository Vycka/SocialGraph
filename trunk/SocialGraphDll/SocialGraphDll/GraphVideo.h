#pragma once
#include "graph.h"


class GraphVideo : public Graph
{
public:
	GraphVideo(Config *cfg);
	~GraphVideo();
	void addEdge(const std::string *ln1, const std::string *ln2, double weight,int activity);
	void renderVideo();
	void renderFrames(int &nextRender,int timestamp);
	void renderRelocateNode(Node *n);
	void drawImage(std::wstring *fWPath,int szClock);
	void makeImage(int iterations, std::wstring *output,int tNow = (int)time(NULL));
	void doLayout(int gSpringEmbedderIterations);
	void calcBounds();
	void deleteUnusedNodes();
private:
	int vidRendFrame;
	int vidSecsPerFrame;
	GraphRendererQueue *grq;
};
