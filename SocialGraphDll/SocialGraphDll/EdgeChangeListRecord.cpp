#include "EdgeChangeListRecord.h"
#include "Edge.h"

EdgeChangeListRecord::EdgeChangeListRecord()
{
}

EdgeChangeListRecord::EdgeChangeListRecord(int time, std::wstring n1, std::wstring n2, Edge *e)
{
	tBegin = time;
	tLast = time;
	this->n1 = n1;
	this->n2 = n2;
	edge = e;
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