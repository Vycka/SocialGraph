#include "Edge.h"
#include "Node.h"
#include "Tools.h"


Edge::Edge(Node *source, Node *target, double weight, int secs)
{
	this->source = source;
	this->target = target;
	this->weight = weight;
	this->lastActivity = secs;
	changedInPause = false;
	this->userData = NULL;
	changeListLink = NULL;
}

Edge::~Edge(void)
{
	if (userData)
		delete userData;
	if (changeListLink)
		changeListLink->setEdge(NULL);
}

bool Edge::sameNicks(const std::string &ln1, const std::string &ln2)
{
	return ((ln1 == source->getLNick() && ln2 == target->getLNick()) || (ln1 == target->getLNick() && ln2 == source->getLNick()));
}

//if source node doesn't match to a n, then it assumes that its target node
void Edge::updateActivityTimeForNode(const Node *n, int time)
{
	if (n == source)
		updateActivityTimeForSource(time);
	else if (n == target)
		updateActivityTimeForTarget(time);
	else //extra if's are for debug. in theory they should not be needed, and if last statment wont be triggered, it will be removed, if it will be triggered, then hunting for logical bugs begins..
		printToSGWindow("[WARNING] Edge: Update Activity for nick - can't find specified node!");
}
//if source nick doesn't match to a lnick, then it assumes that its target nick
void Edge::updateActivityTimeForNick(const std::string &lnick, int time)
{
	if (lnick == source->getLNick())
		updateActivityTimeForSource(time);
	else if (lnick == target->getLNick())
		updateActivityTimeForTarget(time);
	else //extra if's are for debug. in theory they should not be needed, and if last statment wont be triggered, it will be removed, if it will be triggered, then hunting for logical bugs begins..
		printToSGWindow("[WARNING] Edge: Update Activity for nick - can't find specified nick! Nick: " + lnick);
}

void Edge::updateActivityTime(int secs)
{
	lastActivity = secs;
	if (changeListLink)
		changeListLink->setTimeLast(secs);
}