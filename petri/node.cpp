#include "node.h"
#include <common/message.h>
#include <common/text.h>
#include <limits>

namespace petri {

place::place()
{

}

place::~place()
{

}

place place::merge(int composition, const place &p0, const place &p1)
{
	return place();
}

ostream &operator<<(ostream &os, const place &p) {
	return os;
}

transition::transition()
{

}

transition::~transition()
{

}

transition transition::merge(int composition, const transition &t0, const transition &t1)
{
	return transition();
}

bool transition::mergeable(int composition, const transition &t0, const transition &t1)
{
	return true;
}

bool transition::is_infeasible()
{
	return false;
}

bool transition::is_vacuous()
{
	return false;
}

ostream &operator<<(ostream &os, const transition &t) {
	return os;
}

}

