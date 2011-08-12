#include "Node.h"
#include "Tools.h"



Node::Node(const std::string *nick,const std::string *lNick,double weight, double px, double py)
{
	set(*nick,*lNick,weight,px,py);
}

Node::Node(const std::string *nick,double weight,double px, double py)
{
	std::string lNick;
	strToLower(*nick,lNick);
	set(*nick,lNick,weight,px,py);
}

void Node::set(const std::string &nick, const std::string &lNick, double weight, double posX,double posY)
{
	setNick(&nick,&lNick);
	this->weight = weight;
	x = posX;
	y = posY;
	fx = 0;
	fy = 0;
	cEdges = 0;
	userData = NULL;
}

Node::~Node()
{
	if (userData)
		delete userData;
}

void Node::setNick(const std::string* nick,const std::string *lnick)
{
	this->nick = *nick;
	this->lnick = *lnick;
	wnick.assign(this->nick.begin(),this->nick.end());
}