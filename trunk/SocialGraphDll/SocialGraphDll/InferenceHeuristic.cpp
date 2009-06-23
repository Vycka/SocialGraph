#include "InferenceHeuristic.h"

InferenceHeuristic::InferenceHeuristic(Graph *graph, double weighting)
{
	this->graph = graph;
	this->weighting = weighting;
}

InferenceHeuristic::~InferenceHeuristic(void)
{
}
