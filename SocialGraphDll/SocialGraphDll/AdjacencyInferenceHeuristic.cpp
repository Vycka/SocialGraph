#define _CRT_SECURE_NO_WARNINGS
#include "AdjacencyInferenceHeuristic.h"
#include "Graph.h"
#include "Edge.h"
#include "Tools.h"


//sulygina paskutinius dvieju eiluciu autorius, jei rase ne tie patys nickai, pridedam i sarasa
//algoritmas levas ir podefaultu originale jis net isjungtas (AdjacencyInferenceHeuristic = 0)
//algoritmas pakeistas kad updatintu tik jau egzistuojancius nodes

AdjacencyInferenceHeuristic::AdjacencyInferenceHeuristic(Graph *graph, double weighting) : InferenceHeuristic(graph,weighting)
{
}

AdjacencyInferenceHeuristic::~AdjacencyInferenceHeuristic(void)
{
}

void AdjacencyInferenceHeuristic::infer(const std::string *lnick,const std::string *lmsg)
{
	//std::cout << "AIH: " << lnick << " " << lastNick << std::endl;
	if (lastNick != *lnick)
	{
		Edge *e = getGraph()->findEdge(&lastNick,lnick);
		if (e)
		{
#ifdef HEURISTIC_NOTICES
			char buff[15];
			sprintf(buff,"%f",getWeighting());
			std::string mmsg = "/echo @SocialGraph SocialGraph: Heuristic-Adjancy: ";
			mmsg += *lnick;
			mmsg += " ";
			mmsg += lastNick;
			mmsg += " ";
			mmsg += buff;
			execInMirc(&mmsg);
#endif
			getGraph()->updateEdge(e,getWeighting());
		}
		lastNick = *lnick;
	}
}