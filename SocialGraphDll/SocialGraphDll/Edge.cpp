#include "Edge.h"
#include "Node.h"

Edge::Edge(Node *source, Node *target, double weight, int secs)
{
	this->source = source;
	this->target = target;
	this->weight = weight;
	this->lastActivity = secs;
}

Edge::~Edge(void)
{
}

bool Edge::sameNicks(const std::string *ln1, const std::string *ln2)
{
	return ((*ln1 == *source->getLNick() && *ln2 == *target->getLNick()) || (*ln1 == *target->getLNick() && *ln2 == *source->getLNick()));
}