#pragma once

#include <common/standard.h>
#include "graph.h"

namespace petri
{

// A path records a sequence of step from any node or region of the graph to
// any other node or region of the graph. This is used to understand the
// structure of the graph for state variable insertion.
struct path
{
	// num_places is the total number of places in the graph
	// num_transitions is the total number of transitions in the graph
	path(int num_places, int num_transitions);
	~path();

	// The start and end of the path
	vector<iterator> from, to;

	// This vector is resized to contain an integer for every place and
	// transition in the graph with places listed first, then transitions. Each
	// integer counts the number of times a particular path passes through that
	// place or transition. As a result, this structure can also be used to
	// accumulate multiple paths through the graph from one node or region to
	// another. 
	vector<int> hops;

	// The total number of places in the graph
	int num_places;
	// The total number of transitions in the graph
	int num_transitions;

	// Convertions between iterator and an index into path::hops.
	int idx(iterator i);
	iterator iter(int i);

	void clear();
	bool is_empty();

	vector<iterator> maxima();
	int max();
	int max(int i);
	int max(vector<int> i);

	path mask();
	path inverse_mask();

	void zero(iterator i);
	void zero(vector<iterator> i);
	void inc(iterator i, int v = 1);
	void dec(iterator i, int v = 1);

	path &operator=(path p);
	path &operator+=(path p);
	path &operator-=(path p);
	path &operator*=(path p);
	path &operator*=(int n);
	path &operator/=(int n);

	int &operator[](iterator i);
};

// A path set helps to manage multiple paths from one place or region to
// another to ensure the state variable insertion algorithm is able to cut them
// all with state transitions.
struct path_set
{
	path_set(int num_places, int num_transitions);
	~path_set();

	// The list of paths in this set.
	list<path> paths;

	// The accumulated total visit counts for all paths. This is updated when
	// paths are added to or removed from the set.
	path total;

	void merge(const path_set &s);
	void push(path p);
	void clear();
	void repair();

	list<path>::iterator erase(list<path>::iterator i);
	list<path>::iterator begin();
	list<path>::iterator end();

	void zero(iterator i);
	void zero(vector<iterator> i);
	void inc(iterator i, int v = 1);
	void dec(iterator i, int v = 1);
	void inc(list<path>::iterator i, iterator j, int v = 1);
	void dec(list<path>::iterator i, iterator j, int v = 1);

	path_set mask();
	path_set inverse_mask();

	path_set coverage(iterator i);
	path_set avoidance(iterator i);

	path_set &operator=(path_set p);
	path_set &operator+=(path_set p);
	path_set &operator*=(path p);
};

template <class place, class transition, class token, class state>
path_set trace(graph<place, transition, token, state> &g, vector<iterator> from, vector<iterator> to) {
	// precache "next" list for all nodes in graph to accelerate
	// computation.
	vector<vector<iterator> > n[2];
	for (int type = 0; type < 2; type++) {
		n[type].resize(g.size(type));

		// cut off sections of graph that don't lead to a node in the
		// "to" vector by ignoring those branches. The split groups
		// stored in the graph structure tell us which of those branches
		// lead to our target node.
		if (!g.split_groups_ready[type])
			g.compute_split_groups(type);
		
		vector<split_group> groups;
		for (auto i = to.begin(); i != to.end(); i++) {
			vector<split_group> group;
			if (i->type == place::type) {
				group = g.places[i->index].groups[type];
			} else {
				group = g.transitions[i->index].groups[type];
			}
			// only cut off a branch if it is cut off in all of the split groups
			if (i == to.begin()) {
				groups = group;
				continue;
			}

			int j = (int)group.size()-1;
			int k = (int)groups.size()-1;
			while (j >= 0 and k >= 0) {
				if (group[j].split == groups[k].split) {
					groups[k].branch.insert(groups[k].branch.end(), group[j].branch.begin(), group[j].branch.end());
					sort(groups[k].branch.begin(), groups[k].branch.end());
					groups[k].branch.erase(unique(groups[k].branch.begin(), groups[k].branch.end()), groups[k].branch.end());
					j--;
					k--;
				} else if (group[j].split > groups[k].split) {
					j--;
				} else {
					groups.erase(groups.begin() + k);
					k--;
				}
			}

			if (k >= 0) {
				groups.erase(groups.begin(), groups.begin() + k + 1);
			}
		}

		for (auto j = groups.begin(); j != groups.end(); j++) {
			for (auto k = j->branch.begin(); k != j->branch.end(); k++) {
				n[type][j->split].push_back(iterator(1-type, *k));
			}
		}

		// clean up and fill out the precached "next" lists for the
		// remaining nodes.
		for (auto i = g.begin(type); i != g.end(type); i++) {
			if (n[type][i.index].empty()) {
				n[type][i.index] = g.next(i);
			} else {
				sort(n[type][i.index].begin(), n[type][i.index].end());
				n[type][i.index].erase(unique(n[type][i.index].begin(), n[type][i.index].end()));
			}
		}
	}

	// This is an optimization. Make it easier to identify from/to nodes
	path fromCount;
	path toCount;
	for (auto i = from.begin(); i != from.end(); i++) {
		fromCount.inc(*i);
	}
	for (auto i = to.begin(); i != to.end(); i++) {
		toCount.inc(*i);
	}

	path_set result;
	// each item in the stack is a pair
	// The first element in the pair represents the frontier of the
	// trace. When the frontier is empty, we have completed the trace.
	// The second element in the pair is the path we've traced. Any
	// frontier node that encounters one of our targets gets placed
	// into the path's "to" list. If the trace completes and there are
	// no elements in the path's "to" list, then there was no path
	// found.
	vector<pair<vector<iterator>, path> > stack;
	// initialize the stack. To do this, we break the from list into conditional
	// groups of parallel nodes. Nodes that are in sequence with eachother should
	// be treated as conditional as well.
	vector<vector<iterator> > start;
	start.push_back(from);

	while (not start.empty()) {
		auto curr = start.back();
		start.pop_back();

		bool done = true;
		for (int i = (int)curr.size()-1; i >= 1; i--) {
			for (int j = i-1; j >= 0; j--) {
				if (not g.is(parallel, curr[i], curr[j])) {
					start.push_back(curr);
					start.back().erase(start.back().begin() + i);
					start.push_back(curr);
					start.back().erase(start.back().begin() + j);
					done = false;
				}
			}
		}

		if (done) {
			stack.push_back(pair<vector<iterator>, path>(curr, path(g.places.size(), g.transitions.size())));
			stack.back().second.from = curr;
		}
	}

	while (not stack.empty()) {
		auto curr = stack.back();
		stack.pop_back();

		iterator pos = curr.first.back();
		curr.first.pop_back();

		if (pos.type == transition::type) {
			for (auto i = n[pos.type][pos.index].begin(); i != n[pos.type][pos.index].end(); i++) {
				if (toCount[*i] > 0) {
					curr.second.to.push_back(*i);
				} else if (curr.second[*i] == 0 and fromCount[*i] == 0) {
					curr.second.inc(*i);
					curr.first.push_back(*i);
				}
			}
			if (curr.first.empty()) {
				if (not curr.second.to.empty()) {
					result.push(curr.second);
				}
			} else {
				stack.push_back(curr);
			}
		}	else {
			for (auto i = n[pos.type][pos.index].begin(); i != n[pos.type][pos.index].end(); i++) {
				auto copy = curr;
				if (toCount[*i] > 0) {
					copy.second.to.push_back(*i);
				} else if (curr.second[*i] == 0 and fromCount[*i] == 0) {
					copy.second.inc(*i);
					copy.first.push_back(*i);
				}
				if (copy.first.empty()) {
					if (not copy.second.to.empty()) {
						result.push(copy.second);
					}
				} else {
					stack.push_back(copy);
				}
			}
		}
	}

	return result;
}

}

ostream &operator<<(ostream &os, petri::path p);

petri::path operator+(petri::path p0, petri::path p1);
petri::path operator-(petri::path p0, petri::path p1);
petri::path operator*(petri::path p0, petri::path p1);
petri::path operator/(petri::path p1, int n);
petri::path operator*(petri::path p1, int n);

ostream &operator<<(ostream &os, petri::path_set p);

petri::path_set operator+(petri::path_set p0, petri::path_set p1);
petri::path_set operator*(petri::path_set p0, petri::path p1);
petri::path_set operator*(petri::path p0, petri::path_set p1);

