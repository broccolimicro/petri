#pragma once

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>
#include <common/index_vector.h>

#include <array>

#include "node.h"
#include "iterator.h"

namespace petri {

// Records which outgoing branches of a Petri-net split apply to a node.
//
// `split` is the index of the place or transition where the split occurs.
// `branch` contains the indices of the immediate successors that start
// the represented branches. `count` is the total number of outgoing
// branches at the split, even when `branch` contains only a subset.
// A synthetic reset split uses `split == -1`.
//
// Keep `branch` sorted in ascending order with unique indices when
// passing this group to `compare()` or `merge()`. The constructor stores
// the branch list as supplied.
//
// Ordering uses `split` alone. Equality between two groups checks
// `split` and the branch indices in order. `count` does not affect
// equality. Comparisons with an integer use only `split`.
struct SplitGroup {
	SplitGroup();
	SplitGroup(int split, int count, vector<int> branch=vector<int>());
	~SplitGroup();

	enum Operation {
		INTERSECT = 0,
		UNION = 1,
		DIFFERENCE = 2,
		NEGATIVE_DIFFERENCE = 3,
		SYMMETRIC_DIFFERENCE = 4,
		SUBSET = 5,
		SUBSET_EQUAL = 6,
		NOT_EQUAL = 7
	};

	int split; // index of place/transition with split
	std::vector<int> branch; // index of transitions/places coming out of split
	int count; // total number of branches out of this split

	void unionWith(std::vector<int> newBranches);
	void intersectWith(std::vector<int> newBranches);

	string to_string() const;
};

bool operator<(const SplitGroup &g0, const SplitGroup &g1);
bool operator==(const SplitGroup &g0, const SplitGroup &g1);

bool operator<(const SplitGroup &g0, int split);
bool operator==(const SplitGroup &g0, int split);

ostream &operator<<(ostream &os, const SplitGroup &g0);

bool compare(int group_operation, int branch_operation, vector<SplitGroup> g0, vector<SplitGroup> g1);
vector<SplitGroup> merge(int group_operation, int branch_operation, vector<SplitGroup> g0, vector<SplitGroup> g1);
void mergeInplace(int group_operation, int branch_operation, vector<SplitGroup> &g0, const vector<SplitGroup> &g1, set<int> exclude=set<int>());

// Stores branch membership for one place or transition in the Petri net.
//
// Index `splits` with `Composition::CHOICE` for groups from choice splits
// at places, or with `Composition::PARALLEL` for groups from parallel
// splits at transitions. Each list is sorted by split ID and contains at
// most one group per split. Each group records which branches of that
// split reach the node.
//
// `merges` is reserved for merge groups. The current analysis leaves
// both lists empty.
struct SplitGroups {
	std::array<std::vector<SplitGroup>, 2> splits;
	std::array<std::vector<SplitGroup>, 2> merges;
};

struct Condition {
	std::vector<SplitGroup> groups;
	std::vector<size_t> transitions;
};

// Analyzes choice and parallel composition in a Petri net.
//
// Construct from an `Adjacency` graph or call `build()` to compute split
// groups for each place and transition. A split group records which
// branches of a choice or parallel split reach a node. Use
// `splitGroupsOf()` to get the groups for one node or combine the groups
// for several nodes.
//
// `build()` analyzes choice splits at places before it analyzes parallel
// splits at transitions. It uses the choice groups to handle parallel
// branches that depend on earlier choices. It removes choice splits
// whose outcome is already determined by an earlier choice.
//
// The analysis stores the graph's reset markings. It treats multiple
// reset markings as branches of a synthetic choice split and the places
// within each marking as branches of a synthetic parallel split.
//
// Use `is()` to test sequence, choice, parallelism, implication, or
// exclusion between nodes or regions. Call `build()` again after
// changing the graph or its reset markings.
struct CompositionAnalysis {
	index_vector<SplitGroups> places;
	index_vector<SplitGroups> transitions;
	std::vector<std::vector<petri::iterator> > reset;

	CompositionAnalysis();
	CompositionAnalysis(const Adjacency &g);
	~CompositionAnalysis();

	bool isValid(petri::iterator i) const;
	petri::iterator begin(int type) const;
	petri::iterator end(int type) const;

	bool empty() const;
	void clear();

	bool splitIsCovered(petri::iterator p, vector<petri::iterator> n) const;
	void build(const Adjacency &g, Composition composition, int split, vector<petri::iterator> init);
	void build(Adjacency g);

	void setSplitGroup(Composition composition, petri::iterator node, SplitGroup g);
	SplitGroup getSplitGroup(Composition composition, petri::iterator node, int split) const;
	std::vector<SplitGroup> *splitGroupsIter(Composition composition, petri::iterator node);
	std::vector<SplitGroup> splitGroupsOf(Composition composition, petri::iterator node) const;
	std::vector<SplitGroup> splitGroupsOf(Composition composition, SplitGroup::Operation groupOperation, SplitGroup::Operation branchOperation, vector<petri::iterator> nodes) const;
	
	bool isExcludes(petri::iterator a, petri::iterator b, bool always=false) const;
	bool isImplies(petri::iterator a, petri::iterator b, bool always=false) const;
	bool isChoice(petri::iterator a, petri::iterator b, bool always=false) const;
	bool isParallel(petri::iterator a, petri::iterator b, bool always=false) const;
	bool isSequence(petri::iterator a, petri::iterator b, bool always=false) const;

	bool is(Composition composition, petri::iterator a, petri::iterator b, bool always=false, bool bidir=false) const;
	bool is(Composition composition, petri::region a, petri::region b, bool always=false, bool bidir=false) const;

	vector<SplitGroup> invert(const Adjacency &g, Composition composition, std::vector<SplitGroup> groups) const;
	bool crossesReset(vector<petri::iterator> pos) const;

	bound complete(Composition composition, bound nodes) const;
	vector<array<petri::bound, 2> > deinterfere_choice(vector<petri::iterator> v0, vector<petri::iterator> v1) const;
	vector<array<petri::region, 2> > deinterfere(petri::region v0, petri::region v1) const;
	bound select(Composition composition, vector<petri::iterator> nodes, bool always=false, bool invert=false) const;
	bound group(Composition composition, bound nodes, bool always=false, bool invert=false) const;
	bound partials(Composition composition, petri::region nodes, vector<petri::iterator> other=vector<petri::iterator>()) const;

	/*bool isRedundantTo(petri::iterator p0, petri::iterator p1) const;
	bool isRedundantTo(petri::iterator p0, vector<petri::iterator> p1) const;
	bool isRedundant(petri::iterator p0) const;
	vector<petri::iterator> addRedundant(vector<petri::iterator> p) const;*/

	std::vector<Condition> elaborateConditions(const petri::Adjacency &adj) const;
};

//std::vector<Condition> elaborateConditions(const petri::Adjacency &adj) const;

}
