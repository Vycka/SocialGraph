#include "EdgeChangeListRecord.h"
#include "Edge.h"

EdgeChangeListRecord::EdgeChangeListRecord()
{
}

EdgeChangeListRecord::EdgeChangeListRecord(int timeBegin,const std::string &n1,const std::wstring &wn1, const std::string &n2,const std::wstring &wn2, Edge *e)
{
	set(timeBegin,timeBegin,n1,wn1,n2,wn2,e);
}

EdgeChangeListRecord::EdgeChangeListRecord(int timeBegin, int timeLast,const std::string &n1,const std::wstring &wn1, const std::string &n2,const std::wstring &wn2, Edge *e)
{
	set(timeBegin,timeLast,n1,wn1,n2,wn2,e);
}

void EdgeChangeListRecord::set(int timeBegin, int timeLast,const std::string &n1,const std::wstring &wn1, const std::string &n2,const std::wstring &wn2, Edge *e)
{
	tBegin = timeBegin;
	tLast = timeBegin;
	this->n1 = n1;
	this->n2 = n2;
	this->wn1 = wn1;
	this->wn2 = wn2;
	edge = e;
	if (edge)
		edge->setChangeListLink(this);
}

EdgeChangeListRecord::~EdgeChangeListRecord()
{
	if (edge)
		edge->setChangeListLink(NULL);
}


void EdgeChangeListRecord::setEdge(Edge *e)
{
	edge = e;
}

Edge* EdgeChangeListRecord::getEdge()
{
	return edge;
}