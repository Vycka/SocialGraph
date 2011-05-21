#pragma once
#include <string>
#include <time.h>

class Node;

class Edge
{
public:
	Edge(Node *source, Node *target, double weight, int secs = (int)time(NULL));
	~Edge(void);
	Node* getSource() { return source; };
	Node* getTarget() { return target; };
	double getWeight() { return weight; };
	bool sameNicks(const std::string *ln1,const std::string *ln2);
	int getActivityTime() { return lastActivity; };
	void setChangedInPause(bool state) { this->changedInPause = state; };
	bool getChangedInPause() { return changedInPause; };
	void setSource(Node *source) { this->source = source; };
	void setTarget(Node *target) { this->target = target; };
	void setWeight(double weight) { this->weight = weight; };
	void appWeight(double weight) { this->weight += weight; };
	void updateActivityTime(int secs = (int)time(NULL)) { lastActivity = secs; };

private:
	Node *source;
	Node *target;
	double weight;
	bool changedInPause;
	int lastActivity;
};
