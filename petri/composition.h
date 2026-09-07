#pragma once

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>
#include <common/index_vector.h>

#include <array>

#include "node.h"
#include "iterator.h"

namespace petri {

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

struct SplitGroups {
	// index with place::type or transition::type
	std::array<std::vector<SplitGroup>, 2> splits;
	std::array<std::vector<SplitGroup>, 2> merges;
};

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
	void build(const Adjacency &g);

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
};

}
