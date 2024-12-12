/*
 * graph.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: nbingham
 */

#include "graph.h"
#include <common/message.h>
#include <common/text.h>

namespace petri
{

arc::arc()
{

}

arc::arc(iterator from, iterator to)
{
	this->from = from;
	this->to = to;
}

arc::~arc()
{

}

bool operator<(arc a0, arc a1)
{
	return a0.from < a1.from || (a0.from == a1.from && a0.to < a1.to);
}
bool operator>(arc a0, arc a1)
{
	return a0.from > a1.from || (a0.from == a1.from && a0.to > a1.to);
}

bool operator<=(arc a0, arc a1)
{
	return a0.from < a1.from || (a0.from == a1.from && a0.to <= a1.to);
}

bool operator>=(arc a0, arc a1)
{
	return a0.from > a1.from || (a0.from == a1.from && a0.to >= a1.to);
}

bool operator==(arc a0, arc a1)
{
	return a0.from == a1.from && a0.to == a1.to;
}

bool operator!=(arc a0, arc a1)
{
	return a0.from != a1.from || a0.to != a1.to;
}

}

