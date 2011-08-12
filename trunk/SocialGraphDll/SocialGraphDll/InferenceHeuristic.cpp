#include "InferenceHeuristic.h"
#include "Tools.h"
#include <sstream>

InferenceHeuristic::InferenceHeuristic(Graph *graph, double weighting)
{
	this->graph = graph;
	this->weighting = weighting;
}

InferenceHeuristic::~InferenceHeuristic(void)
{
}

void InferenceHeuristic::sendInferenceChangeToMirc(const std::string &srcNick, const std::string &targetNick, const std::string &name, const double &weight)
{
	std::stringstream ss;
	ss << "/.signal SocialGraph @sg sgInference: " << name << " // " << srcNick << " // " << targetNick << " // " << weight;
	execInMirc(ss.str().c_str());
}