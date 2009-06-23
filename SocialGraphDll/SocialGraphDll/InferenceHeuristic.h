#pragma once

#include <string>
class Graph;

class InferenceHeuristic
{
public:
	InferenceHeuristic(Graph *graph, double weighting);
	~InferenceHeuristic(void);
	double getWeighting() { return weighting; };
	Graph *getGraph() { return graph; };
	virtual void infer(const std::string *lnick, const std::string *lmsg) { return; };
	virtual std::string getName() { return "InferenceHeuristic"; };
private:
	double weighting;
	Graph *graph;
};
