#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include "Tools.h"
#include "Config.h"

#ifndef LPUSERDATA
#define LPUSERDATA void*
#endif
#ifndef USERDATA
#define USERDATA void
#endif

class Edge;

class Node
{
public:
	Node(const std::string *nick,const std::string *lNick,double weight = 0,double px = (rand32(2000) / 1000.0),double py = (rand32(2000) / 1000.0));
	Node(const std::string *nick,double weight = 0,double px = (rand32(2000) / 1000.0),double py = (rand32(2000) / 1000.0));
	~Node();
	void setNick(const std::string* nick,const std::string *lnick);
	inline void setX(double x) { this->x = x; };
	inline void setY(double y) { this->y = y; };
	inline void setFX(double fx) { this->fx = fx; };
	inline void setFY(double fy) { this->fy = fy; };
	void setInertedX(double x);
	void setInertedY(double y);
	inline void setWeight(double weight) { this->weight = weight; };
	inline void appWeight(double weight) { this->weight += weight; };
	//inline void setNodeVisible(bool isNodeStillVisible) { this->nodeVisible = isNodeStillVisible; };

	inline const std::string& getNick() { return nick; };
	inline const std::string& getLNick() { return lnick; };
	inline double getX() { return x; };
	inline double getY() { return y; };
	inline double getFX() { return fx; };
	inline double getFY() { return fy; };
	inline double getWeight() { return weight; };
	inline const wchar_t* getWNick() { return wnick.c_str(); };
	inline int getConEdges() { return cEdges; };
	inline void setUserData(LPUSERDATA lpData) { userData = lpData; };
	inline LPUSERDATA getUserData() { return userData; };
	inline bool hasAlternativeColor() { return _hasAlternativeColor; };
	inline void clearAlternativeColor() { _hasAlternativeColor = false; };
	inline void setAlternativeColor(CColor alternativeColor) { this->alternativeColor = alternativeColor; _hasAlternativeColor = true; };
	inline const CColor& getAlternativeColor() { return alternativeColor; };
	//inline bool getNodeVisible() { return nodeVisible; };
	
	friend Edge;
private:
	inline void appConEdges(int n) { this->cEdges += n; };
	void reset(const std::string &nick, const std::string &lNick, double weight, double posX,double posY);
	//bool nodeVisible;
	LPUSERDATA userData;
	std::string nick,lnick;
	std::wstring wnick;
	double weight,x,y,fx,fy,accelerationX,accelerationY;
	CColor alternativeColor;
	bool _hasAlternativeColor;
	int cEdges;

};
