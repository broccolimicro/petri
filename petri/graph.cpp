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

iterator::iterator()
{
	type = -1;
	index = -1;
}

iterator::iterator(int type, int index)
{
	this->type = type;
	this->index = index;
}

iterator::~iterator()
{

}

iterator &iterator::operator=(iterator i)
{
	type = i.type;
	index = i.index;
	return *this;
}

iterator &iterator::operator--()
{
	index--;
	return *this;
}

iterator &iterator::operator++()
{
	index++;
	return *this;
}

iterator &iterator::operator--(int)
{
	index--;
	return *this;
}

iterator &iterator::operator++(int)
{
	index++;
	return *this;
}

iterator &iterator::operator+=(int i)
{
	index += i;
	return *this;
}

iterator &iterator::operator-=(int i)
{
	index -= i;
	return *this;
}

iterator iterator::operator+(int i)
{
	iterator result(*this);
	result.index += i;
	return result;
}

iterator iterator::operator-(int i)
{
	iterator result(*this);
	result.index -= i;
	return result;
}

bool iterator::operator==(iterator i) const
{
	return (type == i.type && index == i.index);
}

bool iterator::operator!=(iterator i) const
{
	return (type != i.type || index != i.index);
}

bool iterator::operator<(iterator i) const
{
	return (type < i.type ||
		   (type == i.type && index < i.index));
}

bool iterator::operator>(iterator i) const
{
	return (type > i.type ||
		   (type == i.type && index > i.index));
}

bool iterator::operator<=(iterator i) const
{
	return (type < i.type ||
		   (type == i.type && index <= i.index));
}

bool iterator::operator>=(iterator i) const
{
	return (type > i.type ||
		   (type == i.type && index >= i.index));
}

bool iterator::operator==(int i) const
{
	return index == i;
}

bool iterator::operator!=(int i) const
{
	return index != i;
}

bool iterator::operator<(int i) const
{
	return index < i;
}

bool iterator::operator>(int i) const
{
	return index > i;
}

bool iterator::operator<=(int i) const
{
	return index <= i;
}

bool iterator::operator>=(int i) const
{
	return index >= i;
}

ostream &operator<<(ostream &os, iterator i)
{
	if (i.type == place::type)
		os << "P" << i.index;
	else
		os << "T" << i.index;
	return os;
}

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

transition::transition()
{

}

transition::~transition()
{

}

transition merge(int composition, const transition &t0, const transition &t1)
{
	return transition();
}

bool mergeable(int composition, const transition &t0, const transition &t1)
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

parallel_group::parallel_group() {}

parallel_group::parallel_group(int split, int branch, int count)
{
	this->split = split;
	this->branch.push_back(branch);
	this->count = count;
}

parallel_group::~parallel_group() {}

bool operator<(const parallel_group &g0, const parallel_group &g1)
{
	return g0.split < g1.split;
}

bool operator==(const parallel_group &g0, const parallel_group &g1)
{
	if (g0.split != g1.split) {
		return false;
	}

	if (g0.branch.size() != g1.branch.size()) {
		return false;
	}

	for (int i = 0; i < (int)g0.branch.size(); i++) {
		if (g0.branch[i] != g1.branch[i]) {
			return false;
		}
	}
	return true;
}


}

