#pragma once
#include <string>
#include <time.h>
#include "EdgeChangeListRecord.h"

#define LPUSERDATA void*
#define USERDATA void

class Node;

class Edge
{
public:
	Edge(Node *source, Node *target, double weight, int secs = (int)time(NULL));
	~Edge(void);
	inline Node* getSource() { return source; };
	inline Node* getTarget() { return target; };
	inline double getWeight() { return weight; };
	bool sameNicks(const std::string *ln1,const std::string *ln2);
	inline int getActivityTime() { return lastActivity; };
	inline void setChangedInPause(bool state) { this->changedInPause = state; };
	inline bool getChangedInPause() { return changedInPause; };
	inline void setSource(Node *source) { this->source = source; };
	inline void setTarget(Node *target) { this->target = target; };
	inline void setWeight(double weight) { this->weight = weight; };
	inline void appWeight(double weight) { this->weight += weight; };
	//if source node doesn't match to a n, then it assumes that its target node
	void updateActivityTimeForNode(const Node *n, int time = (int)time(NULL));
	//if source nick doesn't match to a lnick, then it assumes that its target nick
	void updateActivityTimeForNick(const std::string &lnick, int time = (int)time(NULL));
	inline void updateActivityTimeForSource(int time = (int)time(NULL)) { this->sourceActivity = time; };
	inline void updateActivityTimeForTarget(int time = (int)time(NULL)) { this->targetActivity = time; };
	void updateActivityTime(int secs = (int)time(NULL));
	inline EdgeChangeListRecord* getChangeListLink() { return changeListLink; };
	inline void setChangeListLink(EdgeChangeListRecord *ecl) { changeListLink = ecl; };
	inline void setUserData(LPUSERDATA lpData) { userData = lpData; };
	inline LPUSERDATA getUserData() { return userData; };
private:
	Node *source;
	Node *target;
	double weight;
	bool changedInPause;
	EdgeChangeListRecord *changeListLink;
	int lastActivity;
	int sourceActivity,targetActivity;
	LPUSERDATA userData;
};
