#pragma once
#include <string>
#include <sstream>
#include <fstream>
#define REVISION 58
#define FILE_VERSION 1

//V0 won't be used anymore, because it doesn't have any headers
//V1 EdgeChangeList support and headers!

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
	
	bool loadV1(std::stringstream &ss);
	bool saveV1(std::ofstream &fr);
	bool loadV0(std::stringstream &ss);
	bool saveV0(std::ofstream &fr);
	std::string dataFileContent,generatedHeader;
	Graph *graph;
	int fileVersion,fileRevision;
	static const std::string cFileEnding;
};

