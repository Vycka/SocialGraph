#pragma once
#include "InferenceHeuristic.h"
#include <list>
#include <set>
#include <string>


class BinarySequenceInferenceHeuristic : public InferenceHeuristic
{
public:
	BinarySequenceInferenceHeuristic(Graph *graph, double weighting);
	~BinarySequenceInferenceHeuristic(void);
	std::string getName() { return "BinarySequenceInferenceHeuristic"; };
	void infer(const std::string *lnick,const std::string *lmsg);
private:
	std::list<std::string> nicks;
	std::set<std::string> uniqNicks;
};
