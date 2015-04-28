#pragma once

//TODO: rewrite format on how logger handles pauses/resumes to be more efficient and seeking to specific time  friendly for parsing.

#define LOG_WRITE_COUNT 30
#include <fstream>
#include <sstream>

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
	void wAddEdge(Edge *e, double weight,const Node *inputFrom);
	void wDelNode(Node *n);
	void wPause();
	void wResume();
	void qFlush(bool force = false);
	bool isLoggerWorking() { return isWorking; }
private:
	bool isWorking;
	std::fstream logFile;
	std::stringstream logQueue;
};
