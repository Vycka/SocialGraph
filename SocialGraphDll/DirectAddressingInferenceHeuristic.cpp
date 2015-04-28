#define _CRT_SECURE_NO_WARNINGS
#include "DirectAddressingInferenceHeuristic.h"

#include "Tools.h"
#include "Graph.h"
#include <sstream>

//tiesioginis kreipinys, vienas is paprastesniu bet irgi neprastu, patikrina ar pirmas zodis yra kreipinys i kokinors
//jau egzistuojanti nicka, jei taip tai (DirectAddressingInferenceHeuristic = 1)

DirectAddressingInferenceHeuristic::DirectAddressingInferenceHeuristic(Graph *graph, double weighting) : InferenceHeuristic(graph,weighting)
{
}

DirectAddressingInferenceHeuristic::~DirectAddressingInferenceHeuristic(void)
{
}


void DirectAddressingInferenceHeuristic::infer(const std::string *lnick,const std::string *lmsg)
{
	std::string des;
	static std::string srcMask = "	 :.(),/&*!'?\"<>+~@%$#";
	static std::string desMask = "                      ";
	replaceChars(lmsg,&des,&srcMask,&desMask,1);
	std::stringstream ss(des);
	std::string tnick;
	ss >> tnick;
	if (getGraph()->findNode(&tnick))
	{
#ifdef HEURISTIC_NOTICES
				this->sendInferenceChangeToMirc(*lnick,tnick,this->getName(),getWeighting());
#endif
		getGraph()->addEdge(lnick,&tnick,getWeighting());
	}
}