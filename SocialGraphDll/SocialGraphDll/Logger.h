#pragma once
#define LOG_WRITE_COUNT 30
#include <fstream>
//#include <queue>
//#include <string>

class Edge;
class Node;

enum GRAPH_VIDMARKERS
{
	VID_INIT = 0,
	VID_SETFRAME = 1,
	VID_ADDNODE = 2,
	VID_ADDEDGE = 3,
	VID_DELETENODE = 4,
	VID_CLEAR = 5,
	VID_PAUSE = 6,
	VID_RESUME = 7
};

class Logger
{
public:
	Logger(const char *f);
	~Logger();
	void wFrame(int frame);
	void wEnd();
	void wAddNode(Node *n, double weight);
	void wAddEdge(Edge *e, double weight);
	void wDelNode(Node *n);
	void wPause();
	void wResume();
	//void qFlush(bool force = false); //TODO: do it in the future
	//void qAdd(std::string *s);
private:
	std::fstream log;
	//std::queue<std::string> logQ;
};
