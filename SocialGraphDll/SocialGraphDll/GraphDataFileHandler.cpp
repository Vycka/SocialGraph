#define _CRT_SECURE_NO_WARNINGS
#include "GraphDataFileHandler.h"
#include <fstream>
#include "Graph.h"
#include "Edge.h"
#include "Logger.h"
#include "GraphConfig.h"

const std::string GraphDataFileHandler::cFileEnding = "SGEOF";

//universal header/footer
// SGF VV RR



GraphDataFileHandler::GraphDataFileHandler(Graph *graph)
{
	this->graph = graph;
	fileVersion = 0;
	fileRevision = 0;
	generateHeader(1);
}


GraphDataFileHandler::~GraphDataFileHandler()
{
}

void GraphDataFileHandler::generateHeader(int fVersion,int revNum)
{
	char cFVer[11],cRev[11];
	_itoa(fVersion,cFVer,10);
	_itoa(revNum,cRev,10);
	generatedHeader = "SGF" + std::string(" ") + std::string(cFVer) + std::string(" ") + cRev;
}

bool GraphDataFileHandler::save(const char *fName)
{
	std::ofstream fr(fName,std::ios_base::trunc);
	if (!fr.good())
		return false;
	fr << generatedHeader << std::endl;

	saveV1(fr);

	fr << std::endl << cFileEnding << std::endl;
	fr.close();
	return true;
}

bool GraphDataFileHandler::load(const char *fName)
{

	std::ifstream fs(fName);
	if (!fs.good())
		return false;
	dataFileContent = std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
	fs.close();
	if (dataFileContent.empty())
		return false;

	std::stringstream ss(dataFileContent);

	//Version check
	if (dataFileContent[0] == 'S') //NEW VERSION
	{
		
		ss.ignore(4);
		ss >> fileVersion;
		ss >> fileRevision;
	}
	else //Support for old version from alpha (without the header) (assumed file version V0)
	{
		dataFileContent.append("\n");
		dataFileContent.append(cFileEnding);
		dataFileContent.append("\n");
		ss = std::stringstream(dataFileContent);
	}

	int loadState = false; 

	switch (fileVersion)
	{
	case 2:
		loadState = loadV2(ss);
		break;
	case 1:
		loadState = loadV1(ss);
		break;
	case 0:
		loadState = loadV0(ss);
		break;
	default:
		return false;
	};
	std::string endCheck;
	ss >> endCheck;
	if (endCheck != this->cFileEnding)
		return false;
	return true;
}

//V2 Special version for stills generation from custom graph file
bool GraphDataFileHandler::loadV2(std::stringstream &ss)
{
	if (!loadV1(ss))
		return false;

	int specialMarkerCount;
	ss >> specialMarkerCount;

	for (int x = 0; x < specialMarkerCount; x++)
	{
		std::string nodeName,lNodeName;
		ss >> nodeName;
		
		strToLower(nodeName,lNodeName);

		Node *node = graph->findNode(&lNodeName);

		if (node != NULL)
			node->IsSecondaryColor = true;
	}

	return true;
}

//V1 Added EdgeChangeList saving and version flag.
bool GraphDataFileHandler::loadV1(std::stringstream &ss)
{
	if (!loadV0(ss))
		return false;

	int edgeChangeListCount;
	ss >> edgeChangeListCount;
	for (int x = 0; x < edgeChangeListCount; x++)
	{
		std::string nick1,nick2, lNick1, lNick2;
		std::wstring wNick1,wNick2;
		int tBegin,tLast;
		ss >> nick1 >> nick2 >> tBegin >> tLast;
		wNick1.assign(nick1.begin(),nick1.end());
		wNick2.assign(nick2.begin(),nick2.end());
		Edge *e = graph->findEdge(strToLower(nick1,lNick1),strToLower(nick2,lNick2));
		graph->addEdgeChangeList(tBegin,tLast,nick1,wNick1,nick2,wNick2,e);
	}

	return true;
}
bool GraphDataFileHandler::saveV1(std::ofstream &fr)
{
	if (!saveV0(fr))
		return false;

	fr << graph->edgeChangeList.size() << std::endl;
	for (std::list<EdgeChangeListRecord*>::iterator i = graph->edgeChangeList.begin(); i != graph->edgeChangeList.end(); i++)
	{
		EdgeChangeListRecord *r = *i;
		fr << r->getNickSource() << '\t' << r->getNickTarget() << '\t' << r->getTimeBegin() << '\t' << r->getTimeLast() << std::endl;
	}
	fr << std::endl;
	return true;
}

//V0 First format from alpha
bool GraphDataFileHandler::loadV0(std::stringstream &ss)
{
	int nodesCount;
	int edgesCount;
	int ignoresCount;
	int timeSaved;

	//lastframe and time saved
	ss >> graph->lastFrame >> timeSaved >> nodesCount;
	
	//nodes List
	for (int x = 0;x < nodesCount;x++)
	{
		std::string nick;
		double coordX,coordY,weight;
		ss >> nick >> coordX >> coordY >> weight;
		Node *node = new Node(&nick,weight,coordX,coordY);
		graph->nodes.insert(std::make_pair(node->getLNick(),node));
	}

	//Edges list
	ss >> edgesCount;
	int tPassed = ((int)time(NULL) - timeSaved);
	for (int x = 0;x < edgesCount;x++)
	{
		std::string n1,n2,ln1,ln2;
		double weight;
		int secs;
		ss >> n1 >> n2 >> weight >> secs;
		strToLower(n1,ln1);
		strToLower(n2,ln2);
		Node *node1 = graph->findNode(&ln1);
		Node *node2 = graph->findNode(&ln2);
		node1->appConEdges(1);
		node2->appConEdges(1);
		Edge *e = new Edge(node1,node2,weight,secs + tPassed);
		graph->edges.push_back(e);
	}

	//ignored nicks list
	ss >> ignoresCount;
	for (int x = 0;x < ignoresCount;x++)
	{
		std::string n1;
		ss >> n1;
		graph->ignoreNicks.insert(n1);
	}

	return true;
}
bool GraphDataFileHandler::saveV0(std::ofstream &fr)
{
	//last generated frame num, and time when file was saved
	fr << graph->lastFrame << ' ' << (int)time(NULL) <<  std::endl;

	//Nodes List
	fr << graph->nodes.size() << std::endl;
	for (std::map<std::string,Node*>::iterator i = graph->nodes.begin();i != graph->nodes.end();i++)
	{
		Node *n = i->second;
		fr << n->getNick() << "\t" << n->getX() << '\t' << n->getY() << '\t' << n->getWeight() << std::endl;
	}

	//Edges List
	fr << std::endl << graph->edges.size() << std::endl;
	for (std::vector<Edge*>::iterator i = graph->edges.begin(); i != graph->edges.end(); i++)
	{	
		Edge *e = *i;
		fr << e->getSource()->getNick() << "\t" << e->getTarget()->getNick() << "\t" << e->getWeight()
			<< "\t" << e->getActivityTime() << std::endl;
	}

	//ignored nicks list
	fr << std::endl << graph->ignoreNicks.size() << std::endl;
	for (std::set<std::string>::iterator i = graph->ignoreNicks.begin();i != graph->ignoreNicks.end();i++)
		fr << *i << std::endl;
	fr << std::endl;
	return true;
}