#include "reachability.h"

namespace petri {

ReachabilityAnalysis::ReachabilityAnalysis() {
	places = 0;
	nodes = 0;
}

ReachabilityAnalysis::ReachabilityAnalysis(const Adjacency &g) {
	places = 0;
	nodes = 0;
	build(g);
}

ReachabilityAnalysis::~ReachabilityAnalysis() {
}

size_t ReachabilityAnalysis::index(petri::iterator i) const {
	return places*i.type + i.index;
}

size_t ReachabilityAnalysis::index(petri::iterator from, petri::iterator to) const {
	return index(to)*nodes + index(from);
}

void ReachabilityAnalysis::reset(const Adjacency &g) {
	places = g.size(place::type);
	nodes = places + g.size(transition::type);

	distances.assign(nodes*nodes, std::numeric_limits<int>::min());
	for (size_t i = 0; i < nodes; i++) {
		distances[i*nodes + i] = 0;
	}
}

// Updates the node distance matrix for a specific node position
// This calculates minimum arc distances from the given node to all other nodes
// Uses a breadth-first search approach to find the shortest paths
// Maintains the cached distance data in distances
// Used in reachability analysis and path calculations
void ReachabilityAnalysis::build(const Adjacency &g, petri::iterator pos) {
	size_t posIdx = index(pos);

	vector<petri::iterator> stack;
	set<petri::iterator> seen;
	stack.push_back(pos);
	seen.insert(pos);
	while (not stack.empty()) {
		petri::iterator curr = stack.back();
		stack.pop_back();

		int toIdx = index(curr);
		for (auto i : g.prev(curr)) {
			if (not i.valid()) continue;

			int fromIdx = index(i);
			if (seen.insert(i).second) {
				distances[posIdx*nodes + fromIdx] = max(distances[posIdx*nodes + fromIdx], distances[posIdx*nodes + toIdx] + 1);
				stack.push_back(i);
			}
		}
	}
}

// Updates the entire node distance matrix for all nodes in the Petri net
// Calls update_distances for each place and transition
// Used to calculate the complete reachability information for the net
// This is an expensive operation and is performed only when necessary
void ReachabilityAnalysis::build(const Adjacency &g) {
	// TODO(edward.bingham) This needs to take the max of the node distances
	// for all of the pre-set nodes. However, doing so triggers an infinite
	// loop around loops with splits and merges

	// clear the current set of distances
	reset(g);

	for (auto i = g.begin(place::type); i != g.end(place::type); i++) {
		if (g.isValid(i)) {
			build(g, i);
		}
	}

	for (auto i = g.begin(transition::type); i != g.end(transition::type); i++) {
		if (g.isValid(i)) {
			build(g, i);
		}
	}
}

int ReachabilityAnalysis::distance(petri::iterator from, petri::iterator to) const {
	size_t idx = index(from, to);
	if (idx < distances.size()) {
		return distances[idx];
	}
	return std::numeric_limits<int>::min();
}

int ReachabilityAnalysis::distance(vector<petri::iterator> from, vector<petri::iterator> to) const {
	int result = std::numeric_limits<int>::min();
	for (auto i : from) {
		for (auto j : to) {
			int d = distance(i, j);
			if (d >= 0 and d > result) {
				result = d;
			}
		}
	}
	return result;
}

int ReachabilityAnalysis::minDistance(vector<petri::iterator> from, vector<petri::iterator> to) const {
	int result = std::numeric_limits<int>::max();
	for (auto i : from) {
		for (auto j : to) {
			int d = distance(i, j);
			if (d >= 0 and d < result) {
				result = d;
			}
		}
	}
	return result;
}

bool ReachabilityAnalysis::isReachable(petri::iterator from, petri::iterator to) const {
	return (distance(from, to) >= 0);
}

bool ReachabilityAnalysis::isReachable(vector<petri::iterator> from, vector<petri::iterator> to) const {
	for (auto i = from.begin(); i != from.end(); i++) {
		for (auto j = to.begin(); j != to.end(); j++) {
			if (distance(*i, *j) >= 0) {
				return true;
			}
		}
	}
	return false;
}

void ReachabilityAnalysis::print() const {
	for (size_t i = 0; i < places; i++) {
		cout << "p" << i << " " << "{";
		for (size_t j = 0; j < places; j++) {
			if (distance(petri::iterator(place::type, i), petri::iterator(place::type, j)) >= 0) {
				cout << "->p" << j << ":" << distance(petri::iterator(place::type, i), petri::iterator(place::type, j)) << " ";
			}
			if (distance(petri::iterator(place::type, j), petri::iterator(place::type, i)) >= 0) {
				cout << "p" << j << "->:" << distance(petri::iterator(place::type, j), petri::iterator(place::type, i)) << " ";
			}
		}

		for (size_t j = 0; j < nodes-places; j++) {
			if (distance(petri::iterator(place::type, i), petri::iterator(transition::type, j)) >= 0) {
				cout << "->t" << j << ":" << distance(petri::iterator(place::type, i), petri::iterator(transition::type, j)) << " ";
			}
			if (distance(petri::iterator(transition::type, j), petri::iterator(place::type, i)) >= 0) {
				cout << "t" << j << "->:" << distance(petri::iterator(transition::type, j), petri::iterator(place::type, i)) << " ";
			}
		}
		cout << "}" << endl;
	}
	for (size_t i = 0; i < nodes-places; i++) {
		cout << "t" << i << ": " << "{";

		for (size_t j = 0; j < places; j++) {
			if (distance(petri::iterator(transition::type, i), petri::iterator(place::type, j)) >= 0) {
				cout << "->p" << j << ":" << distance(petri::iterator(transition::type, i), petri::iterator(place::type, j)) << " ";
			}
			if (distance(petri::iterator(place::type, j), petri::iterator(transition::type, i)) >= 0) {
				cout << "p" << j << "->:" << distance(petri::iterator(place::type, j), petri::iterator(transition::type, i)) << " ";
			}
		}

		for (size_t j = 0; j < nodes-places; j++) {
			if (distance(petri::iterator(transition::type, i), petri::iterator(transition::type, j)) >= 0) {
				cout << "->t" << j << ":" << distance(petri::iterator(transition::type, i), petri::iterator(transition::type, j)) << " ";
			}
			if (distance(petri::iterator(transition::type, j), petri::iterator(transition::type, i)) >= 0) {
				cout << "t" << j << "->:" << distance(petri::iterator(transition::type, j), petri::iterator(transition::type, i)) << " ";
			}
		}
		cout << "}" << endl;
	}
}

}

