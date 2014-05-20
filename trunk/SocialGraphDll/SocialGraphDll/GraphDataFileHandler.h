#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include "Tools.h" //just for revision
#define FILE_VERSION 1

//V0 won't be used anymore, because it doesn't have any headers
//V1 EdgeChangeList support and headers!

//TODO: Don't give the graph object for the filehandler, but give the the data struct to populate it with data, and if reading was successful,
//Only then copy them to the graph in loadFromFile() function.
//This would also fix early clear() call before logger is initialized. And it would allow to for existing graph to continue if GraphData reading fails.

class Graph;

class GraphDataFileHandler
{
public:
	GraphDataFileHandler(Graph *graph);
	~GraphDataFileHandler();
	bool load(const char *fName);
	bool save(const char *fName);
	void generateHeader(int fVersion,int revNum = REVISION);
private:
	
	bool loadV2(std::stringstream &ss);

	bool loadV1(std::stringstream &ss);
	bool saveV1(std::ofstream &fr);
	bool loadV0(std::stringstream &ss);
	bool saveV0(std::ofstream &fr);
	std::string dataFileContent,generatedHeader;
	Graph *graph;
	int fileVersion,fileRevision;
	static const std::string cFileEnding;
};

