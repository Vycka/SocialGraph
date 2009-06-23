#pragma once
#include "InferenceHeuristic.h"

class IndirectAddressingInferenceHeuristic : public InferenceHeuristic
{
public:
	IndirectAddressingInferenceHeuristic(Graph *graph, double weighting);
	~IndirectAddressingInferenceHeuristic(void);
	std::string getName() { return "IndirectAddressingInferenceHeuristic"; };
	void infer(const std::string *lnick, const std::string *lmsg);
};
