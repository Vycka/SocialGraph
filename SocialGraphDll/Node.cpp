#include "Node.h"
#include "Tools.h"



Node::Node(const std::string *nick,const std::string *lNick,double weight, double px, double py) : accelerationX(0), accelerationY(0)
{
	set(*nick,*lNick,weight,px,py);

	IsSecondaryColor = false;
}

Node::Node(const std::string *nick,double weight,double px, double py) : accelerationX(0), accelerationY(0)
{
	std::string lNick;
	strToLower(*nick,lNick);
	set(*nick,lNick,weight,px,py);

	IsSecondaryColor = false;
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

void Node::setInertedX(double x)
{
	double diff = accelerationX - x;
	double udiff = abs(diff);
	diff *= 1 - (pow(10, abs(udiff > 2 ? 2 : udiff)) / 100);
}
void Node::setInertedY(double y)
{

}