#include "Logger.h"
#include "Node.h"
#include "Edge.h"
#include <sstream> //quick workaround for messing logs
#include <time.h>


Logger::Logger(const char *f)
{
	log.open(f,std::ios_base::out | std::ios_base::app);
	int t = (int)time(NULL);
	std::stringstream ss;
	ss << t << ' ' << VID_INIT << std::endl;
	log << ss.str(); 
}

Logger::~Logger()
{
	log.close();
}

void Logger::wFrame(int frame)
{
	int t = (int)time(NULL);
	std::stringstream ss;
	ss << t << ' ' << VID_SETFRAME << ' ' << frame << std::endl; 
	log << ss.str();
}

void Logger::wAddNode(Node *n, double weight)
{
	int t = (int)time(NULL);
	std::stringstream ss;
	ss << t << ' ' << VID_ADDNODE << ' ' << *n->getNick() << ' ' << weight << std::endl;
	log << ss.str();
}

void Logger::wDelNode(Node *n)
{
	
	int t = (int)time(NULL);
	std::stringstream ss;
	ss << t << ' ' << VID_DELETENODE << ' ' << *n->getLNick() << std::endl; 
	log << ss.str();
}

void Logger::wAddEdge(Edge *e,double weight)
{
	int t = (int)time(NULL);
	std::stringstream ss;
	ss << t << ' ' << VID_ADDEDGE << ' ' << *e->getSource()->getLNick() << ' '
		<< *e->getTarget()->getLNick() << ' ' << weight << ' '
		<< e->getActivityTime() << std::endl; 
	log << ss.str();
}

void Logger::wEnd()
{
	int t = (int)time(NULL);
	std::stringstream ss;
	ss << t << ' ' << VID_CLEAR << std::endl; 
	log << ss.str();
}

void Logger::wPause()
{
	int t = (int)time(NULL);
	std::stringstream ss;
	ss << t << ' ' << VID_PAUSE << std::endl; 
	log << ss.str();
}

void Logger::wResume()
{
	int t = (int)time(NULL);
	std::stringstream ss;
	ss << t << ' ' << VID_RESUME << std::endl; 
	log << ss.str();
}