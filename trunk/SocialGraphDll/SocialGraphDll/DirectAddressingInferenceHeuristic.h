#pragma once
#include "InferenceHeuristic.h"


class DirectAddressingInferenceHeuristic : public InferenceHeuristic
{
public:
	DirectAddressingInferenceHeuristic(Graph *graph, double weighting);
	~DirectAddressingInferenceHeuristic(void);
	std::string getName() { return "DirectAddressingInferenceHeuristic"; };
	void infer(const std::string *lnick,const std::string *lmsg);
};
