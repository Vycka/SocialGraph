#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <fstream>
#include "Tools.h"

class Edge;

class Node
{
public:
	Node(const std::string *nick,const std::string *lnick,double weight = 0,double px = (rand32(2000) / 1000.0),double py = (rand32(2000) / 1000.0));
	Node(const std::string *nick,double weight = 0,double px = (rand32(2000) / 1000.0),double py = (rand32(2000) / 1000.0));
	Node(std::fstream *f);
	~Node(void);
	void setNick(const std::string* nick,const std::string *lnick);
	inline void setX(double x) { this->x = x; };
	inline void setY(double y) { this->y = y; };
	inline void setFX(double fx) { this->fx = fx; };
	inline void setFY(double fy) { this->fy = fy; };
	inline void setWeight(double weight) { this->weight = weight; };
	inline void appWeight(double weight) { this->weight += weight; };
	inline void appConEdges(int n) { this->cEdges += n; };
	//inline void setNodeVisible(bool isNodeStillVisible) { this->nodeVisible = isNodeStillVisible; };

	inline const std::string* getNick() { return &nick; };
	inline const std::string* getLNick() { return &lnick; };
	inline double getX() { return x; };
	inline double getY() { return y; };
	inline double getFX() { return fx; };
	inline double getFY() { return fy; };
	inline double getWeight() { return weight; };
	inline const wchar_t* getWNick() { return wnick.c_str(); };
	inline int getConEdges() { return cEdges; };
	//inline bool getNodeVisible() { return nodeVisible; };
	


	friend Edge;
private:
	//bool nodeVisible;
	std::string nick,lnick;
	std::wstring wnick;
	double weight,x,y,fx,fy;
	int cEdges;
};
