#pragma once

#include <vector>

#include "iterator.h"
#include "node.h"

namespace petri {

// Calculate the minimum number of arcs between any two nodes. This data
// can be used to determine if one node is reachable from another or as
// a way to guide logic minimization heuristics based on what is the
// "most recent transition". The result is stored in distances in
// the following grid pattern:
//
// p = places.size();
// t = transitions.size();
//
//                              |               from                  |
//                              |                |                    |
//                              | places         | transitions        |
//                              | 0, 1, ..., p-1 | p, p+1, ..., p+t-1 |
//    __________________________|________________|____________________|
//                0*(p+t)       | from places    | from transitions   |
//         places 1*(p+t)       | to places      | to places          |
//                ...           |                |                    |
//                (p-1)*(p+t)   |                |                    |
//    to________________________|________________|____________________|
//                (p)*(p+t)     | from places    | from transitions   |
//    transitions (p+1)*(p+t)   | to transitions | to transitions     |
//                ...           |                |                    |
//                (p+t-1)*(p+t) |                |                    |
//    __________________________|________________|____________________|
struct ReachabilityAnalysis {
	std::vector<int> distances;
	size_t places;
	size_t nodes;

	ReachabilityAnalysis();
	ReachabilityAnalysis(const Adjacency &g);
	~ReachabilityAnalysis();

	size_t index(petri::iterator i) const;
	size_t index(petri::iterator from, petri::iterator to) const;

	void reset(const Adjacency &g);
	void build(const Adjacency &g, petri::iterator pos);
	void build(const Adjacency &g);

	int distance(petri::iterator from, petri::iterator to) const;
	int distance(vector<petri::iterator> from, vector<petri::iterator> to) const;
	int minDistance(vector<petri::iterator> from, vector<petri::iterator> to) const;

	bool isReachable(petri::iterator from, petri::iterator to) const;
	bool isReachable(vector<petri::iterator> from, vector<petri::iterator> to) const;

	void print() const;
};

}
