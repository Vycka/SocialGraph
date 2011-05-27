#define LOG_FLUSH_INTERVAL 60
#include "Logger.h"
#include "Node.h"
#include "Edge.h"
#include <time.h>


Logger::Logger(const char *f)
{
	logFile.open(f,std::ios_base::out | std::ios_base::app);
	int t = (int)time(NULL);
	logQueue << t << ' ' << VID_INIT << std::endl;
	qFlush();
}

Logger::~Logger()
{
	qFlush(true);
	logFile.close();
}

void Logger::wFrame(int frame)
{
	int t = (int)time(NULL);
	logQueue << t << ' ' << VID_SETFRAME << ' ' << frame << std::endl; 
	qFlush();
}

void Logger::wAddNode(Node *n, double weight)
{
	int t = (int)time(NULL);
	logQueue << t << ' ' << VID_ADDNODE << ' ' << *n->getNick() << ' ' << weight << std::endl;
	qFlush();
}

void Logger::wDelNode(Node *n)
{
	
	int t = (int)time(NULL);
	logQueue << t << ' ' << VID_DELETENODE << ' ' << *n->getLNick() << std::endl; 
	qFlush();
}

void Logger::wAddEdge(Edge *e,double weight,Node *inputFrom)
{
	int t = (int)time(NULL);
	if (e->getSource() == inputFrom)
	{
		logQueue << t << ' ' << VID_ADDEDGE << ' ' << *e->getSource()->getLNick() << ' '
			<< *e->getTarget()->getLNick() << ' ' << weight << ' '
			<< e->getActivityTime() << std::endl; 
	}
	else
	{
		logQueue << t << ' ' << VID_ADDEDGE << ' ' << *e->getTarget()->getLNick() << ' '
			<< *e->getSource()->getLNick() << ' ' << weight << ' '
			<< e->getActivityTime() << std::endl; 
	}
	qFlush();
}

void Logger::wEnd()
{
	int t = (int)time(NULL);
	logQueue << t << ' ' << VID_CLEAR << std::endl; 
	qFlush();
}

void Logger::wPause()
{
	int t = (int)time(NULL);
	logQueue << t << ' ' << VID_PAUSE << std::endl; 
	qFlush();
}

void Logger::wResume()
{
	int t = (int)time(NULL);
	logQueue << t << ' ' << VID_RESUME << std::endl; 
	qFlush();
}

void Logger::qFlush(bool force)
{
	static time_t tLast = 0;
	time_t tNow = time(NULL);
	if (force || (tNow - tLast) >= LOG_FLUSH_INTERVAL)
	{
		tLast = tNow;
		logFile << logQueue.str();
		logQueue.str("");
		logFile.flush();
	}
}