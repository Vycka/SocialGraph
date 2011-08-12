#define _CRT_SECURE_NO_WARNINGS
#include "AdjacencyInferenceHeuristic.h"
#include "Graph.h"
#include "Edge.h"
#include "Tools.h"
#include "Node.h"


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
		Edge *e = getGraph()->findEdge(lastNick,*lnick);
		if (e)
		{
#ifdef HEURISTIC_NOTICES
			if (*lnick == *e->getSource()->getLNick())
				this->sendInferenceChangeToMirc(*e->getSource()->getLNick(),*e->getTarget()->getLNick(),this->getName(),getWeighting());
			else
				this->sendInferenceChangeToMirc(*e->getTarget()->getLNick(),*e->getSource()->getLNick(),this->getName(),getWeighting());
#endif
			if (*lnick == e->getSource()->getLNick())
				getGraph()->updateEdge(e,getWeighting(),e->getSource());
			else
				getGraph()->updateEdge(e,getWeighting(),e->getTarget());
		}
		lastNick = *lnick;
	}
}