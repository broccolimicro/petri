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
	vector<petri::iterator> from, to;

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

	// Convertions between petri::iterator and an index into path::hops.
	int idx(petri::iterator i) const;
	petri::iterator iter(int i) const;

	void clear();
	bool is_empty();

	vector<petri::iterator> maxima();
	int max();
	int max(int i);
	int max(vector<int> i);

	path mask();
	path inverse_mask();

	void zero(petri::iterator i);
	void zero(vector<petri::iterator> i);
	void inc(petri::iterator i, int v = 1);
	void dec(petri::iterator i, int v = 1);
	void set(petri::iterator i, int v = 1);

	path &operator=(path p);
	path &operator+=(path p);
	path &operator-=(path p);
	path &operator*=(path p);
	path &operator&=(path p);
	path &operator*=(int n);
	path &operator/=(int n);

	int &operator[](petri::iterator i);
	int operator[](petri::iterator i) const;
};

template <class place, class transition, class token, class state>
bool normalize(graph<place, transition, token, state> &g, path &p0, path &p1) {
	bool p0_empty = true;
	bool p1_empty = true;
	for (int i = 0; i < (int)p0.hops.size() and i < (int)p1.hops.size(); i++) {
		if (p0.hops[i] > p1.hops[i]) {
			bool found = false;
			for (int j = 0; j < (int)p1.hops.size() and not found; j++) {
				found = (p1.hops[j] > 0 and g.is(choice, p0.iter(i), p1.iter(j)));
			}

			if (not found) {
				p0.hops[i] = p1.hops[i];
			}
		} else if (p0.hops[i] < p1.hops[i]) {
			bool found = false;
			for (int j = 0; j < (int)p0.hops.size() and not found; j++) {
				found = (p0.hops[j] > 0 and g.is(choice, p0.iter(j), p1.iter(i)));
			}

			if (not found) {
				p1.hops[i] = p0.hops[i];
			}
		}
		p0_empty = p0_empty and p0.hops[i] == 0;
		p1_empty = p1_empty and p1.hops[i] == 0;
	}

	return not p0_empty and not p1_empty;
}

template <class place, class transition, class token, class state>
bool mergible(graph<place, transition, token, state> &g, const path &p0, const path &p1) {
	bool p0_empty = true;
	bool p1_empty = true;
	for (int i = 0; i < (int)p0.hops.size() and i < (int)p1.hops.size(); i++) {
		if (p0.hops[i] > p1.hops[i]) {
			bool found = false;
			for (int j = 0; j < (int)p1.hops.size() and not found; j++) {
				found = (p1.hops[j] > 0 and g.is(choice, p0.iter(i), p1.iter(j)));
			}

			p0_empty = p0_empty and (found ? p0.hops[i] : p1.hops[i]) == 0;
			p1_empty = p1_empty and p1.hops[i] == 0;
		} else if (p0.hops[i] < p1.hops[i]) {
			bool found = false;
			for (int j = 0; j < (int)p0.hops.size() and not found; j++) {
				found = (p0.hops[j] > 0 and g.is(choice, p0.iter(j), p1.iter(i)));
			}

			p0_empty = p0_empty and p0.hops[i] == 0;
			p1_empty = p1_empty and (found ? p1.hops[i] : p0.hops[i]) == 0;
		} else {
			p0_empty = p0_empty and p0.hops[i] == 0;
			p1_empty = p1_empty and p1.hops[i] == 0;
		}
	}

	return not p0_empty and not p1_empty;
}

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

	void push(path p);
	void clear();
	void repair();

	list<path>::iterator erase(list<path>::iterator i);
	list<path>::iterator begin();
	list<path>::iterator end();

	void zero(petri::iterator i);
	void zero(vector<petri::iterator> i);
	void inc(petri::iterator i, int v = 1);
	void dec(petri::iterator i, int v = 1);
	void inc(list<path>::iterator i, petri::iterator j, int v = 1);
	void dec(list<path>::iterator i, petri::iterator j, int v = 1);

	path_set mask();
	path_set inverse_mask();

	path_set coverage(petri::iterator i);
	path_set avoidance(petri::iterator i);
	path_set avoidance(vector<petri::iterator> i);
	bool covers(petri::iterator i) const;
	bool covers(vector<petri::iterator> i) const;
	bool touches(vector<petri::iterator> i) const;

	vector<vector<petri::iterator> > enumerate();

	path_set &operator=(const path_set &p);
	path_set &operator+=(const path_set &p);
	path_set &operator*=(const path &p);

	template <class place, class transition, class token, class state>
	bool normalize(graph<place, transition, token, state> &g) {
		for (auto x = paths.begin(); x != paths.end(); x++) {
			for (auto y = std::next(x); y != paths.end(); y++) {
				if (not petri::normalize(g, *x, *y)) {
					clear();
					return false;
				}
			}
		}

		repair();
		return true;
	}

	template <class place, class transition, class token, class state>
	bool merge(graph<place, transition, token, state> &g, path_set p1)
	{
		for (auto x = paths.begin(); x != paths.end(); x++) {
			for (auto y = p1.paths.begin(); y != p1.paths.end(); y++) {
				if (not petri::normalize(g, *x, *y)) {
					clear();
					return false;
				}
			}
		}

		paths.insert(paths.end(), p1.paths.begin(), p1.paths.end());
		repair();
		return true;
	}
};

template <class place, class transition, class token, class state>
bool mergible(graph<place, transition, token, state> &g, const path_set &p0, const path_set &p1)
{
	for (auto x = p0.paths.begin(); x != p0.paths.end(); x++) {
		for (auto y = p1.paths.begin(); y != p1.paths.end(); y++) {
			if (not petri::mergible(g, *x, *y)) {
				return false;
			}
		}
	}

	return true;
}

ostream &operator<<(ostream &os, const path &p);

path operator+(path p0, path p1);
path operator-(path p0, path p1);
path operator*(path p0, path p1);
path operator/(path p1, int n);
path operator*(path p1, int n);

ostream &operator<<(ostream &os, const path_set &p);

path_set operator+(path_set p0, path_set p1);
path_set operator&(path_set p0, path_set p1);
path_set operator*(path_set p0, path p1);
path_set operator*(path p0, path_set p1);

template <class place, class transition, class token, class state>
path_set trace(graph<place, transition, token, state> &g, vector<vector<petri::iterator> > from, vector<petri::iterator> to, bool mark_from=false, bool mark_to=false) {
	if (from.empty() or to.empty()) {
		return path_set(g.places.size(), g.transitions.size());
	}

	path_set result(g.places.size(), g.transitions.size());
	// each item in the stack is a pair
	// The first element in the pair represents the frontier of the
	// trace. When the frontier is empty, we have completed the trace.
	// The second element in the pair is the path we've traced. Any
	// frontier node that encounters one of our targets gets placed
	// into the path's "to" list. If the trace completes and there are
	// no elements in the path's "to" list, then there was no path
	// found.

	vector<pair<vector<petri::iterator>, path> > stack;
	// initialize the stack. To do this, we break the from list into conditional
	// groups of parallel nodes. Nodes that are in sequence with eachother should
	// be treated as conditional as well.
	for (auto partial = from.begin(); partial != from.end(); partial++) {
		stack.push_back(pair<vector<petri::iterator>, path>(*partial, path(g.places.size(), g.transitions.size())));
		stack.back().second.from = *partial;
	}

	// precache "next" list for all nodes in graph to accelerate
	// computation.
	array<vector<vector<petri::iterator> >, 2> n;
	for (int type = 0; type < 2; type++) {
		n[type].resize(g.size(type), vector<petri::iterator>());

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

			int j = ((int)group.size())-1;
			int k = ((int)groups.size())-1;
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
			if (j->split >= 0) {
				for (auto k = j->branch.begin(); k != j->branch.end(); k++) {
					n[type][j->split].push_back(petri::iterator(1-type, *k));
				}
			}
		}

		// clean up and fill out the precached "next" lists for the
		// remaining nodes.
		for (auto i = g.begin(type); i != g.end(type); i++) {
			if (n[i.type][i.index].empty()) {
				n[i.type][i.index] = g.next(i);
			}
		}
	}

	// This is an optimization. Make it easier to identify from/to nodes
	path fromCount(g.places.size(), g.transitions.size());
	path toCount(g.places.size(), g.transitions.size());
	for (auto i = from.begin(); i != from.end(); i++) {
		for (auto j = i->begin(); j != i->end(); j++) {
			fromCount.set(*j);
		}
	}
	for (auto i = to.begin(); i != to.end(); i++) {
		toCount.set(*i);
	}

	while (not stack.empty()) {
		auto curr = stack.back();
		stack.pop_back();

		petri::iterator pos = curr.first.back();
		curr.first.pop_back();

		if (pos.type == transition::type) {
			for (auto i = n[pos.type][pos.index].begin(); i != n[pos.type][pos.index].end(); i++) {
				if (toCount[*i] > 0) {
					if (curr.second[*i] == 0) {
						curr.second.to.push_back(*i);
					}
				} else if (curr.second[*i] == 0 and fromCount[*i] == 0) {
					curr.first.push_back(*i);
					curr.second.set(*i);
				}
			}
			if (curr.first.empty()) {
				if (not curr.second.to.empty()) {
					if (mark_from) {
						for (auto i = curr.second.from.begin(); i != curr.second.from.end(); i++) {
							curr.second.set(*i, 1);
						}
					}
					if (mark_to) {
						for (auto i = curr.second.to.begin(); i != curr.second.to.end(); i++) {
							curr.second.set(*i, 1);
						}
					}
					result.push(curr.second);
				}
			} else {
				stack.push_back(curr);
			}
		}	else {
			for (auto i = n[pos.type][pos.index].begin(); i != n[pos.type][pos.index].end(); i++) {
				pair<vector<petri::iterator>, path> copy = curr;
				if (toCount[*i] > 0) {
					if (copy.second[*i] == 0) {
						copy.second.to.push_back(*i);
					}
					copy.second.set(*i);
				} else if (curr.second[*i] == 0 and fromCount[*i] == 0) {
					copy.second.set(*i);
					copy.first.push_back(*i);
				}
				if (copy.first.empty()) {
					if (not copy.second.to.empty()) {
						if (mark_from) {
							for (auto i = copy.second.from.begin(); i != copy.second.from.end(); i++) {
								copy.second.set(*i, 1);
							}
						}
						if (mark_to) {
							for (auto i = copy.second.to.begin(); i != copy.second.to.end(); i++) {
								copy.second.set(*i, 1);
							}
						}
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

