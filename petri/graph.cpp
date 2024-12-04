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

string iterator::to_string() const
{
	return (type == place::type ? "P" : "T") + ::to_string(index);
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

split_group::split_group() {
	split = 0;
	count = 0;
}

split_group::split_group(int split, int branch, int count) {
	this->split = split;
	this->branch.push_back(branch);
	this->count = count;
}

split_group::~split_group() {}

string split_group::to_string() const {
	return "(" + ::to_string(split) + "," + ::to_string(branch) + "/" + ::to_string(count) + ")";
}

bool operator<(const split_group &g0, const split_group &g1)
{
	return g0.split < g1.split;
}

bool operator==(const split_group &g0, const split_group &g1)
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

bool compare(int group_operation, int branch_operation, vector<split_group> g0, vector<split_group> g1) {
	// group_operation is one of:
	// split_group::INTERSECT
	// split_group::DIFFERENCE
	// split_group::SUBSET

	// branch_operation is one of:
	// split_group::INTERSECT
	// split_group::DIFFERENCE
	// split_group::SUBSET

	int branch_cmp = -1;
	int group_cmp = -1;

	for (int i = 0, j = 0; i < (int)g0.size() and j < (int)g1.size(); ) {
		if (g0[i].split == g1[j].split) {
			if (group_operation == split_group::INTERSECT) {
				bool found0 = false;
				bool found1 = false;
				bool found2 = false;
				int k = 0, l = 0;
				while (k < (int)g0[i].branch.size() and l < (int)g1[j].branch.size()) {
					if (g0[i].branch[k] == g1[j].branch[l]) {
						found2 = true;
						k++;
						l++;
					} else if (g0[i].branch[k] < g1[j].branch[l]) {
						found0 = true;
						k++;
					} else {
						found1 = true;
						l++;
					}
				}
				found0 = found0 or (k < (int)g0[i].branch.size());
				found1 = found1 or (l < (int)g1[j].branch.size());
				if ((branch_operation == split_group::DIFFERENCE and found0 and found1)
					or (branch_operation == split_group::INTERSECT and found2)) {
					return true;
				} else if (branch_operation == split_group::SUBSET and found0) {
					if (branch_cmp == 1) {
						return false;
					}
					branch_cmp = 0;
				} else if (branch_operation == split_group::SUBSET and found1) {
					if (branch_cmp == 0) {
						return false;
					}
					branch_cmp = 1;
				}
			}
			i++;
			j++;
		} else if (g0[i].split < g1[j].split) {
			if (group_operation == split_group::DIFFERENCE) {
				return true;
			} else if (group_operation == split_group::SUBSET) {
				if (group_cmp == 1) {
					return false;
				}
				group_cmp = 0;
			}
			i++;
		} else {
			if (group_operation == split_group::DIFFERENCE) {
				return true;
			} else if (group_operation == split_group::SUBSET) {
				if (group_cmp == 0) {
					return false;
				}
				group_cmp = 1;
			}
			j++;
		}
	}
	return ((group_operation == split_group::SUBSET
		or group_operation == split_group::INTERSECT)
			and branch_operation == split_group::SUBSET);
}

// What operations do I need to do?
//
// 1. Determine composition of partial states.
// 2. What are the choices that lead to any state in this partial? - group-unioned branch-intersected
// 3. what are the choices that lead to any state in any partial that overlaps this partial? - group-unioned branch-unioned
// 4. What are the choices that lead away from every state in this partial? not group-unioned branch-intersected
// 6. What are the choices that lead away from every state in this partial but lead to states in this other partial?

// Is there a data-structure difference between the always-/every- and the
// sometimes-/any- preconditioned questions? What kind of considerations do I
// need for operations between those two types of split groups?
vector<split_group> merge(int group_operation, int branch_operation, vector<split_group> g0, vector<split_group> g1) {
	// group_operation is one of:
	// split_group::INTERSECT
	// split_group::UNION

	// branch_operation is one of:
	// split_group::INTERSECT
	// split_group::UNION
	// split_group::DIFFERENCE
	
	vector<split_group> result;
	for (int i = 0, j = 0; i < (int)g0.size() and j < (int)g1.size(); ) {
		if (g0[i].split == g1[j].split) {
			result.push_back(split_group());
			result.back().split = g0[i].split;
			result.back().count = g0[i].count;

			int k = 0, l = 0;
			while (k < (int)g0[i].branch.size() and l < (int)g1[j].branch.size()) {
				if (g0[i].branch[k] == g1[j].branch[l]) {
					if (branch_operation != split_group::DIFFERENCE) {
						result.back().branch.push_back(g0[i].branch[k]);
					}
					k++;
					l++;
				} else if (g0[i].branch[k] < g1[j].branch[l]) {
					if (branch_operation != split_group::INTERSECT) {
						result.back().branch.push_back(g0[i].branch[k]);
					}
					k++;
				} else {
					if (branch_operation == split_group::UNION) {
						result.back().branch.push_back(g1[j].branch[l]);
					}
					l++;
				}
			}
			i++;
			j++;
		} else if (g0[i].split < g1[j].split) {
			if (group_operation != split_group::INTERSECT) {
				result.push_back(g0[i]);
			}
			i++;
		} else {
			if (group_operation == split_group::UNION) {
				result.push_back(g1[j]);
			}
			j++;
		}
	}
	return result;
}

}

