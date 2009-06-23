#pragma once
#include "InferenceHeuristic.h"

class Graph;

class AdjacencyInferenceHeuristic : public InferenceHeuristic
{
public:
	AdjacencyInferenceHeuristic(Graph *graph, double weighting);
	~AdjacencyInferenceHeuristic(void);
	void infer(const std::string *nick, const std::string *msg);
	std::string getName() { return "AdjacencyInferenceHeuristic"; };
private:
	std::string lastNick;
};
