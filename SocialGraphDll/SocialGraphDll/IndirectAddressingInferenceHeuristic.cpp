#define _CRT_SECURE_NO_WARNINGS
#include "IndirectAddressingInferenceHeuristic.h"
#include "Tools.h"
#include "Graph.h"
#include <sstream>

//analizuoja iskaido parasyta teksta po zodi ir kiekviena zodi lygina esamu nicku sarase
//jei toki nicka randa tai padaro sioki toki edge (IndirectAddressingInferenceHeuristic = 0.3)

IndirectAddressingInferenceHeuristic::IndirectAddressingInferenceHeuristic(Graph *graph, double weighting) : InferenceHeuristic(graph,weighting)
{
}

IndirectAddressingInferenceHeuristic::~IndirectAddressingInferenceHeuristic(void)
{
}

void IndirectAddressingInferenceHeuristic::infer(const std::string *lnick, const std::string *lmsg)
{
	std::string des;
	static std::string srcMask = "	 :.(),/&*!'?\"<>+~@%$#";
	static std::string desMask = "                      ";
	replaceChars(lmsg,&des,&srcMask,&desMask);
	std::stringstream ss(des);
	std::string tnick;
	ss >> tnick;
	while (!ss.fail())
	{
		if (getGraph()->findNode(&tnick))
		{
#ifdef HEURISTIC_NOTICES
				this->sendInferenceChangeToMirc(*lnick,tnick,this->getName(),getWeighting());
#endif
			getGraph()->addEdge(lnick,&tnick,getWeighting());
		}
		
		ss >> tnick;
	}
}