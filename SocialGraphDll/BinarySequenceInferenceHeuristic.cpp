#define _CRT_SECURE_NO_WARNINGS
#include "BinarySequenceInferenceHeuristic.h"
#include "Graph.h"
#include "Tools.h"
#define MIN_SEQ_SIZE 7
//rimtesnis checkas, prisimena paskutiniu x eiluciu autorius, ir jei tose x eilutese kalbasi tik du pasnekovai
//tada uzskaitom (BinarySequenceInferenceHeuristic = 1)

BinarySequenceInferenceHeuristic::BinarySequenceInferenceHeuristic(Graph *graph, double weighting) : InferenceHeuristic(graph,weighting)
{
}

BinarySequenceInferenceHeuristic::~BinarySequenceInferenceHeuristic(void)
{
}

void BinarySequenceInferenceHeuristic::infer(const std::string *lnick,const std::string *lmsg)
{
	nicks.push_back(*lnick);
	if (nicks.size() > MIN_SEQ_SIZE)
	{
		nicks.pop_front();
		uniqNicks.clear();
		for (std::list<std::string>::iterator in = nicks.begin();in != nicks.end();in++)
			uniqNicks.insert(*in);
		if (uniqNicks.size() == 2)
		{
			std::set<std::string>::iterator iUniqNicks = uniqNicks.begin();
			std::string s1 = *iUniqNicks++;
			std::string s2 = *iUniqNicks;
#ifdef HEURISTIC_NOTICES
				this->sendInferenceChangeToMirc(*lnick,(s1 != *lnick ? s1 : s2),this->getName(),getWeighting());
#endif
			getGraph()->addEdge(lnick,(s1 != *lnick ? &s1 : &s2),getWeighting());
		}
	}
}