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
	EdgeChangeListRecord(int time, std::wstring n1, std::wstring n2, Edge *e, CColor lastKnownColor);
	~EdgeChangeListRecord();
	inline void setTimeBegin(int tBegin) { this->tBegin = tBegin; };
	inline void setTimeLast(int tLast) { this->tLast = tLast; };
	inline int getTimeBegin() { return tBegin; };
	inline int getTimeLast() { return tLast; };
	void setEdge(Edge *e);
	Edge* getEdge();
	inline const std::wstring& getNickSource() { return n1; };
	inline const std::wstring& getNickTarget() { return n2; };
	//inline CColor getLastKnownColor() { return lastKnownColor; };
	//inline void setLastKnownColor(CColor color) { lastKnownColor = color; };
private:
	int tBegin,tLast;
	std::wstring n1, n2;
	Edge *edge;
	CColor lastKnownColor;
};

