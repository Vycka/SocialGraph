#pragma once
#include <string>
#include "Config.h"


/*
struct GraphEdgeChangeList
{
	GraphEdgeChangeList() : edge(NULL) {};
	GraphEdgeChangeList(int tNow, std::wstring n1, std::wstring n2, bool isAppearing) 
		: tNow(tNow), n1(n1), n2(n2), isAppearing(isAppearing), tEnd(0), edge(NULL) {};
	~GraphEdgeChangeList()
	{
		if (edge)
			edge->setChangeListLink(NULL);
	}
	int tNow,tEnd;
	std::wstring n1, n2;
	bool isAppearing;
	Edge *edge;
};
*/
class Edge;

class EdgeChangeListRecord
{
public:
	EdgeChangeListRecord();
	EdgeChangeListRecord(int timeBegin,const std::string &n1,const std::wstring &wn1, const std::string &n2,const std::wstring &wn2, Edge *e);
	EdgeChangeListRecord(int timeBegin, int timeLast,const std::string &n1,const std::wstring &wn1, const std::string &n2,const std::wstring &wn2, Edge *e);
	~EdgeChangeListRecord();
	inline void setTimeBegin(int tBegin) { this->tBegin = tBegin; };
	inline void setTimeLast(int tLast) { this->tLast = tLast; };
	inline int getTimeBegin() { return tBegin; };
	inline int getTimeLast() { return tLast; };
	void setEdge(Edge *e);
	Edge* getEdge();
	inline const std::wstring& getWNickSource() { return wn1; };
	inline const std::wstring& getWNickTarget() { return wn2; };
	inline const std::string& getNickSource() { return n1; };
	inline const std::string& getNickTarget() { return n2; };

private:
	void set(int timeBegin, int timeLast,const std::string &n1,const std::wstring &wn1, const std::string &n2,const std::wstring &wn2, Edge *e);
	int tBegin,tLast;
	std::wstring wn1, wn2;
	std::string n1,n2;
	Edge *edge;
};

