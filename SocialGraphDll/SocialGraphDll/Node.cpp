#include "Node.h"
#include "Tools.h"



Node::Node(const std::string *nick,const std::string *lnick,double weight, double px, double py)
{
	setNick(nick,lnick);
	this->weight = weight;
	x = px;
	y = py;
	fx = 0;
	fy = 0;
}

Node::Node(const std::string *nick,double weight,double px, double py)
{
	std::string lnick;
	strToLower(nick,&lnick);
	Node(nick,&lnick,weight,px,py);
}
Node::Node(std::fstream *f)
{
	*f >> nick >> x >> y >> weight;
	fx = 0;
	fy = 0;

	strToLower(&nick,&lnick);
	wnick.assign(nick.begin(),nick.end());
}

Node::~Node(void)
{
}

void Node::setNick(const std::string* nick,const std::string *lnick)
{
	this->nick = *nick;
	this->lnick = *lnick;
	wnick.assign(this->nick.begin(),this->nick.end());
}