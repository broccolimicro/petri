#pragma once

#include <common/standard.h>
#include "iterator.h"
#include "composition.h"

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

bool normalize(const CompositionAnalysis &comp, path &p0, path &p1);
bool mergible(const CompositionAnalysis &comp, const path &p0, const path &p1);

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

	bool normalize(const CompositionAnalysis &comp);
	bool merge(const CompositionAnalysis &comp, path_set p1);
};

bool mergible(const CompositionAnalysis &comp, const path_set &p0, const path_set &p1);

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

path_set trace(const Adjacency &adj, const CompositionAnalysis &comp, petri::bound from, vector<petri::iterator> to, bool mark_from=false, bool mark_to=false);

}

