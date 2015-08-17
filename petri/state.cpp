/*
 * state.cpp
 *
 *  Created on: Jun 23, 2015
 *      Author: nbingham
 */

#include "state.h"

namespace petri
{

enabled_transition::enabled_transition()
{
	index = -1;
}

enabled_transition::enabled_transition(int index)
{
	this->index = index;
}

enabled_transition::~enabled_transition()
{

}

bool operator<(enabled_transition t0, enabled_transition t1)
{
	return t0.index < t1.index;
}

bool operator>(enabled_transition t0, enabled_transition t1)
{
	return t0.index > t1.index;
}

bool operator<=(enabled_transition t0, enabled_transition t1)
{
	return t0.index <= t1.index;
}

bool operator>=(enabled_transition t0, enabled_transition t1)
{
	return t0.index >= t1.index;
}

bool operator==(enabled_transition t0, enabled_transition t1)
{
	return t0.index == t1.index;
}

bool operator!=(enabled_transition t0, enabled_transition t1)
{
	return t0.index != t1.index;
}

token::token()
{
	this->index = 0;
}

token::token(int index)
{
	this->index = index;
}

token::~token()
{
}

void token::hash(hasher &hash) const
{
	hash.put(&index);
}

bool operator<(token i, token j)
{
	return i.index < j.index;
}

bool operator>(token i, token j)
{
	return i.index > j.index;
}

bool operator<=(token i, token j)
{
	return i.index < j.index;
}

bool operator>=(token i, token j)
{
	return i.index > j.index;
}

bool operator==(token i, token j)
{
	return i.index == j.index;
}

bool operator!=(token i, token j)
{
	return i.index != j.index;
}

}
