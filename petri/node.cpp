#include "node.h"
#include <common/message.h>
#include <common/text.h>
#include <limits>

namespace petri
{

split_group::split_group() {
	split = std::numeric_limits<int>::min();
	count = 0;
}

split_group::split_group(int split, int count, vector<int> branch) {
	this->split = split;
	this->count = count;
	this->branch = branch;
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

bool operator<(const split_group &g0, int split) {
	return g0.split < split;
}

bool operator==(const split_group &g0, int split) {
	return g0.split == split;
}

ostream &operator<<(ostream &os, const split_group &g0) {
	os << g0.to_string();
	return os;
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

	int i = 0, j = 0;
	while (i < (int)g0.size() or j < (int)g1.size()) {
		if (i < (int)g0.size() and j < (int)g1.size() and g0[i].split == g1[j].split) {
			if (group_operation == split_group::INTERSECT
				or group_operation == split_group::DIFFERENCE
				or group_operation == split_group::NEGATIVE_DIFFERENCE) {
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
				if ((branch_operation == split_group::SYMMETRIC_DIFFERENCE and found0 and found1)
					or (branch_operation == split_group::INTERSECT and found2)) {
					return true;
				}

				if (branch_operation == split_group::DIFFERENCE and found0) {
					return true;
				}
				if (branch_operation == split_group::NEGATIVE_DIFFERENCE and found1) {
					return true;
				}
				if (branch_operation == split_group::NOT_EQUAL and (found0 or found1)) {
					return true;
				}

				if ((branch_operation == split_group::SUBSET
					or branch_operation == split_group::SUBSET_EQUAL) and found0) {
					if (branch_cmp == 1) {
						return false;
					}
					branch_cmp = 0;
				}

				if ((branch_operation == split_group::SUBSET
					or branch_operation == split_group::SUBSET_EQUAL) and found1) {
					if (branch_cmp == 0) {
						return false;
					}
					branch_cmp = 1;
				}
			}
			i++;
			j++;
		} else if (i < (int)g0.size() and (j >= (int)g1.size() or g0[i].split < g1[j].split)) {
			if ((int)g0[i].branch.size() < g0[i].count
				and (group_operation == split_group::DIFFERENCE
					or group_operation == split_group::SYMMETRIC_DIFFERENCE)) {
				return true;
			} else if (group_operation == split_group::SUBSET
				or group_operation == split_group::SUBSET_EQUAL) {
				if (group_cmp == 1) {
					return false;
				}
				group_cmp = 0;
			}
			i++;
		} else if (j < (int)g1.size()) {
			if ((int)g1[j].branch.size() < g1[j].count
				and (group_operation == split_group::NEGATIVE_DIFFERENCE
					or group_operation == split_group::SYMMETRIC_DIFFERENCE)) {
				return true;
			} else if (group_operation == split_group::SUBSET
				or group_operation == split_group::SUBSET_EQUAL) {
				if (group_cmp == 0) {
					return false;
				}
				group_cmp = 1;
			}
			j++;
		}
	}
	return ((group_operation == split_group::SUBSET_EQUAL
		or (group_operation == split_group::SUBSET and group_cmp != -1)
		or group_operation == split_group::INTERSECT)
			and (branch_operation == split_group::SUBSET_EQUAL
				or (branch_operation == split_group::SUBSET and branch_cmp != -1)));
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
	int i = 0, j = 0;
	while (i < (int)g0.size() or j < (int)g1.size()) {
		if (i < (int)g0.size() and j < (int)g1.size() and g0[i].split == g1[j].split) {
			result.push_back(split_group());
			result.back().split = g0[i].split;
			result.back().count = g0[i].count;

			int k = 0, l = 0;
			while (k < (int)g0[i].branch.size() or l < (int)g1[j].branch.size()) {
				if (k < (int)g0[i].branch.size() and l < (int)g1[j].branch.size() and g0[i].branch[k] == g1[j].branch[l]) {
					if (branch_operation != split_group::DIFFERENCE) {
						result.back().branch.push_back(g0[i].branch[k]);
					}
					k++;
					l++;
				} else if (k < (int)g0[i].branch.size() and (l >= (int)g1[j].branch.size() or g0[i].branch[k] < g1[j].branch[l])) {
					if (branch_operation != split_group::INTERSECT) {
						result.back().branch.push_back(g0[i].branch[k]);
					}
					k++;
				} else if (l < (int)g1[j].branch.size()) {
					if (branch_operation == split_group::UNION) {
						result.back().branch.push_back(g1[j].branch[l]);
					}
					l++;
				}
			}
			i++;
			j++;
		} else if (i < (int)g0.size() and (j >= (int)g1.size() or g0[i].split < g1[j].split)) {
			if (group_operation != split_group::INTERSECT) {
				result.push_back(g0[i]);
			}
			i++;
		} else if (j < (int)g1.size()) {
			if (group_operation == split_group::UNION) {
				result.push_back(g1[j]);
			}
			j++;
		}
	}
	return result;
}

void merge_inplace(int group_operation, int branch_operation, vector<split_group> &g0, const vector<split_group> &g1, set<int> exclude) {
	// group_operation is one of:
	// split_group::INTERSECT
	// split_group::UNION

	// branch_operation is one of:
	// split_group::INTERSECT
	// split_group::UNION
	// split_group::DIFFERENCE
	
	int i = 0, j = 0;
	while (i < (int)g0.size() or j < (int)g1.size()) {
		while (j < (int)g1.size() and exclude.find(g1[j].split) != exclude.end()) {
			j++;
		}

		if (i < (int)g0.size() and j < (int)g1.size() and g0[i].split == g1[j].split) {
			int k = 0, l = 0;
			while (k < (int)g0[i].branch.size() or l < (int)g1[j].branch.size()) {
				if (k < (int)g0[i].branch.size() and l < (int)g1[j].branch.size() and g0[i].branch[k] == g1[j].branch[l]) {
					k++;
					l++;
				} else if (k < (int)g0[i].branch.size() and (l >= (int)g1[j].branch.size() or g0[i].branch[k] < g1[j].branch[l])) {
					if (branch_operation == split_group::INTERSECT) {
						g0[i].branch.erase(g0[i].branch.begin()+k);
					} else {
						k++;
					}
				} else if (l < (int)g1[j].branch.size()) {
					if (branch_operation == split_group::UNION) {
						g0[i].branch.insert(g0[i].branch.begin()+k, g1[j].branch[l]);
						k++;
					}
					l++;
				}
			}
			i++;
			j++;
		} else if (i < (int)g0.size() and (j >= (int)g1.size() or g0[i].split < g1[j].split)) {
			if (group_operation == split_group::INTERSECT) {
				g0.erase(g0.begin()+i);
			} else {
				i++;
			}
		} else if (j < (int)g1.size()) {
			if (group_operation == split_group::UNION) {
				g0.insert(g0.begin()+i, g1[j]);
				i++;
			}
			j++;
		}
	}
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

