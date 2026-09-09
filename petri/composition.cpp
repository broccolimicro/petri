#include "composition.h"
#include <common/message.h>
#include <common/text.h>
#include <limits>

namespace petri {

SplitGroup::SplitGroup() {
	split = std::numeric_limits<int>::min();
	count = 0;
}

SplitGroup::SplitGroup(int split, int count, vector<int> branch) {
	this->split = split;
	this->count = count;
	this->branch = branch;
}

SplitGroup::~SplitGroup() {}

string SplitGroup::to_string() const {
	return "(" + ::to_string(split) + "," + ::to_string(branch) + "/" + ::to_string(count) + ")";
}

bool operator<(const SplitGroup &g0, const SplitGroup &g1)
{
	return g0.split < g1.split;
}

bool operator==(const SplitGroup &g0, const SplitGroup &g1)
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

bool operator<(const SplitGroup &g0, int split) {
	return g0.split < split;
}

bool operator==(const SplitGroup &g0, int split) {
	return g0.split == split;
}

ostream &operator<<(ostream &os, const SplitGroup &g0) {
	os << g0.to_string();
	return os;
}

bool compare(int group_operation, int branch_operation, vector<SplitGroup> g0, vector<SplitGroup> g1) {
	// group_operation is one of:
	// SplitGroup::INTERSECT
	// SplitGroup::DIFFERENCE
	// SplitGroup::SUBSET

	// branch_operation is one of:
	// SplitGroup::INTERSECT
	// SplitGroup::DIFFERENCE
	// SplitGroup::SUBSET

	int branch_cmp = -1;
	int group_cmp = -1;

	int i = 0, j = 0;
	while (i < (int)g0.size() or j < (int)g1.size()) {
		if (i < (int)g0.size() and j < (int)g1.size() and g0[i].split == g1[j].split) {
			if (group_operation == SplitGroup::INTERSECT
				or group_operation == SplitGroup::DIFFERENCE
				or group_operation == SplitGroup::NEGATIVE_DIFFERENCE) {
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
				if ((branch_operation == SplitGroup::SYMMETRIC_DIFFERENCE and found0 and found1)
					or (branch_operation == SplitGroup::INTERSECT and found2)) {
					return true;
				}

				if (branch_operation == SplitGroup::DIFFERENCE and found0) {
					return true;
				}
				if (branch_operation == SplitGroup::NEGATIVE_DIFFERENCE and found1) {
					return true;
				}
				if (branch_operation == SplitGroup::NOT_EQUAL and (found0 or found1)) {
					return true;
				}

				if ((branch_operation == SplitGroup::SUBSET
					or branch_operation == SplitGroup::SUBSET_EQUAL) and found0) {
					if (branch_cmp == 1) {
						return false;
					}
					branch_cmp = 0;
				}

				if ((branch_operation == SplitGroup::SUBSET
					or branch_operation == SplitGroup::SUBSET_EQUAL) and found1) {
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
				and (group_operation == SplitGroup::DIFFERENCE
					or group_operation == SplitGroup::SYMMETRIC_DIFFERENCE)) {
				return true;
			} else if (group_operation == SplitGroup::SUBSET
				or group_operation == SplitGroup::SUBSET_EQUAL) {
				if (group_cmp == 1) {
					return false;
				}
				group_cmp = 0;
			}
			i++;
		} else if (j < (int)g1.size()) {
			if ((int)g1[j].branch.size() < g1[j].count
				and (group_operation == SplitGroup::NEGATIVE_DIFFERENCE
					or group_operation == SplitGroup::SYMMETRIC_DIFFERENCE)) {
				return true;
			} else if (group_operation == SplitGroup::SUBSET
				or group_operation == SplitGroup::SUBSET_EQUAL) {
				if (group_cmp == 0) {
					return false;
				}
				group_cmp = 1;
			}
			j++;
		}
	}
	return ((group_operation == SplitGroup::SUBSET_EQUAL
		or (group_operation == SplitGroup::SUBSET and group_cmp != -1)
		or group_operation == SplitGroup::INTERSECT)
			and (branch_operation == SplitGroup::SUBSET_EQUAL
				or (branch_operation == SplitGroup::SUBSET and branch_cmp != -1)));
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
vector<SplitGroup> merge(int group_operation, int branch_operation, vector<SplitGroup> g0, vector<SplitGroup> g1) {
	// group_operation is one of:
	// SplitGroup::INTERSECT
	// SplitGroup::UNION

	// branch_operation is one of:
	// SplitGroup::INTERSECT
	// SplitGroup::UNION
	// SplitGroup::DIFFERENCE

	vector<SplitGroup> result;
	int i = 0, j = 0;
	while (i < (int)g0.size() or j < (int)g1.size()) {
		if (i < (int)g0.size() and j < (int)g1.size() and g0[i].split == g1[j].split) {
			result.push_back(SplitGroup());
			result.back().split = g0[i].split;
			result.back().count = g0[i].count;

			int k = 0, l = 0;
			while (k < (int)g0[i].branch.size() or l < (int)g1[j].branch.size()) {
				if (k < (int)g0[i].branch.size() and l < (int)g1[j].branch.size() and g0[i].branch[k] == g1[j].branch[l]) {
					if (branch_operation != SplitGroup::DIFFERENCE) {
						result.back().branch.push_back(g0[i].branch[k]);
					}
					k++;
					l++;
				} else if (k < (int)g0[i].branch.size() and (l >= (int)g1[j].branch.size() or g0[i].branch[k] < g1[j].branch[l])) {
					if (branch_operation != SplitGroup::INTERSECT) {
						result.back().branch.push_back(g0[i].branch[k]);
					}
					k++;
				} else if (l < (int)g1[j].branch.size()) {
					if (branch_operation == SplitGroup::UNION) {
						result.back().branch.push_back(g1[j].branch[l]);
					}
					l++;
				}
			}
			i++;
			j++;
		} else if (i < (int)g0.size() and (j >= (int)g1.size() or g0[i].split < g1[j].split)) {
			if (group_operation != SplitGroup::INTERSECT) {
				result.push_back(g0[i]);
			}
			i++;
		} else if (j < (int)g1.size()) {
			if (group_operation == SplitGroup::UNION) {
				result.push_back(g1[j]);
			}
			j++;
		}
	}
	return result;
}

void mergeInplace(int group_operation, int branch_operation, vector<SplitGroup> &g0, const vector<SplitGroup> &g1, set<int> exclude) {
	// group_operation is one of:
	// SplitGroup::INTERSECT
	// SplitGroup::UNION

	// branch_operation is one of:
	// SplitGroup::INTERSECT
	// SplitGroup::UNION
	// SplitGroup::DIFFERENCE

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
					if (branch_operation == SplitGroup::INTERSECT) {
						g0[i].branch.erase(g0[i].branch.begin()+k);
					} else {
						k++;
					}
				} else if (l < (int)g1[j].branch.size()) {
					if (branch_operation == SplitGroup::UNION) {
						g0[i].branch.insert(g0[i].branch.begin()+k, g1[j].branch[l]);
						k++;
					}
					l++;
				}
			}
			i++;
			j++;
		} else if (i < (int)g0.size() and (j >= (int)g1.size() or g0[i].split < g1[j].split)) {
			if (group_operation == SplitGroup::INTERSECT) {
				g0.erase(g0.begin()+i);
			} else {
				i++;
			}
		} else if (j < (int)g1.size()) {
			if (group_operation == SplitGroup::UNION) {
				g0.insert(g0.begin()+i, g1[j]);
				i++;
			}
			j++;
		}
	}
}

CompositionAnalysis::CompositionAnalysis() {
}

CompositionAnalysis::CompositionAnalysis(const Adjacency &g) {
	build(g);
}

CompositionAnalysis::~CompositionAnalysis() {
}

bool CompositionAnalysis::isValid(petri::iterator i) const {
	if (i.type == place::type) {
		return places.is_valid(i.index);
	}
	return transitions.is_valid(i.index);
}

petri::iterator CompositionAnalysis::begin(int type) const {
	return petri::iterator(type, 0);
}

petri::iterator CompositionAnalysis::end(int type) const {
	return petri::iterator(type, type == place::type ? places.size() : transitions.size());
}

bool CompositionAnalysis::empty() const {
	return places.empty() and transitions.empty();
}

void CompositionAnalysis::clear() {
	places.clear();
	transitions.clear();
	reset.clear();
}

// DESIGN(edward.bingham) In the following structure, p6 and p4 are not real
// conditional splits. The real conditional split is where the reset
// tokens are placed. If they are placed on p0 and p4, then only the top
// two branches will ever be executed, and they will always be executed
// in sequence. If they are placed on p4 and p8, then only the bottom two
// branches will be executed.
// 
// =->t0-->p0      ->p1-->t1-->p2      ->p3--=   .
//           \    /              \    /          .
//            ->t2                ->t3           .
//           /    \              /    \          .
// =->t4-->p4      ->p5-->t5-->p6      ->p7--=   .
//           \    /              \    /          .
//            ->t6                ->t7           .
//           /    \              /    \          .
// =->t8-->p8      ->p9-->t9-->p10     ->p11-=   .
//
// The same effect happens for non-reset splits as well. If we take the
// t0 branch on the p0 split, then we are _guaranteed_ to execute t3, and
// if we take the t4 branch on the p0 split, then we are _guaranteed_ to
// execute t6. This means that p4 is not a real conditional split. The
// choice has already been made.
//
//          ->p1-->t1-->p2             .
//         /              \            .
//     ->t0                ->t3        .
//    /    \              /    \       .
//  p0      ->p3-->t2-->p4      ->p7   .
//    \    /              \    /       .
//     ->t4                ->t6        .
//         \              /            .
//          ->p5-->t5-->p6             .
//
// If each output transition of that split is on a
// different branch of another conditional split group found at that split,
// then that split is "covered". If a parent split only partially covers
// the child split or if there are multiple parent splits that together
// cover the child split, then it doesn't count because there still exists
// a choice between the ones that are covered and the ones that aren't, or
// the ones that are covered by one split and the ones that are covered by
// the other. If there are two output transitions that have the same parent
// branch, and then another parent branch that covers those two, then it
// counts. So, I need to identify a set of parents that hierarchically
// break apart the transitions into selection groups. This is like a tree
// of splits. Technically each sub split has to also have the parent split
// as a parent. otherwise, it's just compressed properly nested choice
// split followed by parallel merge.
bool CompositionAnalysis::splitIsCovered(petri::iterator p, vector<petri::iterator> n) const {
	if (n.size() <= 1) {
		return true;
	}

	vector<SplitGroup> groups = splitGroupsOf(Composition::CHOICE, p);
	for (auto group = groups.begin(); group != groups.end(); group++) {
		bool found = true;
		vector<vector<petri::iterator> > clusters;
		clusters.resize(group->count);
		for (auto j = n.begin(); j != n.end() and found; j++) {
			vector<SplitGroup> subs = splitGroupsOf(Composition::CHOICE, *j);
			auto pos = find(subs.begin(), subs.end(), group->split);
			if (pos != subs.end()) {
				for (auto i = pos->branch.begin(); i != pos->branch.end() and found; i++) {
					auto idx = find(group->branch.begin(), group->branch.end(), *i);
					if (idx != group->branch.end()) {
						clusters[idx-group->branch.begin()].push_back(*j);
					} else {
						found = false;
					}
				}
			} else {
				found = false;
			}
		}

		for (auto cluster = clusters.begin(); cluster != clusters.end() and found; cluster++) {
			if (cluster->size() == n.size() or not splitIsCovered(p, *cluster)) {
				found = false;
			}
		}

		if (found) {
			return true;
		}
	}

	return false;
}

// Computes and propagates split-merge relationship information for a specific split point.
//
// This function analyzes how nodes in the Petri net relate to a specific split point,
// identifying which execution branches they belong to. It is fundamental for understanding
// concurrent behaviors and choice patterns in the net. The algorithm performs a sophisticated
// flow analysis using forward and backward traversal to handle properly nested and non-properly
// nested splits and merges.
//
// The first phase is a forward traversal that propagates branch information from split points,
// using a specialized algorithm to resolve complex merge situations. The second phase is a
// backward cleanup that removes unnecessary split markings. Together, these phases create a
// complete picture of which branches each node belongs to.
//
// @param composition The composition type to analyze (parallel or choice)
// @param split The index of the split node being analyzed
// @param init Vector of initial nodes representing the branches of the split
void CompositionAnalysis::build(const Adjacency &g, Composition composition, int split, vector<petri::iterator> init) {
	if (init.size() <= 1) {
		// there is no split here
		return;
	}

	petri::iterator splitNode(
			composition == Composition::PARALLEL ? transition::type : place::type,
			split);

	vector<petri::iterator> frontier;
	// TODO(edward.bingham) maybe want to use a "path" here
	set<petri::iterator> seen;
	seen.insert(init.begin(), init.end());
	for (auto i = init.begin(); i != init.end(); i++) {
		if (i->index >= 0) {
			setSplitGroup(composition, *i, SplitGroup(split, init.size(), {i->index}));
		}
	}

	// I need to be able to do two things:
	// 1. continue through a merge
	// 2. stop when we encounter a node we've already seen
	// problem is that when we encounter a merge, we visit that merge once for each branch into that merge. This counts as having "already seen" that node.
	// If we stop and wait for all of the branches of the merge, and we encounter multiple blocked merges, then how do we know which merge to unblock first? Because one of the blocked merges may lead to the next.
	// unblock the merge that is closest to the split

	// Forward iteration step
	//cout << "start loop " << split << endl;
	map<petri::iterator, SplitGroup> enabled;
	map<petri::iterator, int> order;
	int orderLevel = 0;
	do {
		enabled.clear();
		//cout << "step " << ::to_string(seen) << endl;

		map<petri::iterator, SplitGroup> blocked;
		for (int type = 0; type < 2; type++) {
			for (petri::iterator to = g.begin(type); to != g.end(type); to++) {
				if (not g.isValid(to)) continue;

				bool isDisabled = false;
				bool isEnabled = false;
				SplitGroup toSplit = getSplitGroup(composition, to, split);
				// Ensure that we haven't encountered this split yet.
				if (toSplit.split != split) {
					toSplit.split = split;
					toSplit.count = init.size();

					// Check to make sure we've visited all of the input nodes and
					// derive the split group for this node.
					for (auto from = g.prev(to).begin(); from != g.prev(to).end(); from++) {
						SplitGroup fromSplit = getSplitGroup(composition, *from, split);
						if (fromSplit.split != split) {
							isDisabled = true;
							if (type == transition::type
								and (composition == Composition::CHOICE
									or (composition == Composition::PARALLEL
										and compare(SplitGroup::NEGATIVE_DIFFERENCE, SplitGroup::DIFFERENCE,
											splitGroupsOf(Composition::CHOICE, splitNode),
											splitGroupsOf(Composition::CHOICE, *from))))) {
								toSplit.branch.clear();
								break;
							}
							continue;
						}
						isEnabled = true;

						if (from == g.prev(to).begin()
							or not (to.type == transition::type and composition == Composition::CHOICE)) {
							toSplit.branch.insert(
								toSplit.branch.end(),
								fromSplit.branch.begin(),
								fromSplit.branch.end());
							sort(toSplit.branch.begin(), toSplit.branch.end());
							toSplit.branch.erase(unique(toSplit.branch.begin(), toSplit.branch.end()), toSplit.branch.end());
						} else {
							toSplit.branch = vector_intersection(toSplit.branch, fromSplit.branch);
						}
					}

					// If we've visited at least one of the input nodes, then this
					// might be a blocked merge. If we've visited all of the input
					// nodes, then we can make forward progress.
					if (isEnabled and isDisabled) {
						blocked.insert({to, toSplit});
					} else if (isEnabled) {
						enabled.insert({to, toSplit});
					}
				}
			}
		}

		//cout << "enabled: " << ::to_string(enabled) << endl;
		//cout << "blocked: " << ::to_string(blocked) << endl;

		if (enabled.empty() and not blocked.empty()) {
			// We are stuck at a non-properly nested merge somewhere. We need to
			// figure out where and then force progression through that merge. We
			// may be stuck at multiple merges, we need to find the one that
			// precedes all of the others.

			for (auto i = blocked.begin(); i != blocked.end(); i++) {
				auto pos = order.find(i->first);
				if (pos == order.end()) {
					order.insert({i->first, orderLevel});
				}
			}
			orderLevel++;

			//cout << ::to_string(order) << " " << to_string(seen) << endl;
			auto first = blocked.begin();
			for (auto i = ::next(blocked.begin()); i != blocked.end(); i++) {
				bool AtoB = g.precedes(i->first, first->first, seen);
				bool BtoA = g.precedes(first->first, i->first, seen);
				if (AtoB and BtoA) {
					auto orderA = order.find(i->first);
					auto orderB = order.find(first->first);
					if (orderB == order.end()
						or (orderA != order.end() and orderA->second < orderB->second)) {
						first = i;
					}
				} else if (AtoB) {
					first = i;
				}
			}
			//cout << "forcing " << first->first << endl;
			enabled.insert(*first);
		}

		for (auto i = enabled.begin(); i != enabled.end(); i++) {
			sort(i->second.branch.begin(), i->second.branch.end());
			i->second.branch.erase(
				unique(i->second.branch.begin(), i->second.branch.end()),
				i->second.branch.end());
			if (g.next(i->first).empty()) {
				//cout << "ready frontier end adding " << i->first << endl;
				frontier.push_back(i->first);
			}
			for (auto j : g.next(i->first)) {
				if (getSplitGroup(composition, j, split).split == split) {
					//cout << "ready frontier loop adding " << i->first << endl;
					frontier.push_back(i->first);
				}
			}
		}

		for (auto i = enabled.begin(); i != enabled.end(); i++) {
			// replace the set of groups that exist at this location
			setSplitGroup(composition, i->first, i->second);
			seen.insert(i->first);
		}
	} while (not enabled.empty());
	//cout << "done loop" << endl;

	sort(frontier.begin(), frontier.end());
	frontier.erase(unique(frontier.begin(), frontier.end()), frontier.end());

	//cout << ::to_string(frontier) << endl;
	//print();
	//cout << endl << endl;

	// reverse iteration step: walk backward from the identified recursion
	// points and delete back to the last encountered merge

	// TODO(edward.bingham) I think this needs to be a breadth-first-search
	// rather than depth-first-search
	vector<petri::iterator> todo = frontier;
	while (not todo.empty()) {
		petri::iterator curr = todo.back();
		todo.pop_back();

		bool found = false;
		vector<SplitGroup> *cgroups = splitGroupsIter(composition, curr);
		if (cgroups != nullptr) {
			for (int j = (int)cgroups->size()-1; j >= 0; j--) {
				auto pos = cgroups->begin()+j;
				if (pos->split == split and (pos->branch.empty() or (int)pos->branch.size() >= pos->count)) {
					cgroups->erase(pos);
					found = true;
					break;
				}
			}
		}
		if (not found) {
			continue;
		}

		bool closed = true;
		for (auto i = g.prev(curr).begin(); i != g.prev(curr).end() and closed; i++) {
			vector<SplitGroup> groups = splitGroupsOf(composition, *i);
			for (auto j = groups.begin(); j != groups.end() and closed; j++) {
				closed = (j->split != split or j->branch.empty() or (int)j->branch.size() >= j->count);
			}
		}

		if (closed) {
			todo.insert(todo.end(), g.prev(curr).begin(), g.prev(curr).end());
			sort(todo.begin(), todo.end());
			todo.erase(unique(todo.begin(), todo.end()), todo.end());
		}
	}

	//cout << "done backtrack" << endl;
	//print();
	//cout << endl << endl;
}

// Analyzes and computes all split-merge relationships throughout the Petri net.
//
// This function serves as the primary coordinator for split group analysis in the net.
// It systematically analyzes the structure of the Petri net to identify and catalog all 
// conditional (choice) and parallel execution splits. The function first processes choice splits,
// which is necessary to provide context for parallel splits analysis, then analyzes parallel
// splits. It also handles special cases related to reset states, which represent initial 
// markings of the net.
//
// After identifying potential split points, the function calls build for each one,
// then performs post-processing to remove "covered" conditional splits - those that don't 
// represent true choices because they are predetermined by earlier choices in the net. This
// creates a clean, accurate representation of the net's behavioral structure.
//
// The information computed by this function is essential for higher-level relationship
// analysis like determining if nodes are in sequence, choice, or parallel relationships.
void CompositionAnalysis::build(Adjacency g) {
	reset = g.reset;
	for (size_t i = 0; i < reset.size(); i++) {
		for (size_t j = 0; j < reset[i].size(); j++) {
			g.p[place::type][reset[i][j].index].push_back(petri::iterator(transition::type, -i-1));
		}
	}
	// DESIGN(edward.bingham) Choice must go first, because we use that to
	// determine whether we're dealing with non-properly nested parallelism or
	// shared conditional parallel branches. It just so happens that "choice" =
	// 0 and "parallel" = 1
	places.clear();
	transitions.clear();
	places.alloc(g.size(place::type));
	transitions.alloc(g.size(transition::type));
	for (auto i = g.begin(place::type); i != g.end(place::type); i++) {
		if (g.isValid(i)) {
			places.emplace_at(i.index);
		}
	}
	for (auto i = g.begin(transition::type); i != g.end(transition::type); i++) {
		if (g.isValid(i)) {
			transitions.emplace_at(i.index);
		}
	}

	for (int composition = 0; composition != 2; composition++) {
		// each place belongs to some set of parallel splits (init[place])
		if (composition == Composition::PARALLEL) {
			// add parallel splits from reset states
			if (not reset.empty()) {
				for (int i = 0; i < (int)reset.size(); i++) {
					build(g, (Composition)composition, -i-1, reset[i]);
				}
			}
		} else if (composition == Composition::CHOICE) {
			if (reset.size() > 1) {
				vector<petri::iterator> branches;
				for (int i = 0; i < (int)reset.size(); i++) {
					branches.push_back(petri::iterator(transition::type, -i-1));
				}
				build(g, (Composition)composition, -1, branches);
			}
		}

		// A the moment, parallel == transition::type and choice == place::type,
		// but that's not necessarily guaranteed.
		int split_type = (composition == Composition::PARALLEL ? transition::type : place::type);

		// add splits from graph structure at the first branch nodes after each split
		for (petri::iterator i = g.begin(split_type); i != g.end(split_type); i++) {
			if (not g.isValid(i)) continue;

			build(g, (Composition)composition, i.index, g.next(i));
		}

		// See splitIsCovered() for documentation. Remove "covered" conditional splits.
		if (composition == Composition::CHOICE) {
			set<int> covered;
			for (petri::iterator i = g.begin(split_type); i != g.end(split_type); i++) {
				if (not g.isValid(i)) continue;

				if (splitIsCovered(i, g.next(i))) {
					covered.insert(i.index);
				}
			}

			for (int type = 0; type < 2; type++) {
				for (petri::iterator i = g.begin(type); i != g.end(type); i++) {
					if (not g.isValid(i)) continue;

					vector<SplitGroup> *groups = splitGroupsIter(Composition::CHOICE, i);
					for (int j = (int)groups->size()-1; j >= 0; j--) {
						auto pos = groups->begin()+j;
						if (covered.find(pos->split) != covered.end()) {
							groups->erase(pos);
						}
					}
				}
			}
		}
	}
}

void CompositionAnalysis::setSplitGroup(Composition composition, petri::iterator node, SplitGroup g) {
	std::vector<SplitGroup> *groups = splitGroupsIter(composition, node);
	if (groups == nullptr) {
		return;
	}

	auto pos = lower_bound(groups->begin(), groups->end(), g.split);
	if (pos != groups->end() and pos->split == g.split) {
		*pos = g;
	} else {
		groups->insert(pos, g);
	}
}

SplitGroup CompositionAnalysis::getSplitGroup(Composition composition, petri::iterator node, int split) const {
	vector<SplitGroup> groups = splitGroupsOf(composition, node);
	auto pos = lower_bound(groups.begin(), groups.end(), split);
	if (pos != groups.end() and pos->split == split) {
		return *pos;
	}
	return SplitGroup();
}

std::vector<SplitGroup> *CompositionAnalysis::splitGroupsIter(Composition composition, petri::iterator node) {
	if (node.index < 0) {
		return nullptr;
	}

	if (node.type == place::type) {
		return &places[node.index].splits[composition];
	}
	return &transitions[node.index].splits[composition];
}

std::vector<SplitGroup> CompositionAnalysis::splitGroupsOf(Composition composition, petri::iterator node) const {
	if (node.index < 0) {
		if (node.type == transition::type and composition == Composition::CHOICE and (int)reset.size() > 1) {
			return vector<SplitGroup>(1, SplitGroup(-1, (int)reset.size(), vector<int>(1, node.index)));
		}
		return vector<SplitGroup>();
	}

	if (node.type == place::type) {
		return places[node.index].splits[composition];
	}
	return transitions[node.index].splits[composition];
}

std::vector<SplitGroup> CompositionAnalysis::splitGroupsOf(Composition composition, SplitGroup::Operation groupOperation, SplitGroup::Operation branchOperation, vector<petri::iterator> nodes) const {
	std::vector<SplitGroup> groups;
	if (nodes.empty()) {
		return groups;
	}
	groups = splitGroupsOf(composition, nodes[0]);
	for (int i = 1; i < (int)nodes.size(); i++) {
		groups = petri::merge(groupOperation, branchOperation, groups, splitGroupsOf(composition, nodes[i]));
	}
	return groups;
}

// a is sometimes in choice with b if firing a does not imply a firing on b
// a is always in choice with b if firing a implies b will not fire
// a and b are sometimes in bidirectional choice if firing a does not imply a
// firing on b **or** visa-versa.
bool CompositionAnalysis::isExcludes(petri::iterator a, petri::iterator b, bool always) const {
	if (a == b) {
		return true;
	}

	auto ac = splitGroupsOf(Composition::CHOICE, a);
	auto bc = splitGroupsOf(Composition::CHOICE, b);

	return compare(SplitGroup::NEGATIVE_DIFFERENCE, SplitGroup::DIFFERENCE, ac, bc)
		and (not always or not isImplies(a, b, false));
}


// a is sometimes in choice with b if firing a does not imply a firing on b
// a is always in choice with b if firing a implies b will not fire
// a and b are sometimes in bidirectional choice if firing a does not imply a
// firing on b **or** visa-versa.
bool CompositionAnalysis::isImplies(petri::iterator a, petri::iterator b, bool always) const {
	if (a == b) {
		return true;
	}

	auto ac = splitGroupsOf(Composition::CHOICE, a);
	auto bc = splitGroupsOf(Composition::CHOICE, b);

	return not compare(SplitGroup::INTERSECT, SplitGroup::SYMMETRIC_DIFFERENCE, ac, bc)
		and (not always or not isExcludes(a, b, false));
}

// a is sometimes in choice with b if firing a does not imply a firing on b
// a is always in choice with b if firing a implies b will not fire
// a and b are sometimes in bidirectional choice if firing a does not imply a
// firing on b **or** visa-versa.
bool CompositionAnalysis::isChoice(petri::iterator a, petri::iterator b, bool always) const {
	if (a == b) {
		return false;
	}

	auto ac = splitGroupsOf(Composition::CHOICE, a);
	auto bc = splitGroupsOf(Composition::CHOICE, b);
	return (not always and compare(SplitGroup::INTERSECT, SplitGroup::NOT_EQUAL, ac, bc))
		or (always and compare(SplitGroup::INTERSECT, SplitGroup::SYMMETRIC_DIFFERENCE, ac, bc)
			and not isParallel(a, b, false));
}

// a is sometimes in parallel if there exists a state with both a and b.
// This does not imply that all states with one also have the other. This
// relationship is bidirectional.
bool CompositionAnalysis::isParallel(petri::iterator a, petri::iterator b, bool always) const {
	if (a == b) {
		return false;
	}

	auto ap = splitGroupsOf(Composition::PARALLEL, a);
	auto bp = splitGroupsOf(Composition::PARALLEL, b);
	return compare(SplitGroup::INTERSECT, SplitGroup::SYMMETRIC_DIFFERENCE, ap, bp)
		 and (not always or not isChoice(a, b, false));
}

bool CompositionAnalysis::isSequence(petri::iterator a, petri::iterator b, bool always) const {
	if (a == b) {
		return false;
	}

	auto ap = splitGroupsOf(Composition::PARALLEL, a);
	auto bp = splitGroupsOf(Composition::PARALLEL, b);
	auto ac = splitGroupsOf(Composition::CHOICE, a);
	auto bc = splitGroupsOf(Composition::CHOICE, b);

	return compare(SplitGroup::INTERSECT, SplitGroup::SUBSET_EQUAL, ap, bp)
		and compare(SplitGroup::INTERSECT, SplitGroup::SUBSET_EQUAL, ac, bc)
		and (not always or not isChoice(a, b, false));
}

bool CompositionAnalysis::is(Composition composition, petri::iterator a, petri::iterator b, bool always, bool bidir) const {
	if (composition == Composition::SEQUENCE) {
		return isSequence(a, b, always) and (not bidir or isSequence(b, a, always));
	} else if (composition == Composition::CHOICE) {
		return isChoice(a, b, always);
	} else if (composition == Composition::IMPLIES) {
		return isImplies(a, b, always) and (not bidir or isImplies(b, a, always));
	} else if (composition == Composition::EXCLUDES) {
		return isExcludes(a, b, always) or (bidir and isExcludes(b, a, always));
	}
	return isParallel(a, b, always);
}

// This assumes that a and b represent partial states. IE, there exists a set
// of states which each contain all nodes in a and a set of states which each
// contain all nodes in b.
bool CompositionAnalysis::is(Composition composition, petri::region a, petri::region b, bool always, bool bidir) const {
	// sometimes composed in parallel? - Is there a shared parallel split with
	// mutually exclusive branches in the group-intersected, branch-unioned
	// parallel split groups of the nodes of each partial that aren't in the other?

	// sometimes composed in choice? - Is there a shared conditional split
	// with exlusive branches in the group-unioned branch-intersected
	// conditional split groups of the nodes of each partial?

	// always composed in parallel? - sometimes composed in parallel and not
	// sometimes composed in choice

	// always composed in choice? - sometimes composed in choice and not
	// sometimes composed in parallel

	// e. sometimes composed in sequence? - sequencing direction is meaningless
	// most of the time since all processes are cycles. It's even difficult to
	// think about which direction crosses reset because the reset state could
	// be on a separate conditional branch. A and B are sequenced if for both
	// parallel and conditional split groups, branches in A are a superset of
	// the branches in B for all shared groups (or visa versa) of the nodes of
	// each partial that aren't in the other.

	// f. always composed in sequence? - sometimes composed in sequence and not
	// sometimes composed in choice
	//   Is it possible to have nodes composed in sequence sometimes and
	//   parallel others? If so, then also not composed in parallel sometimes.

	if (composition == Composition::PARALLEL or composition == Composition::IMPLIES) {
		for (auto i = a.begin(); i != a.end(); i++) {
			for (auto j = b.begin(); j != b.end(); j++) {
				if (*i != *j and not is(composition, *i, *j, always, bidir)) {
					return false;
				}
			}
		}
		return true;
	} else if (composition == Composition::CHOICE or composition == Composition::SEQUENCE or composition == Composition::EXCLUDES) {
		for (auto i = a.begin(); i != a.end(); i++) {
			for (auto j = b.begin(); j != b.end(); j++) {
				if (*i != *j and is(composition, *i, *j, always, bidir)) {
					return true;
				}
			}
		}
		return false;
	}
	return false;



	/*if (always) {
		if (composition == Composition::SEQUENCE) {
			return is(sequence, a, b, false) and not is(choice, a, b, false);
		}
		return is(composition, a, b, false) and not is(1-composition, a, b, false);
	}

	sort(a.begin(), a.end());
	a.erase(unique(a.begin(), a.end()), a.end());
	sort(b.begin(), b.end());
	b.erase(unique(b.begin(), b.end()), b.end());
	vector_symmetric_complement(a, b);

	if (a.empty() or b.empty()) {
		return false;
	}

	if (composition == Composition::SEQUENCE) {
		cout << "split a: " << ::to_string(splitGroupsOf(Composition::PARALLEL, SplitGroup::INTERSECT, SplitGroup::UNION, a)) << endl;
		cout << "split b: " << ::to_string(splitGroupsOf(Composition::PARALLEL, SplitGroup::INTERSECT, SplitGroup::UNION, b)) << endl;
		cout << "parallel: " << compare(SplitGroup::INTERSECT, SplitGroup::SUBSET_EQUAL,
				splitGroupsOf(Composition::PARALLEL, SplitGroup::INTERSECT, SplitGroup::UNION, a),
				splitGroupsOf(Composition::PARALLEL, SplitGroup::INTERSECT, SplitGroup::UNION, b)) << endl;
		cout << "split a: " << ::to_string(splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, a)) << endl;
		cout << "split b: " << ::to_string(splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, b)) << endl;
		cout << "choice: " << compare(SplitGroup::INTERSECT, SplitGroup::SUBSET_EQUAL,
				splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, a),
				splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, b)) << endl;
		return (compare(SplitGroup::INTERSECT, SplitGroup::SUBSET_EQUAL,
				splitGroupsOf(Composition::PARALLEL, SplitGroup::INTERSECT, SplitGroup::UNION, a),
				splitGroupsOf(Composition::PARALLEL, SplitGroup::INTERSECT, SplitGroup::UNION, b))
			and compare(SplitGroup::INTERSECT, SplitGroup::SUBSET_EQUAL,
				splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, a),
				splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, b)));
	}

	vector<SplitGroup> Ga, Gb;
	if (composition == Composition::PARALLEL) {
		Ga = splitGroupsOf(Composition::PARALLEL, SplitGroup::INTERSECT, SplitGroup::UNION, a);
		Gb = splitGroupsOf(Composition::PARALLEL, SplitGroup::INTERSECT, SplitGroup::UNION, b);
	} else {
		Ga = splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, a);
		Gb = splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, b);
	}

	return compare(SplitGroup::INTERSECT, SplitGroup::SYMMETRIC_DIFFERENCE, Ga, Gb);*/
}

vector<SplitGroup> CompositionAnalysis::invert(const Adjacency &g, Composition composition, std::vector<SplitGroup> groups) const {
	int splitType = (composition == Composition::CHOICE ? place::type : transition::type);
	for (auto &group : groups) {
		vector<petri::iterator> n = g.next(petri::iterator(splitType, group.split));
		vector<int> branches;
		for (int j = 0; j < (int)n.size(); j++) {
			if (find(group.branch.begin(), group.branch.end(), n[j].index) == group.branch.end()) {
				branches.push_back(n[j].index);
			}
		}
		group.branch.swap(branches);
		branches.clear();
	}
	return groups;
}

bool CompositionAnalysis::crossesReset(vector<petri::iterator> pos) const {
	bool beforeReset = false;
	bool afterReset = false;

	for (auto i : pos) {
		if (i.type == transition::type) {
			bool found = false;
			for (const auto &group : transitions[i.index].splits[Composition::PARALLEL]) {
				if (group.split < 0) {
					found = true;
					break;
				}
			}
			beforeReset = beforeReset or not found;
			afterReset = afterReset or found;
		} else {
			bool found = false;
			for (const auto &group : places[i.index].splits[Composition::PARALLEL]) {
				if (group.split < 0) {
					found = true;
					for (int branch : group.branch) {
						if (branch == i.index) {
							beforeReset = true;
						} else {
							afterReset = true;
						}
					}
				}
			}
			beforeReset = beforeReset or not found;
		}
	}
	return beforeReset and afterReset;
}

bound CompositionAnalysis::complete(Composition composition, bound nodes) const {
	// In this function, we are given conditional groups of parallel
	// nodes. In some cases, one group may entirely overlap another.
	// We need to add nodes to differentiate them in the petri net
	// when we insert transitions.

	// So, given A and B such that A is a subset of B, under which
	// choices that lead to a state in A don't lead to a state in B?
	// If a set of choices don't lead to a state in B & ~A, then they
	// also won't lead to a state in B.

	// If all choices that lead to states in A also lead to states in
	// B, then we can safely delete A from the list of groups.

	// 1. Find A and all groups [Bi] such that A is a subset of Bi
	for (int i = (int)nodes.size()-1; i >= 0; i--) {
		// 2. Find the conditional split groups of A union groups, intersect branches.
		vector<SplitGroup> A_groups = splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, nodes[i].nodes);
		vector<SplitGroup> B_groups;

		cout << "starting search " << ::to_string(nodes[i]) << " " << ::to_string(A_groups) << endl;
		for (int j = 0; j < (int)nodes.size(); j++) {
			cout << "checking i=" << i << ":" << ::to_string(nodes[i]) << " and j=" << j << ":" << ::to_string(nodes[j]) << endl;
			if (i != j and vector_is_subset_of(nodes[i].nodes, nodes[j].nodes)) {
				cout << "found subset" << endl;
				// 3. Find the conditional split groups of Bi & ~A union groups, intersect branches.
				vector<petri::iterator> Bj = vector_difference(nodes[j].nodes, nodes[i].nodes);
				cout << "j-i" << ::to_string(Bj) << endl;

				vector<SplitGroup> Bj_groups = splitGroupsOf(Composition::CHOICE, SplitGroup::UNION, SplitGroup::INTERSECT, Bj);
				cout << "groups:" << ::to_string(Bj_groups) << endl;

				// 4. Find the conditional branches that B belongs to that A does
				// not. Merge, intersect groups, subtract branches A-Bi for each Bi
				Bj_groups = petri::merge(SplitGroup::INTERSECT, SplitGroup::DIFFERENCE, A_groups, Bj_groups);
				cout << "after intersect:" << ::to_string(Bj_groups) << endl;

				// 5. merge, union groups, union branches across all [Bi]
				B_groups = petri::merge(SplitGroup::UNION, SplitGroup::UNION, B_groups, Bj_groups);
				cout << "after union:" << ::to_string(B_groups) << endl;
			}
		}
		cout << "done search:" << ::to_string(B_groups) << endl;
		if (B_groups.empty()) {
			continue;
		}

		// 5. Add these transitions to A
		region A = nodes[i];
		for (auto group = B_groups.begin(); group != B_groups.end(); group++) {
			for (auto branch = group->branch.begin(); branch != group->branch.end(); branch++) {
				A.push_back(petri::iterator(transition::type, *branch));
			}
		}
		A.sort();

		/*// 6. select conditional groups of parallel transitions from A (not sometimes conditional)
		vector<vector<petri::iterator> > groups = select(parallel, A, false, true);

		// 7. delete groups that don't include A
		for (auto group = groups.begin(); group != groups.end(); group++) {
			if (vector_is_subset_of(nodes[i], *group)) {
				nodes.push_back(*group);
			}
		}*/
		nodes.push_back(A);

		// TODO(edward.bingham) Do I need to group(parallel, groups, false,
		// false) and then recurse looking for subsets? How does this interact
		// with the conditional split bug? Are there other things that I'm
		// missing here? How do I formally prove that this process creates a full
		// graph cut of the behavior?

		nodes.regions.erase(nodes.begin() + i);
	}

	return nodes;
}

// Find all partial state pairs (for each node in v0 and v1 respectively) that are ordered (not in parallel).
vector<array<petri::bound, 2> > CompositionAnalysis::deinterfere_choice(vector<petri::iterator> v0, vector<petri::iterator> v1) const {
	vector<array<petri::bound, 2> > stack;
	vector<array<petri::bound, 2> > next;

	stack.resize(1);
	for (int i = 0; i < (int)v0.size(); i++) {
		stack.back()[0].push_back({v0[i]});
	}
	for (int i = 0; i < (int)v1.size(); i++) {
		stack.back()[1].push_back({v1[i]});
	}
	if (stack.back()[0].empty() or stack.back()[1].empty()) {
		stack.pop_back();
	}
	for (int i = 0; i < (int)v0.size(); i++) {
		while (not stack.empty()) {
			auto curr = stack.back();
			stack.pop_back();

			for (int j = 0; j < (int)curr[1].size(); j++) {
				auto n = deinterfere(curr[0][i], curr[1][j]);
				for (int k = 0; k < (int)n.size(); k++) {
					if (not n[k][0].empty() and not n[k][1].empty()) {
						next.push_back(curr);
						next.back()[0][i] = n[k][0];
						next.back()[1][j] = n[k][1];
					}
				}
			}
		}
		stack = next;
		next.clear();
	}

	return stack;
}

// Identifies configurations where node groups can be sequentially ordered.
//
// This function finds ways to organize potentially parallel nodes into
// sequentially orderable groups by identifying additional nodes that can 
// resolve parallelism. It's essential for transformations that need to
// convert concurrent behaviors into sequential ones.
//
// The algorithm analyzes relationship patterns between node sets, particularly
// looking for parallel relationships that can be broken by adding specific nodes.
// It systematically explores the graph to find nodes that, when added to either 
// group, would make the two groups non-parallel, enabling sequential execution.
//
// This operation is particularly important for state-variable insertion.
//
// @param v0 First vector of nodes to analyze
// @param v1 Second vector of nodes to analyze
// @return Vector of possible solutions, each containing two vectors of nodes that can be ordered
vector<array<petri::region, 2> > CompositionAnalysis::deinterfere(petri::region v0, petri::region v1) const {
	sort(v0.begin(), v0.end());
	sort(v1.begin(), v1.end());
	vector<petri::iterator> v0p, v1p;
	for (int j = 0; j < 2; j++) {
		for (auto i = begin(j); i != end(j); i++) {
			if (not isValid(i)) continue;

			if (find(v1.begin(), v1.end(), i) == v1.end() and is(Composition::PARALLEL, {i}, v0)) {
				v0p.push_back(i);
			}
			if (find(v0.begin(), v0.end(), i) == v0.end() and is(Composition::PARALLEL, {i}, v1)) {
				v1p.push_back(i);
			}
		}
	}

	vector<array<petri::region, 2> > result;
	if (vector_intersects(v0.flat(), v1.flat())) {
		return result;
	}

	if (not is(Composition::PARALLEL, v0, v1)) {
		result.push_back({v0, v1});
		return result;
	}

	for (auto i = v0p.begin(); i != v0p.end(); i++) {
		if (not is(Composition::PARALLEL, {*i}, v1)) {
			result.push_back({v0, v1});
			result.back()[0].push_back(*i);
		}
	}

	for (auto i = v1p.begin(); i != v1p.end(); i++) {
		if (not is(Composition::PARALLEL, {*i}, v0)) {
			result.push_back({v0, v1});
			result.back()[1].push_back(*i);
		}
	}

	for (auto i = v0p.begin(); i != v0p.end(); i++) {
		for (auto j = v1p.begin(); j != v1p.end(); j++) {
			if (*i != *j and not is(Composition::PARALLEL, *i, *j)) {
				result.push_back({v0, v1});
				result.back()[0].push_back(*i);
				result.back()[1].push_back(*j);
			}
		}
	}

	return result;
}

// select groups nodes into maximal cliques based on specific relationship types
//
// Nodes can be simultaneously composed in both parallel and conditional.
// This function selects nodes into groups based upon a composition operator
// (ex. conditional groups of parallel nodes for the "parallel" composition).
// If the "strict" flag is set, then this function not only separates nodes
// that are not composed as desired, but also separates nodes that are
// compared as desired and also composed as not desired. For example, strict
// will also separate nodes that are simultaneously composed in parallel and
// conditional.
//
// This function identifies sets of nodes that share specific relationships (like choice,
// parallel, sequence, etc.) using the Bron-Kerbosch algorithm to find maximal cliques.
// It's a fundamental analysis tool that supports higher-level understanding of the
// Petri net's behavioral patterns and structural properties.
//
// The function operates by constructing an implicit graph where nodes that share the
// specified relationship have edges between them, then finding all maximal cliques
// in this graph. This provides insight into groups of nodes that have consistent
// behavioral relationships.
//
// The algorithm handles the NP-complete maximal clique problem using an iterative
// frame-based approach that efficiently identifies all relationships matching the
// specified criteria, with options for strict or relaxed relationship requirements.
//
// @param composition The relationship type to analyze (Composition::PARALLEL, choice, implies, excludes)
// @param nodes The set of nodes to analyze for relationships
// @param always If true, requires consistent (always) relationships; if false, allows occasional relationships
// @param invert If true, inverts the relationship criteria, finding opposite relationships
// @return A vector of vectors, where each inner vector contains a maximal clique of related nodes
bound CompositionAnalysis::select(Composition composition, vector<petri::iterator> nodes, bool always, bool invert) const {
	// ~always & ~invert - separate nodes that aren't sometimes composed as requested
	// ~always &  invert - separate nodes that are sometimes composed as the opposite of requested
	//  always & ~invert - separate nodes that aren't always composed as requested.
	//                     For example if parallel requested, then this breaks sequence
	//                     and choice relations.
	//  always &  invert - separate nodes that are always composed as the opposite of requested

	bound result;

	// This is the problem of identifying all maximal cliques in the
	// graph constructed using the nodes in "from" as vertices and
	// creating edges between each pair of parallel nodes. This is an
	// NP-complete problem and we are solving it using the Bron–Kerbosch
	// algorithm.
	struct BronKerboschFrame {
		vector<petri::iterator> R, P, X;
	};

	Composition opposite = petri::invert(composition);

	vector<BronKerboschFrame> frames;
	frames.push_back(BronKerboschFrame());
	frames.back().P = nodes;

	while (not frames.empty()) {
		auto frame = frames.back();
		frames.pop_back();

		if (frame.P.empty() and frame.X.empty()) {
			// Then we've found a maximal clique
			sort(frame.R.begin(), frame.R.end());
			result.push_back(region::from_nodes(frame.R));
		} else {
			// Otherwise, we need to recurse
			while (not frame.P.empty()) {
				frames.push_back(frame);
				frames.back().R.push_back(frame.P.back());
				for (int i = (int)frames.back().P.size()-1; i >= 0; i--) {
					if (frames.back().P[i] == frame.P.back()
						or (not invert and not is(composition, frames.back().P[i], frame.P.back(), always, true))
						or (invert and is(opposite, frames.back().P[i], frame.P.back(), always, true))) {
						frames.back().P.erase(frames.back().P.begin() + i);
					}
				}
				for (int i = (int)frames.back().X.size()-1; i >= 0; i--) {
					if (frames.back().X[i] == frame.P.back()
						or (not invert and not is(composition, frames.back().X[i], frame.P.back(), always, true))
						or (invert and is(opposite, frames.back().X[i], frame.P.back(), always, true))) {
						frames.back().X.erase(frames.back().X.begin() + i);
					}
				}

				frame.X.push_back(frame.P.back());
				frame.P.pop_back();
			}
		}
	}

	return result;
}

// Takes a strict selection of nodes (see graph::select() ) and regroups them into all non-strict selections
bound CompositionAnalysis::group(Composition composition, bound nodes, bool always, bool invert) const {
	// ~always & ~invert - group nodes that are sometimes composed as requested
	// ~always &  invert - group nodes that aren't sometimes composed as the opposite of requested
	//  always & ~invert - group nodes that are always composed as requested.
	//  always &  invert - group nodes that aren't always composed as the opposite of requested

	struct BronKerboschFrame {
		vector<int> R, P, X;
	};

	Composition opposite = petri::invert(composition);

	vector<BronKerboschFrame> frames;
	frames.push_back(BronKerboschFrame());
	for (int i = 0; i < (int)nodes.size(); i++) {
		frames.back().P.push_back(i);
	}

	while (not frames.empty()) {
		auto frame = frames.back();
		frames.pop_back();

		if (frame.P.empty() and frame.X.empty()) {
			// Then we've found a maximal clique
			if ((int)frame.R.size() > 1) {
				nodes.push_back(region());
				for (auto i = frame.R.begin(); i != frame.R.end(); i++) {
					nodes.back().append(nodes[*i]);
				}
				sort(nodes.back().begin(), nodes.back().end());
			}
		} else {
			// Otherwise, we need to recurse
			while (not frame.P.empty()) {
				frames.push_back(frame);
				frames.back().R.push_back(frame.P.back());
				for (int i = (int)frames.back().P.size()-1; i >= 0; i--) {
					if (frames.back().P[i] == frame.P.back()
						or (not invert and not is(composition, nodes[frames.back().P[i]], nodes[frame.P.back()], always))
						or (invert and is(opposite, nodes[frames.back().P[i]], nodes[frame.P.back()], always))) {
						frames.back().P.erase(frames.back().P.begin() + i);
					}
				}
				for (int i = (int)frames.back().X.size()-1; i >= 0; i--) {
					if (frames.back().X[i] == frame.P.back()
						or (not invert and not is(composition, nodes[frames.back().X[i]], nodes[frame.P.back()], always))
						or (invert and is(opposite, nodes[frames.back().X[i]], nodes[frame.P.back()], always))) {
						frames.back().X.erase(frames.back().X.begin() + i);
					}
				}

				frame.X.push_back(frame.P.back());
				frame.P.pop_back();
			}
		}
	}

	return nodes;
}

bound CompositionAnalysis::partials(Composition composition, petri::region nodes, vector<petri::iterator> other) const {
	nodes.sort();
	if (other.empty()) {
		for (auto i = begin(place::type); i != end(place::type); i++) {
			if (isValid(i) and is(composition, {i}, nodes)) {
				other.push_back(i);
			}
		}
		for (auto i = begin(transition::type); i != end(transition::type); i++) {
			if (isValid(i) and is(composition, {i}, nodes)) {
				other.push_back(i);
			}
		}
	}

	// Given the set of nodes in "other" and the set of nodes in "nodes", we
	// need to find all cliques (maximal or not) in the graph created by
	// the requested composition relations.
	bound result;
	list<pair<petri::region, vector<petri::iterator> > > queue;
	queue.push_back({nodes, other});
	while (not queue.empty()) {
		auto curr = queue.front();
		queue.pop_front();

		auto k = lower_bound(result.begin(), result.end(), curr.first);
		if (k == result.end() or *k != curr.first) {
			result.insert(k, curr.first);
			for (auto i = curr.second.begin(); i != curr.second.end(); i++) {
				if (is(composition, {*i}, curr.first)) {
					queue.push_back(curr);
					auto j = lower_bound(queue.back().first.begin(), queue.back().first.end(), *i);
					queue.back().first.insert(j, *i);
					queue.back().second.erase(queue.back().second.begin() + (i-curr.second.begin()));
				}
			}
		}
	}

	return result;
}

/*bool CompositionAnalysis::isRedundantTo(petri::iterator p0, petri::iterator p1) const {
	if (p0 == p1 or not is(Composition::PARALLEL, p0, p1)) {
		return false;
	}

	vector<petri::iterator> n = neighbors(p0);
	for (auto ni = n.begin(); ni != n.end(); ni++) {
		if (is(Composition::PARALLEL, *ni, p1)) {
			return false;
		}
	}

	bool p1IsReset = false;
	for (const auto &i : reset) {
		for (const auto &j : i) {
			if (p0 == j) {
				return true;
			}
			p1IsReset = p1IsReset or p1 == j;
		}
	}
	if (p1IsReset) {
		return false;
	}

	for (const auto &i : reset) {
		for (const auto &j : i) {
			if (j != p0 and j != p1 and is(Composition::PARALLEL, p0, j) and is(Composition::SEQUENCE, p1, j)) {
				return false;
			}
		}
	}

	return true;
}

bool CompositionAnalysis::isRedundantTo(petri::iterator p0, vector<petri::iterator> p1) const {
	for (auto p1i : p1) {
		if (isRedundantTo(p0, p1i)) {
			return true;
		}
	}
	return false;
}

bool CompositionAnalysis::isRedundant(petri::iterator p0) const {
	for (auto i = begin(place::type); i != end(place::type); i++) {
		if (not isValid(i)) continue;

		if (isRedundantTo(p0, i)) {
			//cout << p0 << " is redundant to " << i << endl;
			return true;
		}
	}
	return false;
}

vector<petri::iterator> CompositionAnalysis::addRedundant(vector<petri::iterator> p) const {
	for (auto i = begin(place::type); i != end(place::type); i++) {
		if (isValid(i) and isRedundantTo(i, p)) {
			p.push_back(i);
		}
	}
	sort(p.begin(), p.end());
	p.erase(unique(p.begin(), p.end()), p.end());
	return p;
}*/

}

