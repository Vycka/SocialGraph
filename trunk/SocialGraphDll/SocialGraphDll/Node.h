#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <fstream>

class Edge;

class Node
{
public:
	Node(const std::string *nick,const std::string *lnick,double weight = 0,double px = ((rand() % 2000) / 1000.0),double py = ((rand() % 2000) / 1000.0));
	Node(const std::string *nick,double weight = 0,double px = ((rand() % 2000) / 1000.0),double py = ((rand() % 2000) / 1000.0));
	Node(std::fstream *f);
	~Node(void);
	void setNick(const std::string* nick,const std::string *lnick);
	void setX(double x) { this->x = x; };
	void setY(double y) { this->y = y; };
	void setFX(double fx) { this->fx = fx; };
	void setFY(double fy) { this->fy = fy; };
	void setWeight(double weight) { this->weight = weight; };
	void appWeight(double weight) { this->weight += weight; };

	const std::string* getNick() { return &nick; };
	const std::string* getLNick() { return &lnick; };
	double getX() { return x; };
	double getY() { return y; };
	double getFX() { return fx; };
	double getFY() { return fy; };
	double getWeight() { return weight; };
	const wchar_t* getWNick() { return wnick.c_str(); };

	friend Edge;
private:
	std::string nick,lnick;
	std::wstring wnick;
	double weight,x,y,fx,fy;
};
