//#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <gdiplus.h>
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <time.h>
#include <stdio.h>

#include "Tools.h"
#include "Graph.h"
#include "GraphVideo.h"
#include "GraphConfig.h"

#define ALIAS int __stdcall
#define ALP HWND mWnd, HWND aWnd, char *data, char *parms, BOOL show, BOOL nopause
int mVer;
HWND mHwnd;
//multichannel supportui
std::map<std::string,Graph*> graphs;
GraphVideo *gRender = NULL;

Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;


typedef struct {
   DWORD  mVersion;
   HWND   mHwnd;
   BOOL   mKeep;
} LOADINFO;

void __stdcall LoadDll(LOADINFO* li)
{
	li->mKeep = true;
	mVer = li->mVersion;
	mHwnd = li->mHwnd;
	srand32((unsigned int)time((time_t*)0));
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	execInMircInit(&mHwnd);
	std::stringstream ssEim;
	ssEim << "[INFO] SocialGraphDLL v1.04 R" << REVISION << " BETA Loaded!";
	printToSGWindow(ssEim.str());
}

int __stdcall UnloadDll(int type)
{
	if (type == 1)
		return 0; //NE autounloadui
	
	//multichannel
	for (std::map<std::string,Graph*>::iterator i = graphs.begin();i != graphs.end();i++)
	{
		Graph *g = i->second;
		g->makeImage();
		delete g;
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	printToSGWindow("[INFO] SocialGraphDll Shutting down!");
	execInMircShutdown();
	return 1;
}

ALIAS mInitGraph(ALP)
{
	GraphConfig c(data);
	if (c.isBadConfig())
	{
		printToSGWindow("[ERROR] Init: Graph not initialized due bad config: " + std::string(data));
		return 1;
	}

	std::string lchan;
	strToLower(c.nChannel,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		return 1;

	std::string configFile(data);
	configFile.erase(0,configFile.rfind("\\",configFile.size())+1);
	printToSGWindow("[INFO] Init: " + configFile + " Loaded for channel: " + c.nChannel);


	Graph *graph = new Graph(c);
	graphs.insert(std::make_pair(lchan,graph));
	return 1;
}

ALIAS mUnloadGraph(ALP)
{
	std::string chan(data),lchan;
	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
	{
		delete graphIter->second;
		graphs.erase(graphIter);
	}
	return 1;
}

ALIAS mAddMessage(ALP)
{
	std::string nick,chan,lchan,msg;
	std::stringstream ss(data);
	getline(ss,chan,' ');
	getline(ss,nick,' ');
	getline(ss,msg,'\0');

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		graphIter->second->onMessage(&nick,&msg);

	return 1;
}

ALIAS mAddJoin(ALP)
{
	std::string nick,chan,lchan;
	std::stringstream ss(data);
	ss >> chan >> nick;

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		graphIter->second->onJoin(&nick);
	return 1;
}

ALIAS mMakeImage(ALP)
{
	std::string chan(data),lchan;

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		graphIter->second->makeImage();
	return 1;
}

ALIAS mUpload(ALP)
{
	std::string chan(data),lchan;

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		graphIter->second->upload();
	return 1;
}

ALIAS mAddIgnore(ALP)
{
	std::string nick,lnick,chan,lchan;
	std::stringstream ss(data);
	ss >> chan >> nick;

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
	{
		strToLower(nick,lnick);
		graphIter->second->addIgnore(&lnick);
	}
	return 1;
}

ALIAS mDeleteIgnore(ALP)
{
	std::string nick,lnick,chan,lchan;
	std::stringstream ss(data);
	ss >> chan >> nick;

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
	{
		strToLower(nick,lnick);
		graphIter->second->deleteIgnore(&lnick);
	}
	return 1;
}

ALIAS mDeleteNode(ALP)
{
	std::string nick,lnick,chan,lchan;
	std::stringstream ss(data);
	ss >> chan >> nick;

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
	{
		strToLower(nick,lnick);
		graphIter->second->deleteNode(&lnick);
	}
	return 1;
}

ALIAS mDeleteUnusedNodes(ALP)
{
	std::string chan(data),lchan;

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		graphIter->second->deleteUnusedNodes();
	return 1;
}

//mSaveOld was created mainly for debug purposes only
//It forces to copy current graph image as the old frame for testing, if output directory is correct and it works
ALIAS mSaveOld(ALP)
{
	std::string chan(data),lchan;

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		graphIter->second->saveOldFrame();
	return 1;
}

ALIAS mDumpFile(ALP)
{
	std::string chan,lchan,file;
	std::stringstream ss(data);
	getline(ss,chan,' ');
	getline(ss,file,'\0');

	strToLower(chan,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		graphIter->second->saveToFile(file.c_str());
	return 1;
}

ALIAS mRenderVideo(ALP)
{
	GraphConfig c(data);
	if (c.isBadConfig())
		printToSGWindow("[ERROR] GraphVideo: Unable to render video due bad config: " + std::string(data));
	else if (gRender)
		printToSGWindow("[ERROR] GraphVideo: Unable to render video because another video rendering is in progress");
	else
	{
		gRender = new GraphVideo(c);
		gRender->renderVideo();
		delete gRender;
		gRender = NULL;
	}
	return 1;
}

ALIAS mCancelVideoRendering(ALP)
{
	if (!gRender)
		printToSGWindow("[ERROR] GraphVideo: Video renderer is not running!");
	else
	{
		printToSGWindow("[INFO] GraphVideo: Cancel rendering signal sent!");
		gRender->setCancelRendering(true);
	}
	return 1;
}

ALIAS mIsVideoRenderingInProgress(ALP)
{
	strcpy(data,(gRender ? "1" : "0"));
	return 3;
}

ALIAS mReloadConfig(ALP)
{
	GraphConfig c(data);
	if (c.isBadConfig())
	{
		printToSGWindow("[ERROR] Init: Graph not initialized due bad config: " + std::string(data));
		return 1;
	}

	std::string lchan;
	strToLower(c.nChannel,lchan);
	std::map<std::string,Graph*>::iterator graphIter = graphs.find(lchan);
	if (graphIter != graphs.end())
		graphIter->second->reloadConfig(c);
	else
		printToSGWindow("[ERROR] Init: Config not reloaded because channel does not exist: " + c.nChannel + ". Use mInitGraph to load a new channel.");
	return 1;
}


void main(int argc, char** arg)
{
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	GraphConfig cfg("Config.txt");
	int tNow = (int)time(0);

	if (argc <= 2)
	{
		std::cout << "Missing config file.. will try to read config.txt then...\n";	
		system("pause");
	}
	else
		cfg = GraphConfig(arg[1]);

	if (cfg.isBadConfig())
	{
		std::cout << "Bad Config file\n";	
		system("pause");
		return;
	}

	if (argc > 2)
	{
		tNow = atoi(arg[2]);
	}

	Graph *graph = new Graph(cfg);
	graph->randomizeNodePositions();
	graph->makeImage(tNow);
	delete graph;
	Gdiplus::GdiplusShutdown(gdiplusToken);

	//Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	//CColor test(255,255,255,255);
	//CColor test2 = CColor(test) * 0.5;
	//GraphConfig c("C:\\Users\\Viki\\Documents\\ADV_Seeker1\\SocialGraph\\Configs\\debug\\#linkomanija.config.txt");
	////Graph *g = new Graph(c);
	////g->printLists();
	////g->saveToFileEx("i:\\test.txt");
	//GraphVideo *g = new GraphVideo(c);
	//g->renderVideo();
	//delete g;
	//system("pause");
	//Gdiplus::GdiplusShutdown(gdiplusToken);
}