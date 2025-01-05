#pragma once

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>

#include <array>
#include "state.h"
#include "iterator.h"
#include "node.h"

namespace petri
{

struct arc
{
	arc();
	arc(petri::iterator from, petri::iterator to);
	~arc();

	petri::iterator from;
	petri::iterator to;
};

bool operator<(arc a0, arc a1);
bool operator>(arc a0, arc a1);
bool operator<=(arc a0, arc a1);
bool operator>=(arc a0, arc a1);
bool operator==(arc a0, arc a1);
bool operator!=(arc a0, arc a1);

// Generic petri net graph representation.
template <class place, class transition, class token, class state>
struct graph
{
	mutable vector<int> node_distances;
	mutable bool node_distances_ready;

	mutable bool split_groups_ready;
	mutable bool merge_groups_ready[2];

	vector<place> places;
	vector<transition> transitions;
	// index by from.type
	array<vector<arc>, 2> arcs;
	vector<state> source, sink;
	vector<state> reset;

	graph()
	{
		reset_node_distances();
		split_groups_ready = false;
		split_groups_ready = false;
		merge_groups_ready[0] = false;
		merge_groups_ready[1] = false;
	}

	virtual ~graph()
	{

	}

	// Calculate the minimum number of arcs between any two nodes. This data
	// can be used to determine if one node is reachable from another or as
	// a way to guide logic minimization heuristics based on what is the
	// "most recent transition". The result is stored in node_distances in
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
	virtual void reset_node_distances() const {
		int nodes = (int)(places.size() + transitions.size());
		int offset = (int)places.size();
		node_distances.clear();
		node_distances.assign(nodes*nodes, std::numeric_limits<int>::min());
		for (int i = 0; i < (int)places.size(); i++) {
			node_distances[i*nodes + i] = 0;
		}
		for (int i = 0; i < (int)transitions.size(); i++) {
			node_distances[(offset + i)*nodes + (offset + i)] = 0;
		}
		node_distances_ready = false;
	}

	virtual void update_node_distances(petri::iterator pos) const {
		set<petri::iterator> seen;

		array<vector<vector<petri::iterator> >, 2> p;
		p[place::type].resize(places.size());
		p[transition::type].resize(transitions.size());
		for (int type = 0; type < 2; type++) {
			for (int i = 0; i < (int)arcs[type].size(); i++) {
				p[1-type][arcs[type][i].to.index].push_back(arcs[type][i].from);
			}
		}

		int nodes = (int)(places.size() + transitions.size());
		int offset = (int)places.size();
		int posIdx = offset*pos.type + pos.index;

		vector<petri::iterator> stack;
		stack.push_back(pos);
		seen.insert(pos);
		while (not stack.empty()) {
			petri::iterator curr = stack.back();
			stack.pop_back();

			int toIdx = offset*curr.type + curr.index;
			for (auto i = p[curr.type][curr.index].begin(); i != p[curr.type][curr.index].end(); i++) {
				int fromIdx = offset*i->type + i->index;
				if (seen.insert(*i).second) {
					node_distances[posIdx*nodes + fromIdx] = max(node_distances[posIdx*nodes + fromIdx], node_distances[posIdx*nodes + toIdx] + 1);
					stack.push_back(*i);
				}
			}
		}
	}

	virtual void update_node_distances() const {
		// TODO(edward.bingham) This needs to take the max of the node distances
		// for all of the pre-set nodes. However, doing so triggers an infinite
		// loop around loops with splits and merges

		// clear the current set of distances
		if (not node_distances_ready) {
			reset_node_distances();
		}

		for (int i = 0; i < (int)places.size(); i++) {
			update_node_distances(petri::iterator(place::type, i));
		}

		for (int i = 0; i < (int)transitions.size(); i++) {
			update_node_distances(petri::iterator(transition::type, i));
		}

		node_distances_ready = true;
	}

	virtual int &distance(petri::iterator from, petri::iterator to, bool update = true) const {
		if (update and not node_distances_ready) {
			update_node_distances();
		}

		int nodes = places.size() + transitions.size();
		int fromIdx = places.size()*from.type + from.index;
		int toIdx = places.size()*to.type + to.index;
		int idx = toIdx*nodes + fromIdx;
		if (idx >= (int)node_distances.size()) {
			node_distances.resize(nodes*nodes, std::numeric_limits<int>::min());
		}
		return node_distances[idx];
	}

	virtual int distance(vector<petri::iterator> from, vector<petri::iterator> to) const {
		int result = std::numeric_limits<int>::min();
		for (int i = 0; i < (int)from.size(); i++) {
			for (int j = 0; j < (int)to.size(); j++) {
				result = max(result, distance(from[i], to[j]));
			}
		}
		return result;
	}

	virtual bool is_reachable(petri::iterator from, petri::iterator to) const {
		return (distance(from, to) >= 0);
	}

	virtual bool is_reachable(vector<petri::iterator> from, vector<petri::iterator> to) const {
		for (auto i = from.begin(); i != from.end(); i++) {
			for (auto j = to.begin(); j != to.end(); j++) {
				if (distance(*i, *j) >= 0) {
					return true;
				}
			}
		}
		return false;
	}

	virtual bool precedes(petri::iterator from, petri::iterator to, set<petri::iterator> excl=set<petri::iterator>()) const {
		array<vector<vector<petri::iterator> >, 2> n;
		n[place::type].resize(places.size());
		n[transition::type].resize(transitions.size());
		for (int type = 0; type < 2; type++) {
			for (int i = 0; i < (int)arcs[type].size(); i++) {
				n[type][arcs[type][i].from.index].push_back(arcs[type][i].to);
			}
		}

		set<petri::iterator> seen = excl;
		seen.insert(from);

		vector<petri::iterator> stack;
		stack.push_back(from);
		while (not stack.empty()) {
			petri::iterator curr = stack.back();
			stack.pop_back();

			for (auto i = n[curr.type][curr.index].begin(); i != n[curr.type][curr.index].end(); i++) {
				if (*i == to) {
					return true;
				}
				if (seen.insert(*i).second) {
					stack.push_back(*i);
				}
			}
		}
		return false;
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
	virtual bool split_is_covered(petri::iterator p, vector<petri::iterator> n) const {
		if (n.size() <= 1) {
			return true;
		}

		vector<split_group> groups = split_groups_of(choice, p, false);
		for (auto group = groups.begin(); group != groups.end(); group++) {
			bool found = true;
			vector<vector<petri::iterator> > clusters;
			clusters.resize(group->count);
			for (auto j = n.begin(); j != n.end() and found; j++) {
				vector<split_group> subs = split_groups_of(choice, *j, false);
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
				if (cluster->size() == n.size() or not split_is_covered(p, *cluster)) {
					found = false;
				}
			}

			if (found) {
				return true;
			}
		}
		
		return false;
	}

	virtual void compute_split_group(int composition, int split, vector<petri::iterator> init) const {
		if (init.size() <= 1) {
			// there is no split here
			return;
		}

		// precache previous places
		array<vector<vector<petri::iterator> >, 2> p, n;
		p[place::type].resize(places.size());
		p[transition::type].resize(transitions.size());
		n[place::type].resize(places.size());
		n[transition::type].resize(transitions.size());
		for (int type = 0; type < 2; type++) {
			for (int i = 0; i < (int)arcs[type].size(); i++) {
				n[type][arcs[type][i].from.index].push_back(arcs[type][i].to);
				p[1-type][arcs[type][i].to.index].push_back(arcs[type][i].from);
			}
		}

		petri::iterator splitNode(
				composition == parallel ? transition::type : place::type,
				split);

		vector<petri::iterator> frontier;
		// TODO(edward.bingham) maybe want to use a "path" here
		set<petri::iterator> seen;
		seen.insert(init.begin(), init.end());
		for (auto i = init.begin(); i != init.end(); i++) {
			if (i->index >= 0) {
				set_split_group(composition, *i, split_group(split, init.size(), {i->index}));
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
		map<petri::iterator, split_group> enabled;
		map<petri::iterator, int> order;
		int orderLevel = 0;
		do {
			enabled.clear();
			//cout << "step " << ::to_string(seen) << endl;

			map<petri::iterator, split_group> blocked;
			for (int type = 0; type < 2; type++) {
				for (petri::iterator to = begin(type); to != end(type); to++) {
					bool isDisabled = false;
					bool isEnabled = false;
					split_group toSplit = get_split_group(composition, to, split);
					// Ensure that we haven't encountered this split yet.
					if (toSplit.split != split) {
						toSplit.split = split;
						toSplit.count = init.size();

						// Check to make sure we've visited all of the input nodes and
						// derive the split group for this node.
						for (auto from = p[to.type][to.index].begin(); from != p[to.type][to.index].end(); from++) {
							split_group fromSplit = get_split_group(composition, *from, split);
							if (fromSplit.split != split) {
								isDisabled = true;
								if (type == transition::type
									and (composition == choice
										or (composition == parallel
											and compare(split_group::NEGATIVE_DIFFERENCE, split_group::DIFFERENCE,
												split_groups_of(choice, splitNode, false),
												split_groups_of(choice, *from, false))))) {
									toSplit.branch.clear();
									break;
								}
								continue;
							}
							isEnabled = true;

							if (from == p[to.type][to.index].begin()
								or not (to.type == transition::type and composition == choice)) {
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
					bool AtoB = precedes(i->first, first->first, seen);
					bool BtoA = precedes(first->first, i->first, seen);
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
				if (n[i->first.type][i->first.index].empty()) {
					//cout << "ready frontier end adding " << i->first << endl;
					frontier.push_back(i->first);
				}
				for (auto j = n[i->first.type][i->first.index].begin(); j != n[i->first.type][i->first.index].end(); j++) {
					if (get_split_group(composition, *j, split).split == split) {
						//cout << "ready frontier loop adding " << i->first << endl;
						frontier.push_back(i->first);
					}
				}
			}

			for (auto i = enabled.begin(); i != enabled.end(); i++) {
				// replace the set of groups that exist at this location
				set_split_group(composition, i->first, i->second);
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
			vector<split_group> *cgroups = split_groups_iter(composition, curr);
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
			for (auto i = p[curr.type][curr.index].begin(); i != p[curr.type][curr.index].end() and closed; i++) {
				vector<split_group> groups = split_groups_of(composition, *i, false);
				for (auto j = groups.begin(); j != groups.end() and closed; j++) {
					closed = (j->split != split or j->branch.empty() or (int)j->branch.size() >= j->count);
				}
			}

			if (closed) {
				todo.insert(todo.end(), p[curr.type][curr.index].begin(), p[curr.type][curr.index].end());
				sort(todo.begin(), todo.end());
				todo.erase(unique(todo.begin(), todo.end()), todo.end());
			}
		}

		//cout << "done backtrack" << endl;
		//print();
		//cout << endl << endl;
	}

	virtual void compute_split_groups() const {
		// DESIGN(edward.bingham) Choice must go first, because we use that to determine whether we're dealing with non-properly nested parallelism or shared conditional parallel branches. It just so happens that "choice" = 0 and "parallel" = 1
		for (int composition = 0; composition < 2; composition++) {
			// clear previous executions of this function and cache previous places and
			// transitions as an optimization.
			for (int i = 0; i < (int)places.size(); i++) {
				places[i].splits[composition].clear();
			}
			for (int i = 0; i < (int)transitions.size(); i++) {
				transitions[i].splits[composition].clear();
			}

			// each place belongs to some set of parallel splits (init[place])
			if (composition == parallel) {
				// add parallel splits from reset states
				if (not reset.empty()) {
					for (int i = 0; i < (int)reset.size(); i++) {
						vector<petri::iterator> branches;
						for (auto j = reset[i].tokens.begin(); j != reset[i].tokens.end(); j++) {
							branches.push_back(petri::iterator(place::type, j->index));
						}
						compute_split_group(composition, -i-1, branches);
					}
				}
			} else if (composition == choice) {
				if (reset.size() > 1) {
					vector<petri::iterator> branches;
					for (int i = 0; i < (int)reset.size(); i++) {
						branches.push_back(petri::iterator(transition::type, -i-1));
					}
					compute_split_group(composition, -1, branches);
				}
			}

			// A the moment, parallel == transition::type and choice == place::type,
			// but that's not necessarily guaranteed.
			int split_type = (composition == parallel ? transition::type : place::type);

			// add splits from graph structure at the first branch nodes after each split
			for (petri::iterator i = begin(split_type); i != end(split_type); i++) {
				compute_split_group(composition, i.index, next(i));
			}

			// See split_is_covered() for documentation. Remove "covered" conditional splits.
			if (composition == choice) {
				set<int> covered;
				for (petri::iterator i = begin(split_type); i != end(split_type); i++) {
					if (split_is_covered(i, next(i))) {
						covered.insert(i.index);
					}
				}

				for (int type = 0; type < 2; type++) {
					for (petri::iterator i = begin(type); i != end(type); i++) {
						vector<split_group> *groups = split_groups_iter(choice, i);
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
		split_groups_ready = true;
	}

	virtual void mark_modified()
	{
		node_distances_ready = false;
		split_groups_ready = false;
	}

	virtual int size(int type=-1) const {
		return type == -1 ? (int)(places.size()+transitions.size()) : (type == place::type ? (int)places.size() : (int)transitions.size());
	}

	virtual petri::iterator begin(int type) const {
		return petri::iterator(type, 0);
	}

	virtual petri::iterator end(int type) const {
		return petri::iterator(type, size(type));
	}

	virtual petri::iterator rbegin(int type) const {
		return petri::iterator(type, size(type)-1);
	}

	virtual petri::iterator rend(int type) const {
		return petri::iterator(type, -1);
	}

	virtual petri::iterator begin_arc(int type) const {
		return petri::iterator(type, 0);
	}

	virtual petri::iterator end_arc(int type) const {
		return petri::iterator(type, (int)arcs[type].size());
	}

	virtual petri::iterator rbegin_arc(int type) const {
		return petri::iterator(type, (int)arcs[type].size()-1);
	}

	virtual petri::iterator rend_arc(int type) const {
		return petri::iterator(type, -1);
	}

	virtual petri::iterator create_at(place p, int index)
	{
		mark_modified();
		if (index >= (int)places.size()) {
			places.resize(index+1);
		}
		places[index] = p;
		return petri::iterator(place::type, index);
	}

	virtual petri::iterator create_at(transition t, int index)
	{
		mark_modified();
		if (index >= (int)transitions.size()) {
			transitions.resize(index+1);
		}
		transitions[index] = t;
		return petri::iterator(transition::type, index);
	}

	virtual petri::iterator create(place p)
	{
		mark_modified();
		places.push_back(p);
		return petri::iterator(place::type, (int)places.size()-1);
	}

	virtual petri::iterator create(transition t)
	{
		mark_modified();
		transitions.push_back(t);
		return petri::iterator(transition::type, (int)transitions.size()-1);
	}

	virtual petri::iterator create(int n)
	{
		if (n == place::type)
			return create(place());
		else if (n == transition::type)
			return create(transition());
		else
			return petri::iterator();
	}

	virtual vector<petri::iterator> create(vector<place> p)
	{
		mark_modified();
		vector<petri::iterator> result;
		for (int i = 0; i < (int)p.size(); i++)
		{
			places.push_back(p[i]);
			result.push_back(petri::iterator(place::type, (int)places.size()-1));
		}
		return result;
	}

	virtual vector<petri::iterator> create(vector<transition> t)
	{
		mark_modified();
		vector<petri::iterator> result;
		for (int i = 0; i < (int)t.size(); i++)
		{
			transitions.push_back(t[i]);
			result.push_back(petri::iterator(transition::type, (int)transitions.size()-1));
		}
		return result;
	}

	virtual vector<petri::iterator> create(place p, int num)
	{
		mark_modified();
		vector<petri::iterator> result;
		for (int i = 0; i < num; i++)
		{
			places.push_back(p);
			result.push_back(petri::iterator(place::type, (int)places.size()-1));
		}
		return result;
	}

	virtual vector<petri::iterator> create(transition t, int num)
	{
		mark_modified();
		vector<petri::iterator> result;
		for (int i = 0; i < num; i++)
		{
			transitions.push_back(t);
			result.push_back(petri::iterator(transition::type, (int)transitions.size()-1));
		}
		return result;
	}

	virtual vector<petri::iterator> create(int n, int num)
	{
		if (n == place::type)
			return create(place(), num);
		else if (n == transition::type)
			return create(transition(), num);
		else
			return vector<petri::iterator>();
	}

	virtual pair<vector<petri::iterator>, vector<petri::iterator> > erase(petri::iterator n)
	{
		mark_modified();
		pair<vector<petri::iterator>, vector<petri::iterator> > result;
		for (int i = (int)arcs[n.type].size()-1; i >= 0; i--)
		{
			if (arcs[n.type][i].from.index == n.index)
			{
				result.second.push_back(arcs[n.type][i].to);
				arcs[n.type].erase(arcs[n.type].begin() + i);
			}
			else if (arcs[n.type][i].from.index > n.index)
				arcs[n.type][i].from.index--;
		}
		for (int i = (int)arcs[1-n.type].size()-1; i >= 0; i--)
		{
			if (arcs[1-n.type][i].to.index == n.index)
			{
				result.first.push_back(arcs[1-n.type][i].from);
				arcs[1-n.type].erase(arcs[1-n.type].begin() + i);
			}
			else if (arcs[1-n.type][i].to.index > n.index)
				arcs[1-n.type][i].to.index--;
		}

		if (n.type == place::type)
		{
			for (int j = 0; j < (int)source.size(); j++)
				for (int i = (int)source[j].tokens.size()-1; i >= 0; i--)
				{
					if (source[j].tokens[i].index == n.index)
						source[j].tokens.erase(source[j].tokens.begin() + i);
					else if (source[j].tokens[i].index > n.index)
						source[j].tokens[i].index--;
				}

			for (int j = 0; j < (int)reset.size(); j++)
				for (int i = (int)reset[j].tokens.size()-1; i >= 0; i--)
				{
					if (reset[j].tokens[i].index == n.index)
						reset[j].tokens.erase(reset[j].tokens.begin() + i);
					else if (reset[j].tokens[i].index > n.index)
						reset[j].tokens[i].index--;
				}

			for (int j = 0; j < (int)sink.size(); j++)
				for (int i = (int)sink[j].tokens.size()-1; i >= 0; i--)
				{
					if (sink[j].tokens[i].index == n.index)
						sink[j].tokens.erase(sink[j].tokens.begin() + i);
					else if (sink[j].tokens[i].index > n.index)
						sink[j].tokens[i].index--;
				}
		}

		if (n.type == place::type)
			places.erase(places.begin() + n.index);
		else if (n.type == transition::type)
			transitions.erase(transitions.begin() + n.index);

		return result;
	}

	static void erase(petri::iterator n, vector<petri::iterator> &iter_list)
	{
		for (int i = (int)iter_list.size()-1; i >= 0; i--)
		{
			if (iter_list[i] == n)
				iter_list.erase(iter_list.begin() + i);
			else if (iter_list[i].type == n.type && iter_list[i].index > n.index)
				iter_list[i].index--;
		}
	}

	static void erase(vector<petri::iterator> n, vector<petri::iterator> &iter_list)
	{
		sort(n.rbegin(), n.rend());
		for (int i = 0; i < (int)n.size(); i++)
			erase(n[i], iter_list);
	}

	static void erase(petri::iterator n, int type, vector<int> &iter_list)
	{
		if (n.type != type)
			return;

		for (int i = (int)iter_list.size()-1; i >= 0; i--)
		{
			if (iter_list[i] == n.index)
				iter_list.erase(iter_list.begin() + i);
			else if (iter_list[i] > n.index)
				iter_list[i]--;
		}
	}

	static void erase(petri::iterator n, state &s)
	{
		if (n.type != place::type)
			return;
			
		for (int i = (int)s.tokens.size()-1; i >= 0; i--)
		{
			if (s.tokens[i].index == n.index)
				s.tokens.erase(s.tokens.begin() + i);
			else if (s.tokens[i].index > n.index)
				s.tokens[i]--;
		}
	}

	static void erase(petri::iterator n, vector<state> &s)
	{
		if (n.type != place::type)
			return;
			
		for (int i = 0; i < (int)s.size(); i++)
			for (int j = (int)s[i].tokens.size()-1; j >= 0; j--)
			{
				if (s[i].tokens[j].index == n.index)
					s[i].tokens.erase(s[i].tokens.begin() + j);
				else if (s[i].tokens[j].index > n.index)
					s[i].tokens[j].index--;
			}
	}

	virtual void erase(vector<petri::iterator> n, bool rsorted = false)
	{
		if (!rsorted)
			sort(n.rbegin(), n.rend());

		for (int i = 0; i < (int)n.size(); i++)
			erase(n[i]);
	}

	virtual petri::iterator connect(petri::iterator from, petri::iterator to)
	{
		if (from.type == place::type && to.type == place::type)
		{
			petri::iterator mid = create(transition());
			arcs[from.type].push_back(arc(from, mid));
			arcs[mid.type].push_back(arc(mid, to));
		}
		else if (from.type == transition::type && to.type == transition::type)
		{
			petri::iterator mid = create(place());
			arcs[from.type].push_back(arc(from, mid));
			arcs[mid.type].push_back(arc(mid, to));
		}
		else
		{
			mark_modified();
			arcs[from.type].push_back(arc(from, to));
		}
		return to;
	}

	virtual vector<petri::iterator> connect(petri::iterator from, vector<petri::iterator> to)
	{
		for (int i = 0; i < (int)to.size(); i++)
			connect(from, to[i]);
		return to;
	}

	virtual petri::iterator connect(vector<petri::iterator> from, petri::iterator to)
	{
		for (int i = 0; i < (int)from.size(); i++)
			connect(from[i], to);
		return to;
	}

	virtual vector<petri::iterator> connect(vector<petri::iterator> from, vector<petri::iterator> to)
	{
		for (int i = 0; i < (int)from.size(); i++)
			for (int j = 0; j < (int)to.size(); j++)
				connect(from[i], to[j]);
		return to;
	}

	virtual petri::iterator connect(arc a)
	{
		return connect(a.from, a.to);
	}

	virtual vector<petri::iterator> connect(vector<arc> a)
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)a.size(); i++)
			result.push_back(connect(a[i].from, a[i].to));
		return result;
	}

	virtual petri::iterator connect(vector<petri::iterator> a) {
		for (auto i0 = a.begin(); ::next(i0) != a.end(); i0++) {
			auto i1 = ::next(i0);
			connect(*i0, *i1);
		}
		return a.back();
	}

	virtual void disconnect(petri::iterator a)
	{
		mark_modified();
		arcs[a.type].erase(arcs[a.type].begin() + a.index);
	}


	virtual petri::iterator copy(petri::iterator i)
	{
		if (i.type == place::type && i.index < (int)places.size())
		{
			mark_modified();
			places.push_back(places[i.index]);
			petri::iterator result(i.type, places.size()-1);
			for (int j = 0; j < (int)source.size(); j++)
				for (int k = 0; k < (int)source[j].tokens.size(); k++)
					if (source[j].tokens[k].index == i.index)
					{
						source[j].tokens.push_back(source[j].tokens[k]);
						source[j].tokens.back().index = result.index;
					}

			for (int j = 0; j < (int)reset.size(); j++)
				for (int k = 0; k < (int)reset[j].tokens.size(); k++)
					if (reset[j].tokens[k].index == i.index)
					{
						reset[j].tokens.push_back(reset[j].tokens[k]);
						reset[j].tokens.back().index = result.index;
					}

			for (int j = 0; j < (int)sink.size(); j++)
				for (int k = 0; k < (int)sink[j].tokens.size(); k++)
					if (sink[j].tokens[k].index == i.index)
					{
						sink[j].tokens.push_back(sink[j].tokens[k]);
						sink[j].tokens.back().index = result.index;
					}

			return result;
		}
		else if (i.type == transition::type && i.index < (int)transitions.size())
		{
			mark_modified();
			transitions.push_back(transitions[i.index]);
			return petri::iterator(i.type, transitions.size()-1);
		}
		else
		{
			internal("petri::copy", "iterator out of bounds", __FILE__, __LINE__);
			return petri::iterator();
		}
	}

	virtual vector<petri::iterator> copy(petri::iterator i, int num)
	{
		vector<petri::iterator> result;
		if (i.type == place::type && i.index < (int)places.size()) {
			for (int j = 0; j < num; j++)
			{
				mark_modified();
				places.push_back(places[i.index]);
				result.push_back(petri::iterator(i.type, places.size()-1));
			}

			for (int j = 0; j < (int)source.size(); j++)
				for (int k = 0; k < (int)source[j].tokens.size(); k++)
					if (source[j].tokens[k].index == i.index)
						for (int l = 0; l < (int)result.size(); l++)
						{
							source[j].tokens.push_back(source[j].tokens[k]);
							source[j].tokens.back().index = result[l].index;
						}

			for (int j = 0; j < (int)reset.size(); j++)
				for (int k = 0; k < (int)reset[j].tokens.size(); k++)
					if (reset[j].tokens[k].index == i.index)
						for (int l = 0; l < (int)result.size(); l++)
						{
							reset[j].tokens.push_back(reset[j].tokens[k]);
							reset[j].tokens.back().index = result[l].index;
						}

			for (int j = 0; j < (int)sink.size(); j++)
				for (int k = 0; k < (int)sink[j].tokens.size(); k++)
					if (sink[j].tokens[k].index == i.index)
						for (int l = 0; l < (int)result.size(); l++)
						{
							sink[j].tokens.push_back(sink[j].tokens[k]);
							sink[j].tokens.back().index = result[l].index;
						}
		} else if (i.type == transition::type && i.index < (int)transitions.size()) {
			for (int j = 0; j < num; j++)
			{
				mark_modified();
				transitions.push_back(transitions[i.index]);
				result.push_back(petri::iterator(i.type, transitions.size()-1));
			}
		} else
		{
			internal("petri::copy", "iterator out of bounds", __FILE__, __LINE__);
			return vector<petri::iterator>();
		}
		return result;
	}

	virtual vector<petri::iterator> copy(vector<petri::iterator> i, int num = 1)
	{
		vector<petri::iterator> result;
		for (int j = 0; j < (int)i.size(); j++)
		{
			vector<petri::iterator> temp = copy(i[j], num);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual petri::iterator copy_combine(int composition, petri::iterator i0, petri::iterator i1)
	{
		if (i0.type == place::type && i1.type == place::type)
			return create(place::merge(composition, places[i0.index], places[i1.index]));
		else if (i0.type == transition::type && i1.type == transition::type)
		{
			if (transition::mergeable(composition, transitions[i0.index], transitions[i1.index]))
				return create(transition::merge(composition, transitions[i0.index], transitions[i1.index]));
			else
			{
				internal("petri::copy_combine", "transitions are not mergeable", __FILE__, __LINE__);
				return petri::iterator();
			}
		}
		else
		{
			internal("petri::copy_combine", "iterator types do not match", __FILE__, __LINE__);
			return petri::iterator();
		}
	}

	virtual petri::iterator combine(int composition, petri::iterator i0, petri::iterator i1)
	{
		if (i0.type == place::type && i1.type == place::type)
		{
			places[i0.index] = place::merge(composition, places[i0.index], places[i1.index]);
			return i0;
		}
		else if (i0.type == transition::type && i1.type == transition::type)
		{
			if (transition::mergeable(composition, transitions[i0.index], transitions[i1.index]))
			{
				transitions[i0.index] = transition::merge(composition, transitions[i0.index], transitions[i1.index]);
				return i0;
			}
			else
			{
				internal("petri::combine", "transitions are not mergeable", __FILE__, __LINE__);
				return petri::iterator();
			}
		}
		else
		{
			internal("petri::combine", "iterator types do not match", __FILE__, __LINE__);
			return petri::iterator();
		}
	}

	template <class node>
	petri::iterator push_back(petri::iterator from, node n)
	{
		return connect(from, create(n));
	}

	template <class node>
	petri::iterator push_back(vector<petri::iterator> from, node n)
	{
		return connect(from, create(n));
	}

	template <class node>
	vector<petri::iterator> push_back(petri::iterator from, node n, int num)
	{
		return connect(from, create(n, num));
	}

	template <class node>
	vector<petri::iterator> push_back(vector<petri::iterator> from, node n, int num)
	{
		return connect(from, create(n, num));
	}

	template <class node>
	petri::iterator push_front(petri::iterator to, node n)
	{
		return connect(create(n), to);
	}

	template <class node>
	petri::iterator push_front(vector<petri::iterator> to, node n)
	{
		return connect(create(n), to);
	}

	template <class node>
	vector<petri::iterator> push_front(petri::iterator to, node n, int num)
	{
		return connect(create(n, num), to);
	}

	template <class node>
	vector<petri::iterator> push_front(vector<petri::iterator> to, node n, int num)
	{
		return connect(create(n, num), to);
	}

	virtual petri::iterator insert(petri::iterator a, place n)
	{
		petri::iterator i[2];
		i[place::type] = create(n);
		i[transition::type] = create(transition());
		arcs[a.type].push_back(arc(i[a.type], arcs[a.type][a.index].to));
		arcs[1-a.type].push_back(arc(i[1-a.type], i[a.type]));
		arcs[a.type][a.index].to = i[1-a.type];
		return i[place::type];
	}

	virtual petri::iterator insert(petri::iterator a, transition n)
	{
		petri::iterator i[2];
		i[place::type] = create(place());
		i[transition::type] = create(n);
		arcs[a.type].push_back(arc(i[a.type], arcs[a.type][a.index].to));
		arcs[1-a.type].push_back(arc(i[1-a.type], i[a.type]));
		arcs[a.type][a.index].to = i[1-a.type];
		return i[transition::type];
	}

	virtual petri::iterator insert(petri::iterator a, int n)
	{
		if (n == place::type)
			return insert(a, place());
		else if (n == transition::type)
			return insert(a, transition());
		else
			return petri::iterator();
	}

	virtual petri::iterator insert_alongside(petri::iterator from, petri::iterator to, place n)
	{
		petri::iterator i = create(n);
		if (from.type == i.type)
		{
			petri::iterator j = create(transition());
			connect(from, j);
			connect(j, i);
		}
		else
			connect(from, i);

		if (to.type == i.type)
		{
			petri::iterator j = create(transition());
			connect(i, j);
			connect(j, to);
		}
		else
			connect(i, to);

		return i;
	}

	virtual petri::iterator insert_alongside(petri::iterator from, petri::iterator to, transition n)
	{
		petri::iterator i = create(n);
		if (from.type == i.type)
		{
			petri::iterator j = create(place());
			connect(from, j);
			connect(j, i);
		}
		else
			connect(from, i);

		if (to.type == i.type)
		{
			petri::iterator j = create(place());
			connect(i, j);
			connect(j, to);
		}
		else
			connect(i, to);

		return i;
	}

	virtual petri::iterator insert_alongside(petri::iterator from, petri::iterator to, int n)
	{
		if (n == place::type)
			return insert_alongside(from, to, place());
		else if (n == transition::type)
			return insert_alongside(from, to, transition());
		else
			return petri::iterator();
	}

	virtual petri::iterator insert_before(petri::iterator to, place n)
	{
		petri::iterator i[2];
		i[transition::type] = create(transition());
		i[place::type] = create(n);
		for (int j = 0; j < (int)arcs[1-to.type].size(); j++)
			if (arcs[1-to.type][j].to.index == to.index)
				arcs[1-to.type][j].to.index = i[to.type].index;
		connect(i[1-to.type], to);
		connect(i[to.type], i[1-to.type]);
		return i[place::type];
	}

	virtual petri::iterator insert_before(petri::iterator to, transition n)
	{
		petri::iterator i[2];
		i[transition::type] = create(n);
		i[place::type] = create(place());
		for (int j = 0; j < (int)arcs[1-to.type].size(); j++)
			if (arcs[1-to.type][j].to.index == to.index)
				arcs[1-to.type][j].to.index = i[to.type].index;
		connect(i[1-to.type], to);
		connect(i[to.type], i[1-to.type]);
		return i[transition::type];
	}

	virtual petri::iterator insert_before(petri::iterator to, int n)
	{
		if (n == place::type)
			return insert_before(to, place());
		else if (n == transition::type)
			return insert_before(to, transition());
		else
			return petri::iterator();
	}

	virtual petri::iterator insert_after(petri::iterator from, place n)
	{
		petri::iterator i[2];
		i[transition::type] = create(transition());
		i[place::type] = create(n);
		for (int j = 0; j < (int)arcs[from.type].size(); j++)
			if (arcs[from.type][j].from.index == from.index)
				arcs[from.type][j].from.index = i[from.type].index;
		connect(from, i[1-from.type]);
		connect(i[1-from.type], i[from.type]);
		return i[place::type];
	}


	virtual petri::iterator insert_after(petri::iterator from, transition n)
	{
		petri::iterator i[2];
		i[transition::type] = create(n);
		i[place::type] = create(place());
		for (int j = 0; j < (int)arcs[from.type].size(); j++)
			if (arcs[from.type][j].from.index == from.index)
				arcs[from.type][j].from.index = i[from.type].index;
		connect(from, i[1-from.type]);
		connect(i[1-from.type], i[from.type]);
		return i[transition::type];
	}

	virtual petri::iterator insert_after(petri::iterator from, int n)
	{
		if (n == place::type)
			return insert_after(from, place());
		else if (n == transition::type)
			return insert_after(from, transition());
		else
			return petri::iterator();
	}

	virtual petri::iterator insert_at(vector<petri::iterator> to, transition n) {
		petri::iterator t = create(n);
		// TODO(edward.bingham) inputs should be arcs between nodes
		// 1. identify all possible conditional splits of parallel groups of
		// input nodes.
		// 2. fix fully covered parallel groups to ensure exclusivity by
		// adding an input node that connects back to the most recent
		// non-shared split point.
		// 3. cut associated arcs, adding a single transition per parallel group.

		for (auto i = to.begin(); i != to.end(); i++) {
			petri::iterator p = t;
			if (i->type == place::type) {
				p = create(place::type);
				connect(p, t);
			}
			for (auto j = arcs[1-i->type].begin(); j != arcs[1-i->type].end(); j++) {
				if (j->to.index == i->index) {
					j->to.index = p.index;
				}
			}
			connect(t, *i);
		}
		return t;
	}

	virtual petri::iterator duplicate(int composition, petri::iterator i, bool add = true)
	{
		petri::iterator d = copy(i);
		if ((i.type == transition::type and composition == choice) or (i.type == place::type and composition == parallel))
		{
			for (int j = (int)arcs[i.type].size()-1; j >= 0; j--)
				if (arcs[i.type][j].from == i)
					connect(d, arcs[i.type][j].to);
			for (int j = (int)arcs[1-i.type].size()-1; j >= 0; j--)
				if (arcs[1-i.type][j].to == i)
					connect(arcs[1-i.type][j].from, d);
		}
		else if (add)
		{
			vector<petri::iterator> x = create(1-i.type, 4);
			vector<petri::iterator> y = create(i.type, 2);

			for (int j = (int)arcs[i.type].size()-1; j >= 0; j--)
				if (arcs[i.type][j].from == i)
					arcs[i.type][j].from = y[1];
			for (int j = (int)arcs[1-i.type].size()-1; j >= 0; j--)
				if (arcs[1-i.type][j].to == i)
					arcs[1-i.type][j].to = y[0];

			connect(y[0], x[0]);
			connect(y[0], x[1]);
			connect(x[0], i);
			connect(x[1], d);
			connect(i, x[2]);
			connect(d, x[3]);
			connect(x[2], y[1]);
			connect(x[3], y[1]);
		}
		else
		{
			vector<petri::iterator> n = next(i);
			vector<petri::iterator> p = prev(i);

			for (int j = 0; j < 2; j++)
				for (int k = (int)arcs[j].size()-1; k >= 0; k--)
					if (arcs[j][k].from == i || arcs[j][k].to == i)
						arcs[j].erase(arcs[j].begin() + k);

			vector<petri::iterator> n1, p1;
			for (int l = 0; l < (int)n.size(); l++)
				n1.push_back(duplicate(composition, n[l]));
			for (int l = 0; l < (int)p.size(); l++)
				p1.push_back(duplicate(composition, p[l]));

			connect(p1, d);
			connect(d, n1);
			connect(p, i);
			connect(i, n);
		}

		return d;
	}

	virtual vector<petri::iterator> duplicate(int composition, petri::iterator i, int num, bool add = true)
	{
		if (num == 0) {
			return vector<petri::iterator>();
		}

		vector<petri::iterator> d = copy(i, num-1);
		if ((i.type == transition::type and composition == choice) or (i.type == place::type and composition == parallel))
		{
			for (int j = (int)arcs[i.type].size()-1; j >= 0; j--)
				if (arcs[i.type][j].from == i)
					for (int k = 0; k < (int)d.size(); k++)
						connect(d[k], arcs[i.type][j].to);
			for (int j = (int)arcs[1-i.type].size()-1; j >= 0; j--)
				if (arcs[1-i.type][j].to == i)
					for (int k = 0; k < (int)d.size(); k++)
						connect(arcs[1-i.type][j].from, d[k]);
		}
		else if (add)
		{
			vector<petri::iterator> x = create(1-i.type, 2*(num-1));
			vector<petri::iterator> y = create(i.type, 2);
			vector<petri::iterator> z = create(1-i.type, 2);

			for (int j = (int)arcs[i.type].size()-1; j >= 0; j--)
				if (arcs[i.type][j].from == i)
					arcs[i.type][j].from = y[1];
			for (int j = (int)arcs[1-i.type].size()-1; j >= 0; j--)
				if (arcs[1-i.type][j].to == i)
					arcs[1-i.type][j].to = y[0];

			connect(y[0], z[0]);
			connect(z[0], i);
			connect(i, z[1]);
			connect(z[1], y[1]);

			for (int k = 0; k < (int)d.size(); k++)
			{
				connect(y[0], x[k*2 + 0]);
				connect(x[k*2 + 0], d[k]);
				connect(d[k], x[k*2 + 1]);
				connect(x[k*2 + 1], y[1]);
			}
		}
		else
		{
			vector<petri::iterator> n = next(i);
			vector<petri::iterator> p = prev(i);

			for (int j = 0; j < 2; j++)
				for (int k = (int)arcs[j].size()-1; k >= 0; k--)
					if (arcs[j][k].from == i || arcs[j][k].to == i)
						arcs[j].erase(arcs[j].begin() + k);

			for (int k = 0; k < num-1; k++)
			{
				vector<petri::iterator> n1, p1;
				for (int l = 0; l < (int)n.size(); l++)
					n1.push_back(duplicate(composition, n[l]));
				for (int l = 0; l < (int)p.size(); l++)
					p1.push_back(duplicate(composition, p[l]));

				connect(p1, d[k]);
				connect(d[k], n1);
			}
			connect(p, i);
			connect(i, n);
		}

		d.push_back(i);

		return d;
	}

	virtual vector<petri::iterator> duplicate(int composition, vector<petri::iterator> n, int num = 1, bool interleaved = false, bool add = true)
	{
		vector<petri::iterator> result;
		result.reserve(n.size()*num);
		for (int i = 0; i < (int)n.size(); i++)
		{
			vector<petri::iterator> temp = duplicate(composition, n[i], num, add);
			if (interleaved && i > 0)
				for (int j = 0; j < (int)temp.size(); j++)
					result.insert(result.begin() + j*(i+1) + 1, temp[j]);
			else
				result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual map<petri::iterator, vector<petri::iterator> > pinch(petri::iterator n)
	{
		pair<vector<petri::iterator>, vector<petri::iterator> > neighbors = erase(n);

		vector<petri::iterator> left = duplicate(n.type, neighbors.first, neighbors.second.size(), false);
		vector<petri::iterator> right = duplicate(n.type, neighbors.second, neighbors.first.size(), true);

		for (int i = 0; i < (int)right.size(); i++)
		{
			combine(sequence, left[i], right[i]);

			for (int j = 0; j < (int)arcs[right[i].type].size(); j++)
				if (arcs[right[i].type][j].from == right[i])
					arcs[right[i].type][j].from = left[i];

			for (int j = 0; j < (int)arcs[1-right[i].type].size(); j++)
				if (arcs[1-right[i].type][j].to == right[i])
					arcs[1-right[i].type][j].to = left[i];

			if (right[i].type == place::type)
			{
				for (int j = 0; j < (int)source.size(); j++)
					for (int k = 0; k < (int)source[j].tokens.size(); k++)
						if (source[j].tokens[k].index == right[i].index)
						{
							source[j].tokens.push_back(source[j].tokens[k]);
							source[j].tokens.back().index = left[i].index;
						}

				for (int j = 0; j < (int)reset.size(); j++)
					for (int k = 0; k < (int)reset[j].tokens.size(); k++)
						if (reset[j].tokens[k].index == right[i].index)
						{
							reset[j].tokens.push_back(reset[j].tokens[k]);
							reset[j].tokens.back().index = left[i].index;
						}

				for (int j = 0; j < (int)sink.size(); j++)
					for (int k = 0; k < (int)sink[j].tokens.size(); k++)
						if (sink[j].tokens[k].index == right[i].index)
						{
							sink[j].tokens.push_back(sink[j].tokens[k]);
							sink[j].tokens.back().index = left[i].index;
						}
			}
		}

		erase(right);
		erase(right, left);

		map<petri::iterator, vector<petri::iterator> > result;
		for (int i = 0; i < (int)left.size(); i++)
			result.insert(pair<petri::iterator, vector<petri::iterator> >(right[i], vector<petri::iterator>(1, left[i])));
		return result;
	}

	virtual vector<petri::iterator> next(petri::iterator n) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[n.type].size(); i++)
			if (arcs[n.type][i].from.index == n.index)
				result.push_back(arcs[n.type][i].to);
		return result;
	}

	virtual vector<petri::iterator> next(vector<petri::iterator> n) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)n.size(); i++)
		{
			vector<petri::iterator> temp = next(n[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<petri::iterator> prev(petri::iterator n) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-n.type].size(); i++)
			if (arcs[1-n.type][i].to.index == n.index)
				result.push_back(arcs[1-n.type][i].from);
		return result;
	}

	virtual vector<petri::iterator> prev(vector<petri::iterator> n) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)n.size(); i++)
		{
			vector<petri::iterator> temp = prev(n[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<petri::iterator> neighbors(petri::iterator n, bool sorted = false) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-n.type].size(); i++)
			if (arcs[1-n.type][i].to.index == n.index)
				result.push_back(arcs[1-n.type][i].from);

		for (int i = 0; i < (int)arcs[n.type].size(); i++)
			if (arcs[n.type][i].from.index == n.index)
				result.push_back(arcs[n.type][i].to);

		if (sorted)
			sort(result.begin(), result.end());
		return result;
	}

	virtual vector<int> next(int type, int n) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[type].size(); i++)
			if (arcs[type][i].from.index == n)
				result.push_back(arcs[type][i].to.index);
		return result;
	}

	virtual vector<int> next(int type, vector<int> n) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[type].size(); i++)
			if (find(n.begin(), n.end(), arcs[type][i].from.index) != n.end())
				result.push_back(arcs[type][i].to.index);
		return result;
	}

	virtual vector<int> prev(int type, int n) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (arcs[1-type][i].to.index == n)
				result.push_back(arcs[1-type][i].from.index);
		return result;
	}

	virtual vector<int> prev(int type, vector<int> n) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (find(n.begin(), n.end(), arcs[1-type][i].to.index) != n.end())
				result.push_back(arcs[1-type][i].from.index);
		return result;
	}

	virtual vector<int> neighbors(int type, int n, bool sorted = false) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (arcs[1-type][i].to.index == n)
				result.push_back(arcs[1-type][i].from.index);

		for (int i = 0; i < (int)arcs[type].size(); i++)
			if (arcs[type][i].from.index == n)
				result.push_back(arcs[type][i].to.index);

		if (sorted)
			sort(result.begin(), result.end());

		return result;
	}

	virtual vector<int> neighbors(int type, vector<int> n, bool sorted = false) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (find(n.begin(), n.end(), arcs[1-type][i].to.index) != n.end())
				result.push_back(arcs[1-type][i].from.index);

		for (int i = 0; i < (int)arcs[type].size(); i++)
			if (find(n.begin(), n.end(), arcs[type][i].from.index) != n.end())
				result.push_back(arcs[type][i].to.index);

		if (sorted)
			sort(result.begin(), result.end());

		return result;
	}

	virtual vector<petri::iterator> out(petri::iterator n) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[n.type].size(); i++)
			if (arcs[n.type][i].from.index == n.index)
				result.push_back(petri::iterator(n.type, i));
		return result;
	}

	virtual vector<petri::iterator> out(vector<petri::iterator> n) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)n.size(); i++)
		{
			vector<petri::iterator> temp = out(n[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<petri::iterator> in(petri::iterator n) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-n.type].size(); i++)
			if (arcs[1-n.type][i].to.index == n.index)
				result.push_back(petri::iterator(1-n.type, i));
		return result;
	}

	virtual vector<petri::iterator> in(vector<petri::iterator> n) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)n.size(); i++)
		{
			vector<petri::iterator> temp = in(n[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<int> out(int type, int n) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[type].size(); i++)
			if (arcs[type][i].from.index == n)
				result.push_back(i);
		return result;
	}

	virtual vector<int> out(int type, vector<int> n) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[type].size(); i++)
			if (find(n.begin(), n.end(), arcs[type][i].from.index) != n.end())
				result.push_back(i);
		return result;
	}

	virtual vector<int> in(int type, int n) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (arcs[1-type][i].to.index == n)
				result.push_back(i);
		return result;
	}

	virtual vector<int> in(int type, vector<int> n) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (find(n.begin(), n.end(), arcs[1-type][i].to.index) != n.end())
				result.push_back(i);
		return result;
	}

	virtual vector<petri::iterator> next_arcs(petri::iterator a) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-a.type].size(); i++)
			if (arcs[1-a.type][i].from == arcs[a.type][a.index].to)
				result.push_back(petri::iterator(1-a.type, i));
		return result;
	}

	virtual vector<petri::iterator> next_arcs(vector<petri::iterator> a) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)a.size(); i++)
		{
			vector<petri::iterator> temp = next_arcs(a[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<petri::iterator> prev_arcs(petri::iterator a) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-a.type].size(); i++)
			if (arcs[1-a.type][i].to == arcs[a.type][a.index].from)
				result.push_back(petri::iterator(1-a.type, i));
		return result;
	}

	virtual vector<petri::iterator> prev_arcs(vector<petri::iterator> a) const
	{
		vector<petri::iterator> result;
		for (int i = 0; i < (int)a.size(); i++)
		{
			vector<petri::iterator> temp = prev_arcs(a[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<int> next_arcs(int type, int a) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (arcs[1-type][i].from == arcs[type][a].to)
				result.push_back(i);
		return result;
	}

	virtual vector<int> next_arcs(int type, vector<int> a) const
	{
		vector<int> result;
		for (int i = 0; i < (int)a.size(); i++)
		{
			vector<int> temp = next_arcs(type, a[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<int> prev_arcs(int type, int a) const
	{
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (arcs[1-type][i].to == arcs[type][a].from)
				result.push_back(i);
		return result;
	}

	virtual vector<int> prev_arcs(int type, vector<int> a) const
	{
		vector<int> result;
		for (int i = 0; i < (int)a.size(); i++)
		{
			vector<int> temp = prev_arcs(type, a[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual map<petri::iterator, vector<petri::iterator> > merge(int composition, const graph &g)
	{
		if (places.size() == 0 && transitions.size() == 0)
		{
			places = g.places;
			transitions = g.transitions;
			arcs[0] = g.arcs[0];
			arcs[1] = g.arcs[1];
			source = g.source;
			sink = g.sink;
			reset = g.reset;
			node_distances = g.node_distances;
			node_distances_ready = g.node_distances_ready;
			split_groups_ready = g.split_groups_ready;

			map<petri::iterator, vector<petri::iterator> > result;
			for (int i = 0; i < (int)places.size(); i++)
				result.insert(pair<petri::iterator, vector<petri::iterator> >(petri::iterator(place::type, i), vector<petri::iterator>(1, petri::iterator(place::type, i))));

			for (int i = 0; i < (int)transitions.size(); i++)
				result.insert(pair<petri::iterator, vector<petri::iterator> >(petri::iterator(transition::type, i), vector<petri::iterator>(1, petri::iterator(transition::type, i))));

			return result;
		}
		else if ((g.places.size() == 0 && g.transitions.size() == 0) || (composition == sequence && (sink.size() == 0 || g.source.size() == 0)))
			return map<petri::iterator, vector<petri::iterator> >();
		else
		{
			mark_modified();
			map<petri::iterator, vector<petri::iterator> > result;

			places.reserve(places.size() + g.places.size());
			for (int i = 0; i < (int)g.places.size(); i++)
			{
				result.insert(pair<petri::iterator, vector<petri::iterator> >(petri::iterator(place::type, i), vector<petri::iterator>(1, petri::iterator(place::type, (int)places.size()))));
				places.push_back(g.places[i]);
			}

			transitions.reserve(transitions.size() + g.transitions.size());
			for (int i = 0; i < (int)g.transitions.size(); i++)
			{
				result.insert(pair<petri::iterator, vector<petri::iterator> >(petri::iterator(transition::type, i), vector<petri::iterator>(1, petri::iterator(transition::type, (int)transitions.size()))));
				transitions.push_back(g.transitions[i]);
			}

			for (int i = 0; i < 2; i++)
				for (int j = 0; j < (int)g.arcs[i].size(); j++)
				{
					vector<petri::iterator> from = result[g.arcs[i][j].from];
					vector<petri::iterator> to = result[g.arcs[i][j].to];
					for (int k = 0; k < (int)from.size(); k++)
						for (int l = 0; l < (int)to.size(); l++)
							arcs[i].push_back(arc(from[k], to[l]));
				}

			vector<state> converted_source;
			vector<state> converted_sink;
			vector<state> converted_reset;

			for (int i = 0; i < (int)g.source.size(); i++)
				converted_source.push_back(g.source[i].convert(result));

			for (int i = 0; i < (int)g.sink.size(); i++)
				converted_sink.push_back(g.sink[i].convert(result));

			for (int i = 0; i < (int)g.reset.size(); i++)
				converted_reset.push_back(g.reset[i].convert(result));


			if (composition == choice || source.size() == 0)
			{
				for (int i = 0; i < (int)converted_source.size(); i++)
				{
					typename vector<state>::iterator iter = lower_bound(source.begin(), source.end(), converted_source[i]);
					if (iter != source.end() && *iter == converted_source[i])
						*iter = state::merge(*iter, converted_source[i]);
					else
						source.insert(iter, converted_source[i]);
				}

				for (int i = 0; i < (int)converted_reset.size(); i++)
				{
					typename vector<state>::iterator iter = lower_bound(reset.begin(), reset.end(), converted_reset[i]);
					if (iter != reset.end() && *iter == converted_reset[i])
						*iter = state::merge(*iter, converted_reset[i]);
					else
						reset.insert(iter, converted_reset[i]);
				}
			}
			else if (composition == parallel)
			{
				if (source.size() > 1)
				{
					vector<petri::iterator> rem;
					petri::iterator p = create(place());
					for (int i = 0; i < (int)source.size(); i++)
					{
						if (source[i].tokens.size() > 1)
						{
							petri::iterator t = create(transition());
							connect(p, t);
							for (int j = 0; j < (int)source[i].tokens.size(); j++)
								connect(t, petri::iterator(place::type, source[i].tokens[j].index));
						}
						else if (source[i].tokens.size() == 1)
						{
							petri::iterator p0(place::type, source[i].tokens[0].index);
							connect(p, next(p0));
							connect(prev(p0), p);
							rem.push_back(p0);
						}

						source[i] = state::collapse(p.index, source[i]);
						if (i != 0)
							source[0] = state::merge(source[0], source[i]);
					}
					source = vector<state>(1, state::collapse(p.index, source[0]));

					sort(rem.begin(), rem.end());
					rem.resize(unique(rem.begin(), rem.end()) - rem.begin());
					reverse(rem.begin(), rem.end());

					for (int i = 0; i < (int)rem.size(); i++)
					{
						for (int j = 0; j < (int)sink.size(); j++)
							for (int k = (int)sink[j].tokens.size()-1; k >= 0; k--)
								if (sink[j].tokens[k].index == rem[i].index)
									sink[j].tokens[k].index = p.index;

						for (int j = 0; j < (int)reset.size(); j++)
							for (int k = (int)reset[j].tokens.size()-1; k >= 0; k--)
								if (reset[j].tokens[k].index == rem[i].index)
									reset[j].tokens[k].index = p.index;

						for (map<petri::iterator, vector<petri::iterator> >::iterator j = result.begin(); j != result.end(); j++)
						{
							for (int k = (int)j->second.size()-1; k >= 0; k--)
							{
								if (j->second[k] > rem[i])
									j->second[k]--;
								else if (j->second[k] == rem[i])
								{
									j->second.erase(j->second.begin() + k);
									j->second.push_back(p);
								}
							}

							sort(j->second.begin(), j->second.end());
							j->second.resize(unique(j->second.begin(), j->second.end()) - j->second.begin());
						}

						graph::erase(rem[i], converted_source);
						graph::erase(rem[i], converted_sink);
						graph::erase(rem[i], converted_reset);
					}

					erase(rem, true);
				}

				if (converted_source.size() > 1)
				{
					vector<petri::iterator> rem;
					petri::iterator p = create(place());
					for (int i = 0; i < (int)converted_source.size(); i++)
					{
						if (converted_source[i].tokens.size() > 1)
						{
							petri::iterator t = create(transition());
							connect(p, t);
							for (int j = 0; j < (int)converted_source[i].tokens.size(); j++)
								connect(t, petri::iterator(place::type, converted_source[i].tokens[j].index));
						}
						else if (converted_source[i].tokens.size() == 1)
						{
							petri::iterator p0(place::type, converted_source[i].tokens[0].index);
							connect(p, next(p0));
							connect(prev(p0), p);
							rem.push_back(p0);
						}

						converted_source[i] = state::collapse(p.index, converted_source[i]);
						if (i != 0)
							converted_source[0] = state::merge(converted_source[0], converted_source[i]);
					}
					converted_source = vector<state>(1, state::collapse(p.index, converted_source[0]));

					sort(rem.begin(), rem.end());
					rem.resize(unique(rem.begin(), rem.end()) - rem.begin());
					reverse(rem.begin(), rem.end());

					for (int i = 0; i < (int)rem.size(); i++)
					{
						for (int j = 0; j < (int)converted_sink.size(); j++)
							for (int k = (int)converted_sink[j].tokens.size()-1; k >= 0; k--)
								if (converted_sink[j].tokens[k].index == rem[i].index)
									converted_sink[j].tokens[k].index = p.index;

						for (int j = 0; j < (int)converted_reset.size(); j++)
							for (int k = (int)converted_reset[j].tokens.size()-1; k >= 0; k--)
								if (converted_reset[j].tokens[k].index == rem[i].index)
									converted_reset[j].tokens[k].index = p.index;

						for (map<petri::iterator, vector<petri::iterator> >::iterator j = result.begin(); j != result.end(); j++)
						{
							for (int k = (int)j->second.size()-1; k >= 0; k--)
							{
								if (j->second[k] > rem[i])
									j->second[k]--;
								else if (j->second[k] == rem[i])
								{
									j->second.erase(j->second.begin() + k);
									j->second.push_back(p);
								}
							}

							sort(j->second.begin(), j->second.end());
							j->second.resize(unique(j->second.begin(), j->second.end()) - j->second.begin());
						}

						graph::erase(rem[i], converted_source);
						graph::erase(rem[i], converted_sink);
						graph::erase(rem[i], converted_reset);
					}

					erase(rem, true);
				}

				if (source.size() == 1 && converted_source.size() == 1)
					source[0] = state::merge(source[0], converted_source[0]);
				else if (converted_source.size() == 1)
					source = converted_source;

				if (reset.size() == 0 && converted_reset.size() > 0)
					reset = source;
				else if (reset.size() > 0 && converted_reset.size() == 0)
					converted_reset = converted_source;

				if (reset.size() > 0 || converted_reset.size() > 0)
				{
					int s = (int)reset.size();
					for (int i = 0; i < (int)converted_reset.size()-1; i++)
						for (int j = 0; j < s; j++)
							reset.push_back(state::merge(reset[j], converted_reset[i]));

					if (converted_reset.size() > 0)
						for (int j = 0; j < s; j++)
							reset[j] = state::merge(reset[j], converted_reset.back());
				}
			}

			if (composition == choice || sink.size() == 0)
			{
				for (int i = 0; i < (int)g.sink.size(); i++)
				{
					typename vector<state>::iterator iter = lower_bound(sink.begin(), sink.end(), converted_sink[i]);
					if (iter != sink.end() && *iter == converted_sink[i])
						*iter = state::merge(*iter, converted_sink[i]);
					else
						sink.insert(iter, converted_sink[i]);
				}
			}
			else if (composition == parallel)
			{
				if (sink.size() > 1)
				{
					vector<petri::iterator> rem;
					petri::iterator p = create(place());
					for (int i = 0; i < (int)sink.size(); i++)
					{
						if (sink[i].tokens.size() > 1)
						{
							petri::iterator t = create(transition());
							connect(t, p);
							for (int j = 0; j < (int)sink[i].tokens.size(); j++)
								connect(petri::iterator(place::type, sink[i].tokens[j].index), t);
						}
						else if (sink[i].tokens.size() == 1)
						{
							petri::iterator p0(place::type, sink[i].tokens[0].index);
							connect(p, next(p0));
							connect(prev(p0), p);
							rem.push_back(p0);
						}

						sink[i] = state::collapse(p.index, sink[i]);
						if (i != 0)
							sink[0] = state::merge(sink[0], sink[i]);
					}
					sink = vector<state>(1, state::collapse(p.index, sink[0]));

					sort(rem.begin(), rem.end());
					rem.resize(unique(rem.begin(), rem.end()) - rem.begin());
					reverse(rem.begin(), rem.end());

					for (int i = 0; i < (int)rem.size(); i++)
					{
						for (int j = 0; j < (int)source.size(); j++)
							for (int k = (int)source[j].tokens.size()-1; k >= 0; k--)
								if (source[j].tokens[k].index == rem[i].index)
									source[j].tokens[k].index = p.index;

						for (int j = 0; j < (int)reset.size(); j++)
							for (int k = (int)reset[j].tokens.size()-1; k >= 0; k--)
								if (reset[j].tokens[k].index == rem[i].index)
									reset[j].tokens[k].index = p.index;

						for (map<petri::iterator, vector<petri::iterator> >::iterator j = result.begin(); j != result.end(); j++)
						{
							for (int k = (int)j->second.size()-1; k >= 0; k--)
							{
								if (j->second[k] > rem[i])
									j->second[k]--;
								else if (j->second[k] == rem[i])
								{
									j->second.erase(j->second.begin() + k);
									j->second.push_back(p);
								}
							}

							sort(j->second.begin(), j->second.end());
							j->second.resize(unique(j->second.begin(), j->second.end()) - j->second.begin());
						}

						graph::erase(rem[i], converted_source);
						graph::erase(rem[i], converted_sink);
						graph::erase(rem[i], converted_reset);
					}

					erase(rem, true);
				}

				if (converted_sink.size() > 1)
				{
					vector<petri::iterator> rem;
					petri::iterator p = create(place());
					for (int i = 0; i < (int)converted_sink.size(); i++)
					{
						if (converted_sink[i].tokens.size() > 1)
						{
							petri::iterator t = create(transition());
							connect(t, p);
							for (int j = 0; j < (int)converted_sink[i].tokens.size(); j++)
								connect(petri::iterator(place::type, converted_sink[i].tokens[j].index), t);
						}
						else if (converted_sink[i].tokens.size() == 1)
						{
							petri::iterator p0(place::type, converted_sink[i].tokens[0].index);
							connect(p, next(p0));
							connect(prev(p0), p);
							rem.push_back(p0);
						}

						converted_sink[i] = state::collapse(p.index, converted_sink[i]);
						if (i != 0)
							converted_sink[0] = state::merge(converted_sink[0], converted_sink[i]);
					}
					converted_sink = vector<state>(1, state::collapse(p.index, converted_sink[0]));

					sort(rem.begin(), rem.end());
					rem.resize(unique(rem.begin(), rem.end()) - rem.begin());
					reverse(rem.begin(), rem.end());

					for (int i = 0; i < (int)rem.size(); i++)
					{
						for (int j = 0; j < (int)converted_source.size(); j++)
							for (int k = (int)converted_source[j].tokens.size()-1; k >= 0; k--)
								if (converted_source[j].tokens[k].index == rem[i].index)
									converted_source[j].tokens[k].index = p.index;

						for (int j = 0; j < (int)converted_reset.size(); j++)
							for (int k = (int)converted_reset[j].tokens.size()-1; k >= 0; k--)
								if (converted_reset[j].tokens[k].index == rem[i].index)
									converted_reset[j].tokens[k].index = p.index;

						for (map<petri::iterator, vector<petri::iterator> >::iterator j = result.begin(); j != result.end(); j++)
						{
							for (int k = (int)j->second.size()-1; k >= 0; k--)
							{
								if (j->second[k] > rem[i])
									j->second[k]--;
								else if (j->second[k] == rem[i])
								{
									j->second.erase(j->second.begin() + k);
									j->second.push_back(p);
								}
							}

							sort(j->second.begin(), j->second.end());
							j->second.resize(unique(j->second.begin(), j->second.end()) - j->second.begin());
						}

						graph::erase(rem[i], converted_source);
						graph::erase(rem[i], converted_sink);
						graph::erase(rem[i], converted_reset);
					}

					erase(rem, true);
				}

				if (sink.size() == 1 && converted_sink.size() == 1)
					sink[0] = state::merge(sink[0], converted_sink[0]);
				else if (converted_sink.size() == 1)
					sink = converted_sink;
			}
			else if (composition == sequence)
			{
				if (reset.size() > 0 && converted_reset.size() > 0)
					error("", "only one reset token allowed per sequential", __FILE__, __LINE__);
				else if (reset.size() == 0)
					reset = converted_reset;

				vector<petri::iterator> rem;
				vector<petri::iterator> m;
				if (sink.size() > 1 || converted_source.size() > 1)
				{
					m.push_back(create(place()));

					for (int i = 0; i < (int)sink.size(); i++)
					{
						if (sink[i].tokens.size() > 1)
						{
							petri::iterator pm = create(transition());
							for (int k = 0; k < (int)sink[i].tokens.size(); k++)
								connect(petri::iterator(place::type, sink[i].tokens[k].index), pm);
							connect(pm, m);
						}
						else if (sink[i].tokens.size() == 1)
						{
							petri::iterator pm = petri::iterator(place::type, sink[i].tokens[0].index);
							connect(prev(pm), m);
							connect(m, next(pm));
							for (int k = 0; k < (int)m.size(); k++) {
								places[m[k].index] = place::merge(sequence, places[pm.index], places[m[k].index]);
							}
							rem.push_back(pm);
						}
					}

					for (int i = 0; i < (int)converted_source.size(); i++)
					{
						if (converted_source[i].tokens.size() > 1)
						{
							petri::iterator pm = create(transition());
							for (int k = 0; k < (int)converted_source[i].tokens.size(); k++)
								connect(pm, petri::iterator(place::type, converted_source[i].tokens[k].index));
							connect(m, pm);
						}
						else if (converted_source[i].tokens.size() == 1)
						{
							petri::iterator pm = petri::iterator(place::type, converted_source[i].tokens[0].index);
							connect(prev(pm), m);
							connect(m, next(pm));
							for (int k = 0; k < (int)m.size(); k++) {
								places[m[k].index] = place::merge(sequence, places[m[k].index], places[pm.index]);
							}
							rem.push_back(pm);
						}
					}
				}
				else if (sink.size() == 1 && converted_source.size() == 1)
				{
					if (sink[0].tokens.size() > 1 && converted_source[0].tokens.size() > 1)
					{
						petri::iterator p = create(transition());
						for (int k = 0; k < (int)sink[0].tokens.size(); k++)
							connect(petri::iterator(place::type, sink[0].tokens[k].index), p);
						for (int k = 0; k < (int)converted_source[0].tokens.size(); k++)
							connect(p, petri::iterator(place::type, converted_source[0].tokens[k].index));
					}
					else if (sink[0].tokens.size() >= 1 && converted_source[0].tokens.size() == 1)
					{
						petri::iterator p(place::type, converted_source[0].tokens[0].index);
						vector<petri::iterator> nm = next(p);
						vector<petri::iterator> pm = prev(p);
						for (int k = 0; k < (int)sink[0].tokens.size(); k++)
						{
							m.push_back(petri::iterator(place::type, sink[0].tokens[k].index));
							places[m.back().index] = place::merge(sequence, places[m.back().index], places[p.index]);
							connect(m.back(), nm);
							connect(pm, m.back());
						}
						rem.push_back(p);
					}
					else if (sink[0].tokens.size() == 1 && converted_source[0].tokens.size() >= 1)
					{
						petri::iterator p(place::type, sink[0].tokens[0].index);
						vector<petri::iterator> nm = next(p);
						vector<petri::iterator> pm = prev(p);
						for (int k = 0; k < (int)converted_source[0].tokens.size(); k++)
						{
							m.push_back(petri::iterator(place::type, converted_source[0].tokens[k].index));
							places[m.back().index] = place::merge(sequence, places[p.index], places[m.back().index]);
							connect(m.back(), nm);
							connect(pm, m.back());
						}
						rem.push_back(p);
					}
				}

				sort(rem.begin(), rem.end());
				rem.resize(unique(rem.begin(), rem.end()) - rem.begin());
				reverse(rem.begin(), rem.end());

				for (int i = 0; i < (int)rem.size(); i++)
				{
					for (int j = 0; j < (int)converted_sink.size(); j++)
						for (int k = (int)converted_sink[j].tokens.size()-1; k >= 0; k--)
						{
							if (converted_sink[j].tokens[k].index == rem[i].index)
							{
								for (int l = 0; l < (int)m.size(); l++)
								{
									converted_sink[j].tokens.push_back(converted_sink[j].tokens[k]);
									converted_sink[j].tokens.back().index = m[l].index;
								}
								converted_sink[j].tokens.erase(converted_sink[j].tokens.begin() + k);
							}
						}

					for (int j = 0; j < (int)reset.size(); j++)
						for (int k = (int)reset[j].tokens.size()-1; k >= 0; k--)
						{
							if (reset[j].tokens[k].index == rem[i].index)
							{
								for (int l = 0; l < (int)m.size(); l++)
								{
									reset[j].tokens.push_back(reset[j].tokens[k]);
									reset[j].tokens.back().index = m[l].index;
								}
								reset[j].tokens.erase(reset[j].tokens.begin() + k);
							}
						}

					for (int j = 0; j < (int)source.size(); j++)
						for (int k = (int)source[j].tokens.size()-1; k >= 0; k--)
						{
							if (source[j].tokens[k].index == rem[i].index)
							{
								for (int l = 0; l < (int)m.size(); l++)
								{
									source[j].tokens.push_back(source[j].tokens[k]);
									source[j].tokens.back().index = m[l].index;
								}
								source[j].tokens.erase(source[j].tokens.begin() + k);
							}
						}

					for (map<petri::iterator, vector<petri::iterator> >::iterator j = result.begin(); j != result.end(); j++)
					{
						for (int k = (int)j->second.size()-1; k >= 0; k--)
						{
							if (j->second[k] > rem[i])
								j->second[k]--;
							else if (j->second[k] == rem[i])
							{
								j->second.erase(j->second.begin() + k);
								j->second.insert(j->second.end(), m.begin(), m.end());
							}
						}

						sort(j->second.begin(), j->second.end());
						j->second.resize(unique(j->second.begin(), j->second.end()) - j->second.begin());
					}
				}

				sink = converted_sink;
				erase(rem, true);
			}

			return result;
		}
	}

	virtual vector<state> consolidate(vector<state> to, vector<state> from=vector<state>(), bool out = false) {
		if (from.empty()) {
			from.push_back(state());
			from.back().tokens.push_back(create(place::type).index);
		}
		vector<petri::iterator> rem;

		for (auto k = from.begin(); k != from.end(); k++) {
			vector<petri::iterator> con;
			for (auto i = k->tokens.begin(); i != k->tokens.end(); i++) {
				con.push_back(petri::iterator(place::type, i->index));
			}
			for (auto i = to.begin(); i != to.end(); i++) {
				if (i->tokens.size() > 1) {
					vector<petri::iterator> t;
					if (out) {
						t = connect(create(transition::type), con);
					} else {
						t.push_back(connect(con, create(transition::type)));
					}

					for (auto j = i->tokens.begin(); j != i->tokens.end(); j++) {
						if (out) {
							connect(petri::iterator(place::type, j->index), t);
						} else {
							connect(t, petri::iterator(place::type, j->index));
						}
					}
				} else if (i->tokens.size() == 1) {
					petri::iterator loc(place::type, i->tokens[0].index);
					connect(prev(loc), con);
					connect(con, next(loc));
					for (auto j = con.begin(); j != con.end(); j++) {
						places[j->index] = place::merge(choice, places[j->index], places[loc.index]);
					}
					rem.push_back(loc);
				}
			}
		}

		sort(rem.begin(), rem.end());
		rem.erase(unique(rem.begin(), rem.end()), rem.end());

		for (int i = (int)rem.size()-1; i >= 0; i--) {
			erase(rem[i]);
			erase(rem[i], from);
		}

		return from;
	}

	virtual vector<vector<petri::iterator> > cycles() const
	{
		vector<vector<petri::iterator> > curr;
		vector<vector<petri::iterator> > result;
		for (int i = 0; i < (int)source.size(); i++)
			for (int j = 0; j < (int)source[i].tokens.size(); j++)
				curr.push_back(vector<petri::iterator>(1, petri::iterator(place::type, source[i].tokens[j].index)));

		sort(curr.begin(), curr.end());
		curr.resize(unique(curr.begin(), curr.end()) - curr.begin());

		while (curr.size() > 0)
		{
			vector<petri::iterator> x = curr.back();
			curr.pop_back();

			vector<petri::iterator> n = next(x.back());
			for (int j = 0; j < (int)n.size(); j++)
			{
				vector<petri::iterator>::iterator loopback = find(x.begin(), x.end(), n[j]);
				if (loopback != x.end())
				{
					result.push_back(x);
					result.back().erase(result.back().begin(), result.back().begin() + (loopback - x.begin()));
				}
				else
				{
					curr.push_back(x);
					curr.back().push_back(n[j]);
				}
			}
		}

		return result;
	}

	virtual bool reduce(bool proper_nesting = true, bool aggressive = false)
	{
		bool result = false;
		bool change = true;
		while (change)
		{
			change = false;

			for (petri::iterator i(transition::type, 0); i < (int)transitions.size() && !change; )
			{
				vector<petri::iterator> n = next(i);
				vector<petri::iterator> p = prev(i);

				sort(n.begin(), n.end());
				sort(p.begin(), p.end());

				bool affect = false;
				// If it doesn't have any input places, then we need to add one.
				if (!affect && p.size() == 0)
				{
					p.push_back(create(place::type));
					connect(p, i);
					affect = true;
				}

				// If it doesn't have any output places, then we need to add one.
				if (!affect && n.size() == 0)
				{
					n.push_back(create(place::type));
					connect(i, n);
					affect = true;
				}

				// A transition will never be enabled if it is infeasible.
				// These transitions may be removed while preserving proper nesting, token flow
				// stability, non interference, and deadlock freedom. At this point, it is not
				// possible for this transition to be in the source list.
				if (!affect && transitions[i.index].is_infeasible())
				{
					erase(i);
					affect = true;
				}

				// Vacuous transitions may be pinched while preserving token flow,
				// stability, non interference, and deadlock freedom. However, proper nesting is not necessarily
				// preserved. We have to take special precautions if we want to preserver proper nesting.
				if (!affect && transitions[i.index].is_vacuous())
				{
					if (!proper_nesting)
					{
						pinch(i);
						affect = true;
					}
					else
					{
						vector<petri::iterator> np = next(p);
						vector<petri::iterator> pn = prev(n);

						if (p.size() == 1 && n.size() == 1 && (np.size() == 1 || pn.size() == 1))
						{
							pinch(i);
							affect = true;
						}
						else
						{
							vector<petri::iterator> nn = next(n);
							vector<petri::iterator> nnp = next(np);
							vector<petri::iterator> pp = prev(p);
							vector<petri::iterator> ppn = prev(pn);

							if ((n.size() == 1 && nn.size() == 1 && nnp.size() == 1 && np.size() == 1) ||
								(p.size() == 1 && pp.size() == 1 && ppn.size() == 1 && pn.size() == 1))
							{
								pinch(i);
								affect = true;
							}
						}
					}
				}

				if (!affect)
					i++;
				else
					change = true;
			}

			for (petri::iterator i(place::type, 0); i < (int)places.size() && !change; )
			{
				bool i_is_reset = is_reset(i);

				vector<petri::iterator> n = next(i);
				vector<petri::iterator> p = prev(i);

				sort(n.begin(), n.end());
				sort(p.begin(), p.end());

				bool affect = false;

				// We know a place will never be marked if it is not in the initial marking and it has no input arcs.
				// This means that its output transitions will never fire.
				if (p.size() == 0 && (!i_is_reset || n.size() == 0))
				{
					erase(n);
					erase(i);
					affect = true;
				}

				// Check to see if there are any excess places whose existence doesn't affect the behavior of the circuit
				for (petri::iterator j = i+1; j < (int)places.size(); )
				{
					bool j_is_reset = is_reset(j);

					vector<petri::iterator> n2 = next(j);
					vector<petri::iterator> p2 = prev(j);

					sort(n2.begin(), n2.end());
					sort(p2.begin(), p2.end());

					if (n == n2 && p == p2 && i_is_reset == j_is_reset)
					{
						erase(j);
						affect = true;
					}
					else
						j++;
				}

				if (!affect)
					i++;
				else
					change = true;
			}

			// TODO Once internal parallelism stops assuming isochronic forks we can re-enable this for active transitions
			if (aggressive) {
				vector<petri::iterator> left;
				vector<petri::iterator> right;

				vector<vector<petri::iterator> > n, p;
				vector<vector<pair<vector<petri::iterator>, vector<petri::iterator> > > > nx, px;

				for (petri::iterator i(transition::type, 0); i < (int)transitions.size() && !change; i++)
				{
					n.push_back(next(i));
					p.push_back(prev(i));

					sort(n.back().begin(), n.back().end());
					sort(p.back().begin(), p.back().end());

					nx.push_back(vector<pair<vector<petri::iterator>, vector<petri::iterator> > >());
					px.push_back(vector<pair<vector<petri::iterator>, vector<petri::iterator> > >());
					for (int j = 0; j < (int)n.back().size(); j++)
					{
						nx.back().push_back(pair<vector<petri::iterator>, vector<petri::iterator> >(prev(n.back()[j]), next(n.back()[j])));
						sort(nx.back().back().first.begin(), nx.back().back().first.end());
						sort(nx.back().back().second.begin(), nx.back().back().second.end());
					}
					for (int j = 0; j < (int)p.back().size(); j++)
					{
						px.back().push_back(pair<vector<petri::iterator>, vector<petri::iterator> >(prev(p.back()[j]), next(p.back()[j])));
						sort(px.back().back().first.begin(), px.back().back().first.end());
						sort(px.back().back().second.begin(), px.back().back().second.end());
					}

					for (petri::iterator j = i-1; j >= 0 && !change; j--)
					{
						// Find internally conditioned transitions. Transitions are internally conditioned if they are the same type
						// share all of the same input and output places.
						if (n[j.index] == n[i.index] && p[j.index] == p[i.index])
						{
							transitions[j.index] = transition::merge(choice, transitions[i.index], transitions[j.index]);
							erase(i);
							change = true;
						}

						// Find internally parallel transitions. A pair of transitions A and B are internally parallel if
						// they are the same type, have disjoint sets of input and output places that share a single input
						// or output transition and have no output or input transitions other than A or B.
						else if (vector_intersection_size(n[i.index], n[j.index]) == 0 && vector_intersection_size(p[i.index], p[j.index]) == 0 && nx[i.index] == nx[j.index] && px[i.index] == px[j.index])
						{
							transitions[j.index] = transition::merge(parallel, transitions[i.index], transitions[j.index]);
							vector<petri::iterator> tocut;
							tocut.push_back(i);
							tocut.insert(tocut.end(), n[i.index].begin(), n[i.index].end());
							tocut.insert(tocut.end(), p[i.index].begin(), p[i.index].end());
							erase(tocut);
							change = true;
						}
					}
				}
			}

			result = (result or change);
		}

		return result;
	}

	virtual bool is_floating(petri::iterator n) const {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < (int)arcs[i].size(); j++) {
				if (arcs[i][j].from == n || arcs[i][j].to == n) {
					return false;
				}
			}
		}
		return true;
	}

	virtual void set_split_group(int composition, petri::iterator node, split_group g) const {
		vector<split_group> *groups = split_groups_iter(composition, node);
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

	virtual split_group get_split_group(int composition, petri::iterator node, int split) const {
		vector<split_group> groups = split_groups_of(composition, node, false);
		auto pos = lower_bound(groups.begin(), groups.end(), split);
		if (pos != groups.end() and pos->split == split) {
			return *pos;
		}
		return split_group();
	}

	virtual vector<split_group> *split_groups_iter(int composition, petri::iterator node) const {
		if (node.index < 0) {
			return nullptr;
		}

		if (node.type == place::type) {
			return &places[node.index].splits[composition];
		}
		return &transitions[node.index].splits[composition];
	}

	virtual vector<split_group> split_groups_of(int composition, petri::iterator node, bool update = true) const {
		if (node.index < 0) {
			if (node.type == transition::type and composition == choice and (int)reset.size() > 1) {
				return vector<split_group>(1, split_group(-1, (int)reset.size(), vector<int>(1, node.index)));
			}
			return vector<split_group>();
		}

		if (update and !split_groups_ready) {
			compute_split_groups();
		}
 
		if (node.type == place::type) {
			return places[node.index].splits[composition];
		}
		return transitions[node.index].splits[composition];
	}

	virtual vector<split_group> split_groups_of(int composition, int group_operation, int branch_operation, vector<petri::iterator> nodes) const {
		vector<split_group> groups;
		if (nodes.empty()) {
			return groups;
		}
		groups = split_groups_of(composition, nodes[0]);
		for (int i = 1; i < (int)nodes.size(); i++) {
			groups = petri::merge(group_operation, branch_operation, groups, split_groups_of(composition, nodes[i]));
		}
		return groups;
	}

	// a is sometimes in choice with b if firing a does not imply a firing on b
	// a is always in choice with b if firing a implies b will not fire
	// a and b are sometimes in bidirectional choice if firing a does not imply a
	// firing on b **or** visa-versa.
	virtual bool is_excludes(petri::iterator a, petri::iterator b, bool always=false, bool update=true) const {
		if (a == b) {
			return true;
		}

		auto ac = split_groups_of(choice, a, update);
		auto bc = split_groups_of(choice, b, update);

		return compare(split_group::NEGATIVE_DIFFERENCE, split_group::DIFFERENCE, ac, bc)
			and (not always or not is_implies(a, b, false, update));
	}


	// a is sometimes in choice with b if firing a does not imply a firing on b
	// a is always in choice with b if firing a implies b will not fire
	// a and b are sometimes in bidirectional choice if firing a does not imply a
	// firing on b **or** visa-versa.
	virtual bool is_implies(petri::iterator a, petri::iterator b, bool always=false, bool update=true) const {
		if (a == b) {
			return true;
		}

		auto ac = split_groups_of(choice, a, update);
		auto bc = split_groups_of(choice, b, update);

		return not compare(split_group::INTERSECT, split_group::SYMMETRIC_DIFFERENCE, ac, bc)
			and (not always or not is_excludes(a, b, false, update));
	}

	// a is sometimes in choice with b if firing a does not imply a firing on b
	// a is always in choice with b if firing a implies b will not fire
	// a and b are sometimes in bidirectional choice if firing a does not imply a
	// firing on b **or** visa-versa.
	virtual bool is_choice(petri::iterator a, petri::iterator b, bool always=false, bool update=true) const {
		if (a == b) {
			return false;
		}

		auto ac = split_groups_of(choice, a, update);
		auto bc = split_groups_of(choice, b, update);
		return (not always and compare(split_group::INTERSECT, split_group::NOT_EQUAL, ac, bc))
			or (always and compare(split_group::INTERSECT, split_group::SYMMETRIC_DIFFERENCE, ac, bc)
				and not is_parallel(a, b, false, update));
	}

	// a is sometimes in parallel if there exists a state with both a and b.
	// This does not imply that all states with one also have the other. This
	// relationship is bidirectional.
	virtual bool is_parallel(petri::iterator a, petri::iterator b, bool always=false, bool update=true) const {
		if (a == b) {
			return false;
		}

		auto ap = split_groups_of(parallel, a, update);
		auto bp = split_groups_of(parallel, b, update);
		return compare(split_group::INTERSECT, split_group::SYMMETRIC_DIFFERENCE, ap, bp)
			 and (not always or not is_choice(a, b, false, update));
	}

	virtual bool is_sequence(petri::iterator a, petri::iterator b, bool always=false, bool update=true) const {
		if (a == b) {
			return false;
		}

		auto ap = split_groups_of(parallel, a, update);
		auto bp = split_groups_of(parallel, b, update);
		auto ac = split_groups_of(choice, a, update);
		auto bc = split_groups_of(choice, b, update);

		return compare(split_group::INTERSECT, split_group::SUBSET_EQUAL, ap, bp)
			and compare(split_group::INTERSECT, split_group::SUBSET_EQUAL, ac, bc)
			and (not always or not is_choice(a, b, false, update));
	}

	virtual bool is(int composition, petri::iterator a, petri::iterator b, bool always=false, bool bidir=false, bool update=true) const {
		if (composition == sequence) {
			return is_sequence(a, b, always, update) and (not bidir or is_sequence(b, a, always, update));
		} else if (composition == choice) {
			return is_choice(a, b, always, update);
		} else if (composition == implies) {
			return is_implies(a, b, always, update) and (not bidir or is_implies(b, a, always, update));
		} else if (composition == excludes) {
			return is_excludes(a, b, always, update) or (bidir and is_excludes(b, a, always, update));
		}
		return is_parallel(a, b, always, update);
	}

	// This assumes that a and b represent partial states. IE, there exists a set
	// of states which each contain all nodes in a and a set of states which each
	// contain all nodes in b.
	virtual bool is(int composition, std::vector<petri::iterator> a, std::vector<petri::iterator> b, bool always=false, bool bidir=false) const {
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

		if (composition == parallel or composition == implies) {
			for (auto i = a.begin(); i != a.end(); i++) {
				for (auto j = b.begin(); j != b.end(); j++) {
					if (*i != *j and not is(composition, *i, *j, always, bidir)) {
						return false;
					}
				}
			}
			return true;
		} else if (composition == choice or composition == sequence or composition == excludes) {
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
			if (composition == sequence) {
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

		if (composition == sequence) {
			cout << "split a: " << ::to_string(split_groups_of(parallel, split_group::INTERSECT, split_group::UNION, a)) << endl;
			cout << "split b: " << ::to_string(split_groups_of(parallel, split_group::INTERSECT, split_group::UNION, b)) << endl;
			cout << "parallel: " << compare(split_group::INTERSECT, split_group::SUBSET_EQUAL,
					split_groups_of(parallel, split_group::INTERSECT, split_group::UNION, a),
					split_groups_of(parallel, split_group::INTERSECT, split_group::UNION, b)) << endl;
			cout << "split a: " << ::to_string(split_groups_of(choice, split_group::UNION, split_group::INTERSECT, a)) << endl;
			cout << "split b: " << ::to_string(split_groups_of(choice, split_group::UNION, split_group::INTERSECT, b)) << endl;
			cout << "choice: " << compare(split_group::INTERSECT, split_group::SUBSET_EQUAL,
					split_groups_of(choice, split_group::UNION, split_group::INTERSECT, a),
					split_groups_of(choice, split_group::UNION, split_group::INTERSECT, b)) << endl;
			return (compare(split_group::INTERSECT, split_group::SUBSET_EQUAL,
					split_groups_of(parallel, split_group::INTERSECT, split_group::UNION, a),
					split_groups_of(parallel, split_group::INTERSECT, split_group::UNION, b))
				and compare(split_group::INTERSECT, split_group::SUBSET_EQUAL,
					split_groups_of(choice, split_group::UNION, split_group::INTERSECT, a),
					split_groups_of(choice, split_group::UNION, split_group::INTERSECT, b)));
		}

		vector<split_group> Ga, Gb;
 		if (composition == parallel) {
			Ga = split_groups_of(parallel, split_group::INTERSECT, split_group::UNION, a);
			Gb = split_groups_of(parallel, split_group::INTERSECT, split_group::UNION, b);
		} else {
			Ga = split_groups_of(choice, split_group::UNION, split_group::INTERSECT, a);
			Gb = split_groups_of(choice, split_group::UNION, split_group::INTERSECT, b);
		}

		return compare(split_group::INTERSECT, split_group::SYMMETRIC_DIFFERENCE, Ga, Gb);*/
	}

	// Find all partial state pairs (for each node in v0 and v1 respectively) that are ordered (not in parallel).
	virtual vector<array<vector<vector<petri::iterator> >, 2> > deinterfere_choice(vector<petri::iterator> v0, vector<petri::iterator> v1) {
		vector<array<vector<vector<petri::iterator> >, 2> > stack;
		vector<array<vector<vector<petri::iterator> >, 2> > next;

		stack.resize(1);
		for (int i = 0; i < (int)v0.size(); i++) {
			stack.back()[0].push_back(vector<petri::iterator>(1, v0[i]));
		}
		for (int i = 0; i < (int)v1.size(); i++) {
			stack.back()[1].push_back(vector<petri::iterator>(1, v1[i]));
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

	// Find all partial state pairs (for v0 and v1 respectively) that are ordered (not in parallel).
	virtual vector<array<vector<petri::iterator>, 2> > deinterfere(vector<petri::iterator> v0, vector<petri::iterator> v1) {
		sort(v0.begin(), v0.end());
		sort(v1.begin(), v1.end());
		vector<petri::iterator> v0p, v1p;
		for (int j = 0; j < 2; j++) {
			for (auto i = begin(j); i != end(j); i++) {
				if (find(v1.begin(), v1.end(), i) == v1.end() and is(parallel, vector<petri::iterator>(1, i), v0)) {
					v0p.push_back(i);
				}
				if (find(v0.begin(), v0.end(), i) == v0.end() and is(parallel, vector<petri::iterator>(1, i), v1)) {
					v1p.push_back(i);
				}
			}
		}

		vector<array<vector<petri::iterator>, 2> > result;
		if (vector_intersects(v0, v1)) {
			return result;
		}
		
		if (not is(parallel, v0, v1)) {
			result.push_back({v0, v1});
			return result;
		}

		for (auto i = v0p.begin(); i != v0p.end(); i++) {
			if (not is(parallel, vector<petri::iterator>(1, *i), v1)) {
				result.push_back({v0, v1});
				result.back()[0].push_back(*i);
			}
		}

		for (auto i = v1p.begin(); i != v1p.end(); i++) {
			if (not is(parallel, vector<petri::iterator>(1, *i), v0)) {
				result.push_back({v0, v1});
				result.back()[1].push_back(*i);
			}
		}
		
		for (auto i = v0p.begin(); i != v0p.end(); i++) {
			for (auto j = v1p.begin(); j != v1p.end(); j++) {
				if (*i != *j and not is(parallel, *i, *j)) {
					result.push_back({v0, v1});
					result.back()[0].push_back(*i);
					result.back()[1].push_back(*j);
				}
			}
		}

		return result;
	}

	// Nodes can be simultaneously composed in both parallel and conditional.
	// This function selects nodes into groups based upon a composition operator
	// (ex. conditional groups of parallel nodes for the "parallel" composition).
	// If the "strict" flag is set, then this function not only separates nodes
	// that are not composed as desired, but also separates nodes that are
	// compared as desired and also composed as not desired. For example, strict
	// will also separate nodes that are simultaneously composed in parallel and
	// conditional.
	virtual vector<vector<petri::iterator> > select(int composition, vector<petri::iterator> nodes, bool always=false, bool invert=false) {
		// ~always & ~invert - separate nodes that aren't sometimes composed as requested
		// ~always &  invert - separate nodes that are sometimes composed as the opposite of requested
		//  always & ~invert - separate nodes that aren't always composed as requested.
		//                     For example if parallel requested, then this breaks sequence
		//                     and choice relations.
		//  always &  invert - separate nodes that are always composed as the opposite of requested

		vector<vector<petri::iterator> > result;

		// This is the problem of identifying all maximal cliques in the
		// graph constructed using the nodes in "from" as vertices and
		// creating edges between each pair of parallel nodes. This is an
		// NP-complete problem and we are solving it using the Bron–Kerbosch
		// algorithm.
		struct BronKerboschFrame {
			vector<petri::iterator> R, P, X;
		};

		int opposite = composition;
		if (composition == parallel) {
			opposite = choice;
		} else if (composition == choice) {
			opposite = parallel;
		} else if (composition == implies) {
			opposite = excludes;
		} else if (composition == excludes) {
			opposite = implies;
		}

		vector<BronKerboschFrame> frames;
		frames.push_back(BronKerboschFrame());
		frames.back().P = nodes;

		while (not frames.empty()) {
			auto frame = frames.back();
			frames.pop_back();

			if (frame.P.empty() and frame.X.empty()) {
				// Then we've found a maximal clique
				sort(frame.R.begin(), frame.R.end());
				result.push_back(frame.R);
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
	vector<vector<petri::iterator> > group(int composition, vector<vector<petri::iterator> > nodes, bool always=false, bool invert=false) {
		// ~always & ~invert - group nodes that are sometimes composed as requested
		// ~always &  invert - group nodes that aren't sometimes composed as the opposite of requested
		//  always & ~invert - group nodes that are always composed as requested.
		//  always &  invert - group nodes that aren't always composed as the opposite of requested

		struct BronKerboschFrame {
			vector<int> R, P, X;
		};

		int opposite = composition;
		if (composition == parallel) {
			opposite = choice;
		} else if (composition == choice) {
			opposite = parallel;
		} else if (composition == implies) {
			opposite = excludes;
		} else if (composition == excludes) {
			opposite = implies;
		}

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
					nodes.push_back(vector<petri::iterator>());
					for (auto i = frame.R.begin(); i != frame.R.end(); i++) {
						nodes.back().insert(nodes.back().end(), nodes[*i].begin(), nodes[*i].end());
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

	vector<vector<petri::iterator> > complete(int composition, vector<vector<petri::iterator> > nodes) {
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
			vector<split_group> A_groups = split_groups_of(choice, split_group::UNION, split_group::INTERSECT, nodes[i]);
			vector<split_group> B_groups;

			cout << "starting search " << ::to_string(nodes[i]) << " " << ::to_string(A_groups) << endl;
			for (int j = 0; j < (int)nodes.size(); j++) {
				cout << "checking i=" << i << ":" << ::to_string(nodes[i]) << " and j=" << j << ":" << ::to_string(nodes[j]) << endl;	
				if (i != j and vector_is_subset_of(nodes[i], nodes[j])) {
					cout << "found subset" << endl;
					// 3. Find the conditional split groups of Bi & ~A union groups, intersect branches.
					vector<petri::iterator> Bj = vector_difference(nodes[j], nodes[i]);
					cout << "j-i" << ::to_string(Bj) << endl;

					vector<split_group> Bj_groups = split_groups_of(choice, split_group::UNION, split_group::INTERSECT, Bj);
					cout << "groups:" << ::to_string(Bj_groups) << endl;
					
					// 4. Find the conditional branches that B belongs to that A does
					// not. Merge, intersect groups, subtract branches A-Bi for each Bi
					Bj_groups = petri::merge(split_group::INTERSECT, split_group::DIFFERENCE, A_groups, Bj_groups);
					cout << "after intersect:" << ::to_string(Bj_groups) << endl;

					// 5. merge, union groups, union branches across all [Bi]
					B_groups = petri::merge(split_group::UNION, split_group::UNION, B_groups, Bj_groups);
					cout << "after union:" << ::to_string(B_groups) << endl;
				}
			}
			cout << "done search:" << ::to_string(B_groups) << endl;
			if (B_groups.empty()) {
				continue;
			}

			// 5. Add these transitions to A
			vector<petri::iterator> A = nodes[i];
			for (auto group = B_groups.begin(); group != B_groups.end(); group++) {
				for (auto branch = group->branch.begin(); branch != group->branch.end(); branch++) {
					A.push_back(petri::iterator(transition::type, *branch));
				}
			}
			sort(A.begin(), A.end());
			A.erase(unique(A.begin(), A.end()), A.end());

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

			nodes.erase(nodes.begin() + i);
		}

		return nodes;
	}

	virtual vector<petri::iterator> deselect(const vector<vector<petri::iterator> > &nodes) {
		vector<petri::iterator> result = nodes[0];
		for (int i = 1; i < (int)nodes.size(); i++) {
			result.insert(result.end(), nodes[i].begin(), nodes[i].end());
		}
		sort(result.begin(), result.end());
		result.erase(unique(result.begin(), result.end()), result.end());
		return result;
	}

	virtual vector<vector<petri::iterator> > partials(int composition, vector<petri::iterator> nodes, vector<petri::iterator> other = vector<petri::iterator>()) {
		sort(nodes.begin(), nodes.end());
		nodes.erase(unique(nodes.begin(), nodes.end()), nodes.end());

		if (other.empty()) {
			for (auto i = begin(place::type); i != end(place::type); i++) {
				if (is(composition, vector<petri::iterator>(1, i), nodes)) {
					other.push_back(i);
				}
			}
			for (auto i = begin(transition::type); i != end(transition::type); i++) {
				if (is(composition, vector<petri::iterator>(1, i), nodes)) {
					other.push_back(i);
				}
			}
		}

		// Given the set of nodes in "other" and the set of nodes in "nodes", we
		// need to find all cliques (maximal or not) in the graph created by
		// the requested composition relations.
		vector<vector<petri::iterator> > result;
		list<pair<vector<petri::iterator>, vector<petri::iterator> > > queue;
		queue.push_back(pair<vector<petri::iterator>, vector<petri::iterator> >(nodes, other));
		while (not queue.empty()) {
			auto curr = queue.front();
			queue.pop_front();

			auto k = lower_bound(result.begin(), result.end(), curr.first);
			if (k == result.end() or *k != curr.first) {
				result.insert(k, curr.first);
				for (auto i = curr.second.begin(); i != curr.second.end(); i++) {
					if (is(composition, vector<petri::iterator>(1, *i), curr.first)) {
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

	virtual bool is_reset(petri::iterator i) const {
		if (reset.size() == 0) {
			for (int j = 0; j < (int)source.size(); j++) {
				for (int k = 0; k < (int)source[j].tokens.size(); k++) {
					if (source[j].tokens[k].index == i.index) {
						return true;
					}
				}
			}
		} else {
			for (int j = 0; j < (int)reset.size(); j++) {
				for (int k = 0; k < (int)reset[j].tokens.size(); k++) {
					if (reset[j].tokens[k].index == i.index) {
						return true;
					}
				}
			}
		}
		return false;
	}

	virtual bool is_redundant_to(petri::iterator p0, petri::iterator p1) {
		if (p0 == p1 or not is(parallel, p0, p1)) {
			return false;
		}

		vector<petri::iterator> n = neighbors(p0);
		for (auto ni = n.begin(); ni != n.end(); ni++) {
			if (is(parallel, *ni, p1)) {
				return false;
			}
		}

		if (not is_reset(p0)) {
			if (is_reset(p1)) {
				return false;
			}
			for (auto ri = reset.begin(); ri != reset.end(); ri++) {
				for (auto rj = ri->tokens.begin(); rj != ri->tokens.end(); rj++) {
					petri::iterator p(place::type, rj->index);
					if (p != p0 and p != p1 and is(parallel, p0, p) and is(sequence, p1, p)) {
						return false;
					}
				}
			}
		}

		return true;
	}

	virtual bool is_redundant_to(petri::iterator p0, vector<petri::iterator> p1) {
		for (auto i = p1.begin(); i != p1.end(); i++) {
			if (is_redundant_to(p0, *i)) {
				return true;
			}
		}
		return false;
	}

	virtual bool is_redundant(petri::iterator p0) {
		for (auto i = begin(place::type); i != end(place::type); i++) {
			if (is_redundant_to(p0, i)) {
				//cout << p0 << " is redundant to " << i << endl;
				return true;
			}
		}
		return false;
	}

	virtual vector<petri::iterator> add_redundant(vector<petri::iterator> p) {
		for (auto i = begin(place::type); i != end(place::type); i++) {
			if (is_redundant_to(i, p)) {
				p.push_back(i);
			}
		}
		sort(p.begin(), p.end());
		p.erase(unique(p.begin(), p.end()), p.end());
		return p;
	}

	virtual void erase_redundant() {
		for (auto i = rbegin(place::type); i != rend(place::type); i--) {
			if (is_redundant(i)) {
				//erase(i);
			}
		}
	}

	virtual bool crosses_reset(vector<petri::iterator> pos) {
		bool before_reset = false;
		bool after_reset = false;

		for (auto i = pos.begin(); i != pos.end(); i++) {
			if (i->type == transition::type) {
				bool found = false;
				for (auto group = transitions[i->index].splits[parallel].begin(); group != transitions[i->index].splits[parallel].end() and not found; group++) {
					found = group->split < 0;
				}
				before_reset = before_reset or not found;
				after_reset = after_reset or found;
			} else {
				bool found = false;
				for (auto group = places[i->index].splits[parallel].begin(); group != places[i->index].splits[parallel].end(); group++) {
					if (group->split < 0) {
						found = true;
						for (auto branch = group->branch.begin(); branch != group->branch.end(); branch++) {
							if (*branch == i->index) {
								before_reset = true;
							} else {
								after_reset = true;
							}
						}
					}
				}
				before_reset = before_reset or not found;
			}
		}
		return before_reset and after_reset;
	}

	vector<split_group> invert(int composition, vector<split_group> groups) {
		int split_type = (composition == choice ? place::type : transition::type);
		for (int i = 0; i < (int)groups.size(); i++) {
			vector<petri::iterator> n = next(petri::iterator(split_type, groups[i].split));
			vector<int> branches;
			for (int j = 0; j < (int)n.size(); j++) {
				if (find(groups[i].branch.begin(), groups[i].branch.end(), n[j].index) == groups[i].branch.end()) {
					branches.push_back(n[j].index);
				}
			}
			groups[i].branch.swap(branches);
			branches.clear();
		}
		return groups;
	}

	virtual void print() const {
		for (int i = 0; i < (int)places.size(); i++) {
			cout << "p" << i << ": p" << ::to_string(places[i].splits[place::type]) << " t" << ::to_string(places[i].splits[transition::type]) << "{";
			for (int j = 0; j < (int)places.size(); j++) {
				if (distance(petri::iterator(place::type, i), petri::iterator(place::type, j), false) >= 0) {
					cout << "->p" << j << ":" << distance(petri::iterator(place::type, i), petri::iterator(place::type, j), false) << " ";
				}
				if (distance(petri::iterator(place::type, j), petri::iterator(place::type, i), false) >= 0) {
					cout << "p" << j << "->:" << distance(petri::iterator(place::type, j), petri::iterator(place::type, i), false) << " ";
				}
			}

			for (int j = 0; j < (int)transitions.size(); j++) {
				if (distance(petri::iterator(place::type, i), petri::iterator(transition::type, j), false) >= 0) {
					cout << "->t" << j << ":" << distance(petri::iterator(place::type, i), petri::iterator(transition::type, j), false) << " ";
				}
				if (distance(petri::iterator(transition::type, j), petri::iterator(place::type, i), false) >= 0) {
					cout << "t" << j << "->:" << distance(petri::iterator(transition::type, j), petri::iterator(place::type, i), false) << " ";
				}
			}
			cout << "}" << endl;
		}
		for (int i = 0; i < (int)transitions.size(); i++) {
			cout << "t" << i << ": p" << ::to_string(transitions[i].splits[place::type]) << " t" << ::to_string(transitions[i].splits[transition::type]) << "{";

			for (int j = 0; j < (int)places.size(); j++) {
				if (distance(petri::iterator(transition::type, i), petri::iterator(place::type, j), false) >= 0) {
					cout << "->p" << j << ":" << distance(petri::iterator(transition::type, i), petri::iterator(place::type, j), false) << " ";
				}
				if (distance(petri::iterator(place::type, j), petri::iterator(transition::type, i), false) >= 0) {
					cout << "p" << j << "->:" << distance(petri::iterator(place::type, j), petri::iterator(transition::type, i), false) << " ";
				}
			}

			for (int j = 0; j < (int)transitions.size(); j++) {
				if (distance(petri::iterator(transition::type, i), petri::iterator(transition::type, j), false) >= 0) {
					cout << "->t" << j << ":" << distance(petri::iterator(transition::type, i), petri::iterator(transition::type, j)) << " ";
				}
				if (distance(petri::iterator(transition::type, j), petri::iterator(transition::type, i), false) >= 0) {
					cout << "t" << j << "->:" << distance(petri::iterator(transition::type, j), petri::iterator(transition::type, i), false) << " ";
				}
			}
			cout << "}" << endl;
		}
	}
};

}
