#include "Logger.h"
#include "Node.h"
#include "Edge.h"
#include <time.h>

Logger::Logger(const char *f)
{
	log.open(f,std::ios_base::out | std::ios_base::app);
	int t = (int)time(NULL);
	log << t << ' ' << VID_INIT << std::endl; 
}

Logger::~Logger()
{
	log.close();
}

void Logger::wFrame(int frame)
{
	int t = (int)time(NULL);
	log << t << ' ' << VID_SETFRAME << ' ' << frame << std::endl; 
}

void Logger::wAddNode(Node *n, double weight)
{
	int t = (int)time(NULL);
	log << t << ' ' << VID_ADDNODE << ' ' << *n->getNick() << ' ' << weight << std::endl;
}

void Logger::wDelNode(Node *n)
{
	int t = (int)time(NULL);
	log << t << ' ' << VID_DELETENODE << ' ' << *n->getLNick() << std::endl; 
}

void Logger::wAddEdge(Edge *e,double weight)
{
	int t = (int)time(NULL);
	log << t << ' ' << VID_ADDEDGE << ' ' << *e->getSource()->getLNick() << ' '
		<< *e->getTarget()->getLNick() << ' ' << weight << ' '
		<< e->getActivityTime() << std::endl; 
}

void Logger::wEnd()
{
	int t = (int)time(NULL);
	log << t << ' ' << VID_CLEAR << std::endl; 
}

void Logger::wPause()
{
	int t = (int)time(NULL);
	log << t << ' ' << VID_PAUSE << std::endl; 
}

void Logger::wResume()
{
	int t = (int)time(NULL);
	log << t << ' ' << VID_RESUME << std::endl; 
}