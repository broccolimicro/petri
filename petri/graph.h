#pragma once

#include <array>

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>
#include <common/index_vector.h>
#include <common/mapping.h>

#include "state.h"
#include "iterator.h"
#include "node.h"
#include "composition.h"

namespace petri {

// An arc represents a directed connection between two nodes in a Petri net
// It stores references to both the source ('from') and destination ('to') nodes using iterators
// Arcs are the edges in the bipartite graph structure of a Petri net, connecting
// places to transitions or transitions to places (never place-to-place or transition-to-transition)
// Used for:
// - Modeling flow of tokens and control in the Petri net
// - Defining the topology and connectivity of the net
// - Supporting graph traversal and reachability analysis
struct arc {
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
// A comprehensive implementation of Petri nets for modeling concurrent and distributed systems
// 
// The graph template represents a complete Petri net with:
// - Places (represented by the place template parameter)
// - Transitions (represented by the transition template parameter)
// - Tokens (represented by the token template parameter)
// - States (collections of tokens, represented by the state template parameter)
// - Arcs connecting places to transitions and transitions to places
//
// This class provides extensive functionality for:
// - Creating, modifying, and analyzing Petri net structures
// - Navigation and traversal of the net (next, prev, neighbors functions)
// - Structural analysis (cycles, reachability, distance calculations)
// - Relationship analysis between nodes (parallelism, choice, sequence)
// - Graph transformations and reductions
// - Split group analysis for understanding choice and parallel execution patterns
template <class place, class transition, class state>
struct graph {
	index_vector<place> places;
	index_vector<transition> transitions;
	// index by from.type
	array<vector<arc>, 2> arcs;
	vector<state> reset;

	graph() {
	}

	virtual ~graph() {
	}

	bool is_valid(petri::iterator i) const {
		if (i.type == place::type) {
			return places.is_valid(i.index);
		}
		return transitions.is_valid(i.index);
	}

	virtual std::vector<enabled_transition> find_enabled(int toType, std::vector<petri::iterator> tokens) {
		vector<enabled_transition> result;
		vector<int> disabled;

		result.reserve(tokens.size()*2);
		disabled.reserve(transitions.size());
		for (const arc &a : arcs[1-toType]) {
			// Check to see if we haven't already determined that this transition can't be enabled
			auto d = lower_bound(disabled.begin(), disabled.end(), a.to.index);
			if (d != disabled.end() and *d == a.to.index) {
				continue;
			}

			// Find the index of this transition (if any) in the result pool
			typename vector<enabled_transition>::iterator e = lower_bound(result.begin(), result.end(), enabled_transition(a.to.index));
			bool e_invalid = (e == result.end() or e->index != a.to.index);

			// Check to see if there is any token at the input place of this arc and make sure that
			// this token has not already been consumed by this particular transition
			// Also since we only need one token per arc, we can stop once we've found a token
			bool found = false;
			for (int j = 0; j < (int)tokens.size() && !found; j++) {
				if (a.from == tokens[j] and
					(e_invalid or find(e->tokens.begin(), e->tokens.end(), j) == e->tokens.end())) {
					// We are safe to add this to the list of possibly enabled transitions
					found = true;
					if (e_invalid) {
						e = result.insert(e, enabled_transition(a.to.index));
					}

					e->tokens.push_back(j);
				}
			}

			// If we didn't find a token at the input place, then we know that this transition can't
			// be enabled. So lets remove this from the list of possibly enabled transitions and
			// remember as much in the disabled list.
			if (not found and e != result.end()) {
				disabled.insert(d, a.to.index);
				if (not e_invalid) {
					result.erase(e);
				}
			}
		}

		return result;
	}

	virtual std::vector<enabled_transition> find_renabled(int fromType, std::vector<petri::iterator> tokens) {
		vector<enabled_transition> result;
		vector<int> disabled;

		result.reserve(tokens.size()*2);
		disabled.reserve(transitions.size());
		for (const arc &a : arcs[fromType]) {
			// Check to see if we haven't already determined that this transition can't be enabled
			auto d = lower_bound(disabled.begin(), disabled.end(), a.from.index);
			if (d != disabled.end() and *d == a.from.index) {
				continue;
			}

			// Find the index of this transition (if any) in the result pool
			typename vector<enabled_transition>::iterator e = lower_bound(result.begin(), result.end(), enabled_transition(a.from.index));
			bool e_invalid = (e == result.end() or e->index != a.from.index);

			// Check to see if there is any token at the input place of this arc and make sure that
			// this token has not already been consumed by this particular transition
			// Also since we only need one token per arc, we can stop once we've found a token
			bool found = false;
			for (int j = 0; j < (int)tokens.size() and not found; j++) {
				if (a.to == tokens[j] and
					(e_invalid or find(e->tokens.begin(), e->tokens.end(), j) == e->tokens.end())) {
					// We are safe to add this to the list of possibly enabled transitions
					found = true;
					if (e_invalid) {
						e = result.insert(e, enabled_transition(a.from.index));
					}
					e->tokens.push_back(j);
				}
			}

			// If we didn't find a token at the input place, then we know that this transition can't
			// be enabled. So lets remove this from the list of possibly enabled transitions and
			// remember as much in the disabled list.
			if (not found) {
				disabled.insert(d, a.from.index);
				if (not e_invalid) {
					result.erase(e);
				}
			}
		}

		return result;
	}

	bool is_split(petri::iterator n) {
		int count = 0;
		for (int i = 0; i < (int)arcs[n.type].size(); i++) {
			if (arcs[n.type][i].from.index == n.index and ++count > 1) {
				return true;
			}
		}
		return false;
	}

	bool is_merge(petri::iterator n) {
		int count = 0;
		for (int i = 0; i < (int)arcs[1-n.type].size(); i++) {
			if (arcs[1-n.type][i].to.index == n.index and ++count > 1) {
				return true;
			}
		}
		return false;
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

	virtual const arc& arc_at(petri::iterator arc_iter) const {
		return this->arcs[arc_iter.type][arc_iter.index];
	}

	virtual arc arc_at(petri::iterator arc_iter) {
		return this->arcs[arc_iter.type][arc_iter.index];
	}

	virtual petri::iterator arc_between(petri::iterator from, petri::iterator to) const {
		for (petri::iterator &out_arc : this->out(from)) {
			if (this->arc_at(out_arc).to == to) {
					return out_arc;
			}
		}

		// Arc not found
		return petri::iterator();
	}

	vector<petri::iterator> get_places() const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)places.size(); i++) {
			if (places.is_valid(i)) {
				result.push_back(petri::iterator(place::type, i));
			}
		}
		return result;
	}

	vector<petri::iterator> get_transitions() const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)transitions.size(); i++) {
			if (transitions.is_valid(i)) {
				result.push_back(petri::iterator(transition::type, i));
			}
		}
		return result;
	}

	virtual petri::iterator create_at(place p, int index) {
		places.emplace_at(index, p);
		return petri::iterator(place::type, index);
	}

	virtual petri::iterator create_at(transition t, int index) {
		transitions.emplace_at(index, t);
		return petri::iterator(transition::type, index);
	}

	virtual petri::iterator create(place p) {
		return petri::iterator(place::type, (int)places.emplace(p));
	}

	virtual petri::iterator create(transition t) {
		return petri::iterator(transition::type, (int)transitions.emplace(t));
	}

	virtual petri::iterator create(int n) {
		if (n == place::type)
			return create(place());
		else if (n == transition::type)
			return create(transition());
		else
			return petri::iterator();
	}

	virtual vector<petri::iterator> create(vector<place> p) {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)p.size(); i++) {
			result.push_back(petri::iterator(place::type, (int)places.emplace(p[i])));
		}
		return result;
	}

	virtual vector<petri::iterator> create(vector<transition> t) {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)t.size(); i++) {
			result.push_back(petri::iterator(transition::type, (int)transitions.emplace(t[i])));
		}
		return result;
	}

	virtual vector<petri::iterator> create(place p, int num) {
		vector<petri::iterator> result;
		for (int i = 0; i < num; i++) {
			result.push_back(petri::iterator(place::type, (int)places.emplace(p)));
		}
		return result;
	}

	virtual vector<petri::iterator> create(transition t, int num) {
		vector<petri::iterator> result;
		for (int i = 0; i < num; i++) {
			result.push_back(petri::iterator(transition::type, (int)transitions.emplace(t)));
		}
		return result;
	}

	virtual vector<petri::iterator> create(int n, int num) {
		if (n == place::type)
			return create(place(), num);
		else if (n == transition::type)
			return create(transition(), num);
		else
			return vector<petri::iterator>();
	}

	virtual pair<vector<petri::iterator>, vector<petri::iterator> > erase(petri::iterator n) {
		pair<vector<petri::iterator>, vector<petri::iterator> > result;
		for (int i = (int)arcs[n.type].size()-1; i >= 0; i--) {
			if (arcs[n.type][i].from.index == n.index) {
				result.second.push_back(arcs[n.type][i].to);
				arcs[n.type].erase(arcs[n.type].begin() + i);
			}
		}
		for (int i = (int)arcs[1-n.type].size()-1; i >= 0; i--) {
			if (arcs[1-n.type][i].to.index == n.index) {
				result.first.push_back(arcs[1-n.type][i].from);
				arcs[1-n.type].erase(arcs[1-n.type].begin() + i);
			}
		}

		if (n.type == place::type) {
			for (int j = 0; j < (int)reset.size(); j++) {
				for (int i = (int)reset[j].tokens.size()-1; i >= 0; i--) {
					if (reset[j].tokens[i].index == n.index) {
						reset[j].tokens.erase(reset[j].tokens.begin() + i);
					}
				}
			}
		}

		if (n.type == place::type) {
			places.erase(n.index);
		} else if (n.type == transition::type) {
			transitions.erase(n.index);
		}

		return result;
	}

	static void erase(petri::iterator n, vector<petri::iterator> &iter_list) {
		for (int i = (int)iter_list.size()-1; i >= 0; i--) {
			if (iter_list[i] == n) {
				iter_list.erase(iter_list.begin() + i);
			}
		}
	}

	static void erase(vector<petri::iterator> n, vector<petri::iterator> &iter_list) {
		sort(n.rbegin(), n.rend());
		for (int i = 0; i < (int)n.size(); i++) {
			erase(n[i], iter_list);
		}
	}

	static void erase(petri::iterator n, int type, vector<int> &iter_list) {
		if (n.type != type) {
			return;
		}

		for (int i = (int)iter_list.size()-1; i >= 0; i--) {
			if (iter_list[i] == n.index) {
				iter_list.erase(iter_list.begin() + i);
			}
		}
	}

	static void erase(petri::iterator n, state &s) {
		if (n.type != place::type) {
			return;
		}

		for (int i = (int)s.tokens.size()-1; i >= 0; i--) {
			if (s.tokens[i].index == n.index) {
				s.tokens.erase(s.tokens.begin() + i);
			}
		}
	}

	static void erase(petri::iterator n, vector<state> &s) {
		if (n.type != place::type) {
			return;
		}

		for (int i = 0; i < (int)s.size(); i++) {
			for (int j = (int)s[i].tokens.size()-1; j >= 0; j--) {
				if (s[i].tokens[j].index == n.index) {
					s[i].tokens.erase(s[i].tokens.begin() + j);
				}
			}
		}
	}

	// TODO(edward.bingham) this no longer needs to be reverse sorted
	virtual void erase(vector<petri::iterator> n, bool rsorted = false) {
		if (!rsorted) {
			sort(n.rbegin(), n.rend());
		}

		for (int i = 0; i < (int)n.size(); i++) {
			erase(n[i]);
		}
	}

	virtual petri::iterator nest_in(region to) {
		if (to.size() == 1u) {
			return to[0];
		}

		petri::iterator link = create(transition::type);
		for (auto i = to.begin(); i != to.end(); i++) {
			connect(link, *i);
		}
		return link;
	}

	virtual petri::iterator nest_out(region from) {
		if (from.size() == 1u) {
			return from[0];
		}

		petri::iterator link = create(transition::type);
		for (auto i = from.begin(); i != from.end(); i++) {
			connect(*i, link);
		}
		return link;
	}

	virtual petri::iterator nest_in(bound to) {
		if (to.size() == 1u) {
			return nest_in(to[0]);
		}

		petri::iterator link = create(place::type);
		for (auto i = to.begin(); i != to.end(); i++) {
			connect(link, nest_in(*i));
		}
		return link;
	}

	virtual petri::iterator nest_out(bound from) {
		if (from.size() == 1u) {
			return nest_out(from[0]);
		}

		petri::iterator link = create(place::type);
		for (auto i = from.begin(); i != from.end(); i++) {
			connect(nest_out(*i), link);
		}
		return link;
	}

	virtual petri::iterator connect(petri::iterator from, petri::iterator to) {
		if (from.type == place::type && to.type == place::type) {
			petri::iterator mid = create(transition());
			arcs[from.type].push_back(arc(from, mid));
			arcs[mid.type].push_back(arc(mid, to));
		} else if (from.type == transition::type && to.type == transition::type) {
			petri::iterator mid = create(place());
			arcs[from.type].push_back(arc(from, mid));
			arcs[mid.type].push_back(arc(mid, to));
		} else {
			arcs[from.type].push_back(arc(from, to));
		}
		return to;
	}

	virtual petri::iterator connect(vector<petri::iterator> from, petri::iterator to) {
		for (auto i = from.begin(); i != from.end(); i++) {
			connect(*i, to);
		}
		return to;
	}

	virtual vector<petri::iterator> connect(petri::iterator from, vector<petri::iterator> to) {
		for (auto i = to.begin(); i != to.end(); i++) {
			connect(from, *i);
		}
		return to;
	}

	virtual vector<petri::iterator> connect(vector<petri::iterator> from, vector<petri::iterator> to) {
		for (auto i = from.begin(); i != from.end(); i++) {
			for (auto j = to.begin(); j != to.end(); j++) {
				connect(*i, *j);
			}
		}
		return to;
	}

	virtual petri::region connect(petri::iterator from, petri::region to) {
		if (to.size() == 1u) {
			if (from.valid()) {
				connect(from, to[0]);
			} else {
				from = to[0];
			}
			return to;
		}

		petri::iterator link = from;
		if (link.type != transition::type) {
			link = create(transition::type);
			if (from.valid()) {
				connect(from, link);
			}
		}
		if (not from.valid()) {
			from = link;
		}

		for (auto i = to.begin(); i != to.end(); i++) {
			connect(link, *i);
		}
		return to;
	}

	virtual petri::iterator connect(petri::region from, petri::iterator to) {
		if (from.size() == 1u) {
			if (to.valid()) {
				connect(from[0], to);
			} else {
				to = from[0];
			}
			return to;
		}

		petri::iterator link = to;
		if (link.type != transition::type) {
			link = create(transition::type);
			if (to.valid()) {
				connect(link, to);
			}
		}
		if (not to.valid()) {
			to = link;
		}

		for (auto i = from.begin(); i != from.end(); i++) {
			connect(*i, link);
		}
		return to;
	}

	virtual petri::region connect(petri::region from, petri::region to) {
		if (from.size() == 1u and to.size() == 1u) {
			return {connect(from[0], to[0])};
		} else if (from.size() == 1u) {
			return connect(from[0], to);
		} else if (to.size() == 1u) {
			return {connect(from, to[0])};
		} else {
			petri::iterator link = create(transition::type);
			connect(from, link);
			connect(link, to);
		}
		return to;
	}

	virtual petri::bound connect(petri::bound from, petri::bound to, bool proper=false) {
		petri::bound link;
		if (not proper or (from.size() == 1u and from[0].size() == 1u) or (to.size() == 1u and to[0].size() == 1u)) {
			if (to.size() > 1u) {
				for (auto i = from.begin(); i != from.end(); i++) {
					for (auto j = i->begin(); j != i->end(); j++) {
						if (j->type != place::type) {
							*j = connect(*j, create(place::type));
						}
					}
				}
			}

			if (from.size() > 1u) {
				for (auto i = to.begin(); i != to.end(); i++) {
					link.push_back(region());
					for (auto j = i->begin(); j != i->end(); j++) {
						if (j->type != place::type) {
							petri::iterator n = create(place::type);
							connect(n, *j);
							link.back().push_back(n);
						} else {
							link.back().push_back(*j);
						}
					}
				}
			} else {
				link = to;
			}
		} else {
			petri::bound sub;
			for (auto i = to.begin(); i != to.end(); i++) {
				sub.push_back(region());
				if (i->size() == 1u) {
					sub.regions.back().push_back((*i)[0]);
				} else {
					petri::iterator j = create(transition::type);
					connect(j, *i);
					sub.regions.back().push_back(j);
				}
			}
			if (sub.size() == 1u) {
				link = sub;
			} else {
				petri::iterator n = create(place::type);
				for (auto i = sub.begin(); i != sub.end(); i++) {
					connect(n, *i);
				}
				link.push_back({n});
			}
		}

		for (auto i = from.begin(); i != from.end(); i++) {
			for (auto j = link.begin(); j != link.end(); j++) {
				connect(*i, *j);
			}
		}
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

	virtual petri::region connect(vector<petri::region> a) {
		for (auto i0 = a.begin(); ::next(i0) != a.end(); i0++) {
			auto i1 = ::next(i0);
			connect(*i0, *i1);
		}
		return a.back();
	}

	virtual petri::bound connect(vector<petri::bound> a, bool proper=false) {
		for (auto i0 = a.begin(); ::next(i0) != a.end(); i0++) {
			auto i1 = ::next(i0);
			connect(*i0, *i1, proper);
		}
		return a.back();
	}

	virtual void erase_arc(petri::iterator a) {
		arcs[a.type].erase(arcs[a.type].begin() + a.index);
	}

	virtual petri::iterator copy(petri::iterator i) {
		if (i.type == place::type && i.index < (int)places.size()) {
			auto result = create(places[i.index]);
			for (int j = 0; j < (int)reset.size(); j++) {
				for (int k = 0; k < (int)reset[j].tokens.size(); k++) {
					if (reset[j].tokens[k].index == i.index) {
						reset[j].tokens.push_back(reset[j].tokens[k]);
						reset[j].tokens.back().index = result.index;
					}
				}
			}

			return result;
		} else if (i.type == transition::type && i.index < (int)transitions.size()) {
			return create(transitions[i.index]);
		} else {
			internal("petri::copy", "iterator out of bounds", __FILE__, __LINE__);
			return petri::iterator();
		}
	}

	virtual vector<petri::iterator> copy(petri::iterator i, int num) {
		vector<petri::iterator> result;
		if (i.type == place::type && i.index < (int)places.size()) {
			for (int j = 0; j < num; j++) {
				result.push_back(create(places[i.index]));
			}

			for (int j = 0; j < (int)reset.size(); j++) {
				for (int k = 0; k < (int)reset[j].tokens.size(); k++) {
					if (reset[j].tokens[k].index == i.index) {
						for (int l = 0; l < (int)result.size(); l++) {
							reset[j].tokens.push_back(reset[j].tokens[k]);
							reset[j].tokens.back().index = result[l].index;
						}
					}
				}
			}
		} else if (i.type == transition::type && i.index < (int)transitions.size()) {
			for (int j = 0; j < num; j++) {
				result.push_back(create(transitions[i.index]));
			}
		} else {
			internal("petri::copy", "iterator out of bounds", __FILE__, __LINE__);
			return vector<petri::iterator>();
		}
		return result;
	}

	virtual vector<petri::iterator> copy(vector<petri::iterator> i, int num = 1) {
		vector<petri::iterator> result;
		for (int j = 0; j < (int)i.size(); j++) {
			vector<petri::iterator> temp = copy(i[j], num);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual petri::iterator copy_combine(Composition::Type composition, petri::iterator i0, petri::iterator i1) {
		if (i0.type == place::type and i1.type == place::type) {
			return create(place::merge(composition, places[i0.index], places[i1.index]));
		} else if (i0.type == transition::type and i1.type == transition::type) {
			if (transition::mergeable(composition, transitions[i0.index], transitions[i1.index])) {
				return create(transition::merge(composition, transitions[i0.index], transitions[i1.index]));
			} else {
				internal("petri::copy_combine", "transitions are not mergeable", __FILE__, __LINE__);
			}
		} else {
			internal("petri::copy_combine", "iterator types do not match", __FILE__, __LINE__);
		}
		return petri::iterator();
	}

	virtual petri::iterator combine(Composition::Type composition, petri::iterator i0, petri::iterator i1) {
		if (i0.type == place::type and i1.type == place::type) {
			places[i0.index] = place::merge(composition, places[i0.index], places[i1.index]);
			return i0;
		} else if (i0.type == transition::type and i1.type == transition::type) {
			if (transition::mergeable(composition, transitions[i0.index], transitions[i1.index])) {
				transitions[i0.index] = transition::merge(composition, transitions[i0.index], transitions[i1.index]);
				return i0;
			} else {
				internal("petri::combine", "transitions are not mergeable", __FILE__, __LINE__);
			}
		} else {
			internal("petri::combine", "iterator types do not match", __FILE__, __LINE__);
		}
		return petri::iterator();
	}

	template <class node>
	petri::iterator push_back(petri::iterator from, node n) {
		return connect(from, create(n));
	}

	template <class node>
	petri::iterator push_back(petri::region from, node n) {
		return connect(from, create(n));
	}

	template <class node>
	petri::region push_back(petri::iterator from, node n, int num) {
		return connect(from, create(n, num));
	}

	template <class node>
	petri::region push_back(petri::region from, node n, int num) {
		return connect(from, create(n, num));
	}

	template <class node>
	petri::iterator push_front(petri::iterator to, node n) {
		return connect(create(n), to);
	}

	template <class node>
	petri::iterator push_front(petri::region to, node n) {
		return connect(create(n), to);
	}

	template <class node>
	petri::region push_front(petri::iterator to, node n, int num) {
		return connect(create(n, num), to);
	}

	template <class node>
	petri::region push_front(petri::region to, node n, int num) {
		return connect(create(n, num), to);
	}

	virtual petri::iterator insert(petri::iterator a, place n) {
		petri::iterator i[2];
		i[place::type] = create(n);
		i[transition::type] = create(transition());
		arcs[a.type].push_back(arc(i[a.type], arcs[a.type][a.index].to));
		arcs[1-a.type].push_back(arc(i[1-a.type], i[a.type]));
		arcs[a.type][a.index].to = i[1-a.type];
		return i[place::type];
	}

	virtual petri::iterator insert(petri::iterator a, transition n) {
		petri::iterator i[2];
		i[place::type] = create(place());
		i[transition::type] = create(n);
		arcs[a.type].push_back(arc(i[a.type], arcs[a.type][a.index].to));
		arcs[1-a.type].push_back(arc(i[1-a.type], i[a.type]));
		arcs[a.type][a.index].to = i[1-a.type];
		return i[transition::type];
	}

	virtual petri::iterator insert(petri::iterator a, int n) {
		if (n == place::type) {
			return insert(a, place());
		} else if (n == transition::type) {
			return insert(a, transition());
		}
		return petri::iterator();
	}

	virtual petri::iterator insert_alongside(petri::iterator from, petri::iterator to, place n) {
		petri::iterator i = create(n);
		if (from.type == i.type) {
			petri::iterator j = create(transition());
			connect(from, j);
			connect(j, i);
		} else {
			connect(from, i);
		}

		if (to.type == i.type) {
			petri::iterator j = create(transition());
			connect(i, j);
			connect(j, to);
		} else {
			connect(i, to);
		}

		return i;
	}

	virtual petri::iterator insert_alongside(petri::iterator from, petri::iterator to, transition n) {
		petri::iterator i = create(n);
		if (from.type == i.type) {
			petri::iterator j = create(place());
			connect(from, j);
			connect(j, i);
		} else {
			connect(from, i);
		}

		if (to.type == i.type) {
			petri::iterator j = create(place());
			connect(i, j);
			connect(j, to);
		} else {
			connect(i, to);
		}

		return i;
	}

	virtual petri::iterator insert_alongside(petri::iterator from, petri::iterator to, int n) {
		if (n == place::type) {
			return insert_alongside(from, to, place());
		} else if (n == transition::type) {
			return insert_alongside(from, to, transition());
		}
		return petri::iterator();
	}

	virtual petri::iterator insert_before(petri::iterator to, place n) {
		petri::iterator i[2];
		i[transition::type] = create(transition());
		i[place::type] = create(n);
		for (int j = 0; j < (int)arcs[1-to.type].size(); j++) {
			if (arcs[1-to.type][j].to.index == to.index) {
				arcs[1-to.type][j].to.index = i[to.type].index;
			}
		}
		connect(i[1-to.type], to);
		connect(i[to.type], i[1-to.type]);
		return i[place::type];
	}

	virtual petri::iterator insert_before(petri::iterator to, transition n) {
		petri::iterator i[2];
		i[transition::type] = create(n);
		i[place::type] = create(place());
		for (int j = 0; j < (int)arcs[1-to.type].size(); j++) {
			if (arcs[1-to.type][j].to.index == to.index) {
				arcs[1-to.type][j].to.index = i[to.type].index;
			}
		}
		connect(i[1-to.type], to);
		connect(i[to.type], i[1-to.type]);
		return i[transition::type];
	}

	virtual petri::iterator insert_before(petri::iterator to, int n) {
		if (n == place::type) {
			return insert_before(to, place());
		} else if (n == transition::type) {
			return insert_before(to, transition());
		}
		return petri::iterator();
	}

	virtual petri::iterator insert_after(petri::iterator from, place n) {
		petri::iterator i[2];
		i[transition::type] = create(transition());
		i[place::type] = create(n);
		for (int j = 0; j < (int)arcs[from.type].size(); j++) {
			if (arcs[from.type][j].from.index == from.index) {
				arcs[from.type][j].from.index = i[from.type].index;
			}
		}
		connect(from, i[1-from.type]);
		connect(i[1-from.type], i[from.type]);
		return i[place::type];
	}

	virtual petri::iterator insert_after(petri::iterator from, transition n) {
		petri::iterator i[2];
		i[transition::type] = create(n);
		i[place::type] = create(place());
		for (int j = 0; j < (int)arcs[from.type].size(); j++) {
			if (arcs[from.type][j].from.index == from.index) {
				arcs[from.type][j].from.index = i[from.type].index;
			}
		}
		connect(from, i[1-from.type]);
		connect(i[1-from.type], i[from.type]);
		return i[transition::type];
	}

	virtual petri::iterator insert_after(petri::iterator from, int n) {
		if (n == place::type) {
			return insert_after(from, place());
		} else if (n == transition::type) {
			return insert_after(from, transition());
		}
		return petri::iterator();
	}

	virtual petri::iterator insert_at(petri::region to, transition n) {
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

	virtual petri::iterator duplicate(Composition::Type composition, petri::iterator i, bool add = true) {
		petri::iterator d = copy(i);
		if ((i.type == transition::type and composition == Composition::CHOICE) or (i.type == place::type and composition == Composition::PARALLEL)) {
			for (int j = (int)arcs[i.type].size()-1; j >= 0; j--) {
				if (arcs[i.type][j].from == i) {
					connect(d, arcs[i.type][j].to);
				}
			}
			for (int j = (int)arcs[1-i.type].size()-1; j >= 0; j--) {
				if (arcs[1-i.type][j].to == i) {
					connect(arcs[1-i.type][j].from, d);
				}
			}
		} else if (add) {
			vector<petri::iterator> x = create(1-i.type, 4);
			vector<petri::iterator> y = create(i.type, 2);

			for (int j = (int)arcs[i.type].size()-1; j >= 0; j--) {
				if (arcs[i.type][j].from == i) {
					arcs[i.type][j].from = y[1];
				}
			}
			for (int j = (int)arcs[1-i.type].size()-1; j >= 0; j--) {
				if (arcs[1-i.type][j].to == i) {
					arcs[1-i.type][j].to = y[0];
				}
			}

			connect(y[0], x[0]);
			connect(y[0], x[1]);
			connect(x[0], i);
			connect(x[1], d);
			connect(i, x[2]);
			connect(d, x[3]);
			connect(x[2], y[1]);
			connect(x[3], y[1]);
		} else {
			vector<petri::iterator> n = next(i);
			vector<petri::iterator> p = prev(i);

			for (int j = 0; j < 2; j++) {
				for (int k = (int)arcs[j].size()-1; k >= 0; k--) {
					if (arcs[j][k].from == i or arcs[j][k].to == i) {
						arcs[j].erase(arcs[j].begin() + k);
					}
				}
			}

			vector<petri::iterator> n1, p1;
			for (int l = 0; l < (int)n.size(); l++) {
				n1.push_back(duplicate(composition, n[l]));
			}
			for (int l = 0; l < (int)p.size(); l++) {
				p1.push_back(duplicate(composition, p[l]));
			}

			connect(p1, d);
			connect(d, n1);
			connect(p, i);
			connect(i, n);
		}

		return d;
	}

	virtual vector<petri::iterator> duplicate(Composition::Type composition, petri::iterator i, int num, bool add = true) {
		if (num == 0) {
			return vector<petri::iterator>();
		}

		vector<petri::iterator> d = copy(i, num-1);
		if ((i.type == transition::type and composition == Composition::CHOICE) or (i.type == place::type and composition == Composition::PARALLEL)) {
			for (int j = (int)arcs[i.type].size()-1; j >= 0; j--) {
				if (arcs[i.type][j].from == i) {
					for (int k = 0; k < (int)d.size(); k++) {
						connect(d[k], arcs[i.type][j].to);
					}
				}
			}
			for (int j = (int)arcs[1-i.type].size()-1; j >= 0; j--) {
				if (arcs[1-i.type][j].to == i) {
					for (int k = 0; k < (int)d.size(); k++) {
						connect(arcs[1-i.type][j].from, d[k]);
					}
				}
			}
		} else if (add) {
			vector<petri::iterator> x = create(1-i.type, 2*(num-1));
			vector<petri::iterator> y = create(i.type, 2);
			vector<petri::iterator> z = create(1-i.type, 2);

			for (int j = (int)arcs[i.type].size()-1; j >= 0; j--) {
				if (arcs[i.type][j].from == i) {
					arcs[i.type][j].from = y[1];
				}
			}
			for (int j = (int)arcs[1-i.type].size()-1; j >= 0; j--) {
				if (arcs[1-i.type][j].to == i) {
					arcs[1-i.type][j].to = y[0];
				}
			}

			connect(y[0], z[0]);
			connect(z[0], i);
			connect(i, z[1]);
			connect(z[1], y[1]);

			for (int k = 0; k < (int)d.size(); k++) {
				connect(y[0], x[k*2 + 0]);
				connect(x[k*2 + 0], d[k]);
				connect(d[k], x[k*2 + 1]);
				connect(x[k*2 + 1], y[1]);
			}
		} else {
			vector<petri::iterator> n = next(i);
			vector<petri::iterator> p = prev(i);

			for (int j = 0; j < 2; j++) {
				for (int k = (int)arcs[j].size()-1; k >= 0; k--) {
					if (arcs[j][k].from == i or arcs[j][k].to == i) {
						arcs[j].erase(arcs[j].begin() + k);
					}
				}
			}

			for (int k = 0; k < num-1; k++) {
				vector<petri::iterator> n1, p1;
				for (int l = 0; l < (int)n.size(); l++) {
					n1.push_back(duplicate(composition, n[l]));
				}
				for (int l = 0; l < (int)p.size(); l++) {
					p1.push_back(duplicate(composition, p[l]));
				}

				connect(p1, d[k]);
				connect(d[k], n1);
			}
			connect(p, i);
			connect(i, n);
		}

		d.push_back(i);

		return d;
	}

	virtual vector<petri::iterator> duplicate(Composition::Type composition, vector<petri::iterator> n, int num = 1, bool interleaved = false, bool add = true) {
		vector<petri::iterator> result;
		result.reserve(n.size()*num);
		for (int i = 0; i < (int)n.size(); i++) {
			vector<petri::iterator> temp = duplicate(composition, n[i], num, add);
			if (interleaved and i > 0) {
				for (int j = 0; j < (int)temp.size(); j++) {
					result.insert(result.begin() + j*(i+1) + 1, temp[j]);
				}
			} else {
				result.insert(result.end(), temp.begin(), temp.end());
			}
		}
		return result;
	}

	// Pinch removes a node without affecting the connectivity, node dominance,
	// or token flow of the graph by creating direct connections between
	// predecessor and successor nodes
	//
	// 1. Analyze incoming and outgoing connections of the target node
	// 2. Create direct connections that bypass the node
	// 3. Duplicate nodes as needed to maintain proper connection patterns
	// 4. Handle state updates for source, sink, and reset tokens
	//
	// Return mapping between original and modified nodes
	virtual void pinch(petri::iterator n) {
		pair<vector<petri::iterator>, vector<petri::iterator> > neighbors = erase(n);

		vector<petri::iterator> left = duplicate((Composition::Type)n.type, neighbors.first, neighbors.second.size(), false);
		vector<petri::iterator> right = duplicate((Composition::Type)n.type, neighbors.second, neighbors.first.size(), true);

		for (int i = 0; i < (int)right.size(); i++) {
			combine(Composition::SEQUENCE, left[i], right[i]);

			for (int j = 0; j < (int)arcs[right[i].type].size(); j++) {
				if (arcs[right[i].type][j].from == right[i]) {
					arcs[right[i].type][j].from = left[i];
				}
			}

			for (int j = 0; j < (int)arcs[1-right[i].type].size(); j++) {
				if (arcs[1-right[i].type][j].to == right[i]) {
					arcs[1-right[i].type][j].to = left[i];
				}
			}

			if (right[i].type == place::type) {
				for (int j = 0; j < (int)reset.size(); j++) {
					for (int k = (int)reset[j].tokens.size()-1; k >= 0; k--) {
						if (reset[j].tokens[k].index == right[i].index) {
							//reset[j].tokens.push_back(reset[j].tokens[k]);
							reset[j].tokens[k].index = left[i].index;
						}
					}
				}
			}
		}

		erase(right);
		erase(right, left);
	}

	virtual vector<petri::iterator> next(petri::iterator n, bool sorted=false) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[n.type].size(); i++) {
			if (arcs[n.type][i].from.index == n.index) {
				result.push_back(arcs[n.type][i].to);
			}
		}
		if (sorted) {
			sort(result.begin(), result.end());
			result.erase(unique(result.begin(), result.end()), result.end());
		}
		return result;
	}

	virtual vector<petri::iterator> next(vector<petri::iterator> n, bool sorted=false) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)n.size(); i++) {
			vector<petri::iterator> temp = next(n[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		if (sorted) {
			sort(result.begin(), result.end());
			result.erase(unique(result.begin(), result.end()), result.end());
		}
		return result;
	}

	virtual vector<petri::iterator> prev(petri::iterator n, bool sorted=false) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-n.type].size(); i++) {
			if (arcs[1-n.type][i].to.index == n.index) {
				result.push_back(arcs[1-n.type][i].from);
			}
		}
		if (sorted) {
			sort(result.begin(), result.end());
			result.erase(unique(result.begin(), result.end()), result.end());
		}
		return result;
	}

	virtual vector<petri::iterator> prev(vector<petri::iterator> n, bool sorted=false) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)n.size(); i++) {
			vector<petri::iterator> temp = prev(n[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		if (sorted) {
			sort(result.begin(), result.end());
			result.erase(unique(result.begin(), result.end()), result.end());
		}
		return result;
	}

	virtual vector<petri::iterator> neighbors(petri::iterator n, bool sorted = false) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-n.type].size(); i++) {
			if (arcs[1-n.type][i].to.index == n.index) {
				result.push_back(arcs[1-n.type][i].from);
			}
		}

		for (int i = 0; i < (int)arcs[n.type].size(); i++) {
			if (arcs[n.type][i].from.index == n.index) {
				result.push_back(arcs[n.type][i].to);
			}
		}

		if (sorted) {
			sort(result.begin(), result.end());
		}
		return result;
	}

	virtual vector<int> next(int type, int n) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[type].size(); i++) {
			if (arcs[type][i].from.index == n) {
				result.push_back(arcs[type][i].to.index);
			}
		}
		return result;
	}

	virtual vector<int> next(int type, vector<int> n) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[type].size(); i++) {
			if (find(n.begin(), n.end(), arcs[type][i].from.index) != n.end()) {
				result.push_back(arcs[type][i].to.index);
			}
		}
		return result;
	}

	virtual vector<int> prev(int type, int n) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++) {
			if (arcs[1-type][i].to.index == n) {
				result.push_back(arcs[1-type][i].from.index);
			}
		}
		return result;
	}

	virtual vector<int> prev(int type, vector<int> n) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++) {
			if (find(n.begin(), n.end(), arcs[1-type][i].to.index) != n.end()) {
				result.push_back(arcs[1-type][i].from.index);
			}
		}
		return result;
	}

	virtual vector<int> neighbors(int type, int n, bool sorted = false) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++) {
			if (arcs[1-type][i].to.index == n) {
				result.push_back(arcs[1-type][i].from.index);
			}
		}
		for (int i = 0; i < (int)arcs[type].size(); i++) {
			if (arcs[type][i].from.index == n) {
				result.push_back(arcs[type][i].to.index);
			}
		}
		if (sorted) {
			sort(result.begin(), result.end());
		}
		return result;
	}

	virtual vector<int> neighbors(int type, vector<int> n, bool sorted = false) const {
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

	virtual vector<petri::iterator> out(petri::iterator n) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[n.type].size(); i++)
			if (arcs[n.type][i].from.index == n.index)
				result.push_back(petri::iterator(n.type, i));
		return result;
	}

	virtual vector<petri::iterator> out(vector<petri::iterator> n) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)n.size(); i++) {
			vector<petri::iterator> temp = out(n[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<petri::iterator> in(petri::iterator n) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-n.type].size(); i++)
			if (arcs[1-n.type][i].to.index == n.index)
				result.push_back(petri::iterator(1-n.type, i));
		return result;
	}

	virtual vector<petri::iterator> in(vector<petri::iterator> n) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)n.size(); i++) {
			vector<petri::iterator> temp = in(n[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<int> out(int type, int n) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[type].size(); i++)
			if (arcs[type][i].from.index == n)
				result.push_back(i);
		return result;
	}

	virtual vector<int> out(int type, vector<int> n) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[type].size(); i++)
			if (find(n.begin(), n.end(), arcs[type][i].from.index) != n.end())
				result.push_back(i);
		return result;
	}

	virtual vector<int> in(int type, int n) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (arcs[1-type][i].to.index == n)
				result.push_back(i);
		return result;
	}

	virtual vector<int> in(int type, vector<int> n) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (find(n.begin(), n.end(), arcs[1-type][i].to.index) != n.end())
				result.push_back(i);
		return result;
	}

	virtual vector<petri::iterator> next_arcs(petri::iterator a) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-a.type].size(); i++)
			if (arcs[1-a.type][i].from == arcs[a.type][a.index].to)
				result.push_back(petri::iterator(1-a.type, i));
		return result;
	}

	virtual vector<petri::iterator> next_arcs(vector<petri::iterator> a) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)a.size(); i++) {
			vector<petri::iterator> temp = next_arcs(a[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<petri::iterator> prev_arcs(petri::iterator a) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)arcs[1-a.type].size(); i++)
			if (arcs[1-a.type][i].to == arcs[a.type][a.index].from)
				result.push_back(petri::iterator(1-a.type, i));
		return result;
	}

	virtual vector<petri::iterator> prev_arcs(vector<petri::iterator> a) const {
		vector<petri::iterator> result;
		for (int i = 0; i < (int)a.size(); i++) {
			vector<petri::iterator> temp = prev_arcs(a[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<int> next_arcs(int type, int a) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (arcs[1-type][i].from == arcs[type][a].to)
				result.push_back(i);
		return result;
	}

	virtual vector<int> next_arcs(int type, vector<int> a) const {
		vector<int> result;
		for (int i = 0; i < (int)a.size(); i++) {
			vector<int> temp = next_arcs(type, a[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual vector<int> prev_arcs(int type, int a) const {
		vector<int> result;
		for (int i = 0; i < (int)arcs[1-type].size(); i++)
			if (arcs[1-type][i].to == arcs[type][a].from)
				result.push_back(i);
		return result;
	}

	virtual vector<int> prev_arcs(int type, vector<int> a) const {
		vector<int> result;
		for (int i = 0; i < (int)a.size(); i++) {
			vector<int> temp = prev_arcs(type, a[i]);
			result.insert(result.end(), temp.begin(), temp.end());
		}
		return result;
	}

	virtual segment loop(segment s0, bool proper=false) {
		if (s0.source.empty() or s0.sink.empty()) {
			return s0;
		}
		connect(s0.sink, s0.source, proper);

		petri::iterator link = create(place::type);
		connect({{link}}, s0.source);
		connect(s0.sink, {{link}});
		s0.sink = petri::bound({{link}});
		if (not s0.reset.empty()) {
			s0.source = s0.reset;
			s0.reset.clear();
		} else {
			s0.source = petri::bound({{link}});
		}
		return s0;
	}

	// This function implements graph composition operations that merge
	// the current Petri net with another one according to one of three fundamental
	// composition patterns: sequence, choice, or parallel. Each composition type creates
	// a different relationship between the two nets:
	//
	// - Sequence: Connect the sink nodes of the current net to the source nodes of the
	//   provided net, creating a sequential flow.
	// - Choice: Create a structure where either the current net or the provided net
	//   will execute, but not both.
	// - Parallel: Create a structure where both nets execute concurrently.
	//
	// @param composition The composition type to use (Composition::SEQUENCE, choice, or parallel)
	// @param g The Petri net to merge with the current one
	// @return A mapping from original nodes to corresponding nodes in the merged net
	virtual segment compose(Composition::Type composition, segment s0, segment s1, bool proper=false) {
		if (s0.source.empty()) {
			s0 = s1;
		} else if (s1.source.empty()) {
			// skip
		} else if (composition == Composition::SEQUENCE) {
			if (s0.sink.empty()) {
				printf("warning: sequencing creates dead code.\n");
			} else {
				connect(s0.sink, s1.source, proper);
			}
			s0.sink = s1.sink;
			if (s0.reset.empty()) {
				s0.reset = s1.reset;
			}
		} else {
			if (proper) {
				s0.source = bound({{nest_in(s0.source)}});
				s0.sink = bound({{nest_out(s0.sink)}});
				s1.source = bound({{nest_in(s1.source)}});
				s1.sink = bound({{nest_out(s1.sink)}});
			}
			s0.compose(composition, s1);
		}

		return s0;
	}

	virtual Mapping<petri::iterator> merge(const graph<place, transition, state> &g) {
		Mapping<petri::iterator> result(petri::iterator(), false);
		for (int i = 0; i < (int)g.places.size(); i++) {
			if (g.places.is_valid(i)) {
				result.set(petri::iterator(place::type, i), create(g.places[i]));
			}
		}
		for (int i = 0; i < (int)g.transitions.size(); i++) {
			if (g.transitions.is_valid(i)) {
				result.set(petri::iterator(transition::type, i), create(g.transitions[i]));
			}
		}
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < (int)g.arcs[i].size(); j++) {
				arcs[i].push_back(arc(result.map(g.arcs[i][j].from), result.map(g.arcs[i][j].to)));
			}
		}
		return result;
	}

	// Do a depth first search to find all cycles in the graph from the starting node
	virtual vector<strand> cycles(vector<petri::iterator> from, bool sorted=false) const {
		vector<strand> curr;
		vector<strand> result;
		for (auto i = from.begin(); i != from.end(); i++) {
			curr.push_back({*i});
		}

		sort(curr.begin(), curr.end());
		curr.erase(unique(curr.begin(), curr.end()), curr.end());

		while (curr.size() > 0) {
			strand x = curr.back();
			curr.pop_back();

			vector<petri::iterator> n = next(x.back());
			for (int j = 0; j < (int)n.size(); j++) {
				vector<petri::iterator>::iterator loopback = find(x.begin(), x.end(), n[j]);
				if (loopback != x.end()) {
					result.push_back(x);
					result.back().nodes.erase(result.back().begin(), result.back().begin() + (loopback - x.begin()));
				} else {
					curr.push_back(x);
					curr.back().push_back(n[j]);
				}
			}
		}

		if (sorted) {
			for (auto i = result.begin(); i != result.end(); i++) {
				i->sort();
			}
			sort(result.begin(), result.end());
		}

		return result;
	}

	petri::region regionFromState(const state &s) const {
		petri::region result;
		for (auto t = s.tokens.begin(); t != s.tokens.end(); t++) {
			result.push_back(petri::iterator(place::type, t->index));
		}
		return result;
	}

	petri::bound boundFromStates(const vector<state> &s) const {
		petri::bound result;
		for (auto t = s.begin(); t != s.end(); t++) {
			result.push_back(regionFromState(*t));
		}
		return result;
	}

	// TODO(edward.bingham) I need to find vacuous loops and simplify them
	virtual bool remove_vacuous_loops() {
		if (reset.empty()) {
			return false;
		}

		vector<strand> complete = cycles(regionFromState(reset[0]).nodes, true);
		vector<strand> partial;

		if (complete.empty()) {
			return false;
		}

		// a loop is only vacuous if it exists in all reset states.
		for (size_t i = 1; i < reset.size(); i++) {
			vector<strand> newLoops = cycles(regionFromState(reset[i]).nodes, true);

			size_t j, k;
			for (j = 0, k = 0; j < complete.size() and k < newLoops.size();) {
				if (complete[j] < newLoops[k]) {
					partial.push_back(complete[j]);
					complete.erase(complete.begin()+j);
				} else if (newLoops[k] < complete[j]) {
					partial.push_back(newLoops[k]);
					k++;
				} else {
					j++;
					k++;
				}
			}
			partial.insert(partial.end(), complete.begin()+j, complete.end());
			complete.erase(complete.begin()+j, complete.end());
			partial.insert(partial.end(), newLoops.begin()+k, newLoops.end());

			sort(partial.begin(), partial.end());
			partial.erase(unique(partial.begin(), partial.end()), partial.end());

			if (complete.empty()) {
				return false;
			}
		}

		// look at all complete that are shared across all reset states for which
		// all transitions are vacuous.
		std::set<petri::iterator> vacuousNodes;
		std::set<petri::iterator> activeNodes;
		for (auto i = partial.begin(); i != partial.end(); i++) {
			activeNodes.insert(i->begin(), i->end());
		}
		for (auto i = complete.begin(); i != complete.end(); i++) {
			bool isVacuous = true;
			for (auto j = i->begin(); j != i->end() and isVacuous; j++) {
				isVacuous = isVacuous and (j->type == place::type or transitions[j->index].is_vacuous());
			}

			if (isVacuous) {
				vacuousNodes.insert(i->begin(), i->end());
			} else {
				activeNodes.insert(i->begin(), i->end());
			}
		}

		// Remove any nodes that don't exist in complete that are not vacuous.
		std::set<petri::iterator> result;
		std::set_difference(vacuousNodes.begin(), vacuousNodes.end(), activeNodes.begin(), activeNodes.end(), std::inserter(result, result.end()));
		for (auto i = result.begin(); i != result.end(); i++) {
			erase(*i);
		}
		return not result.empty();
	}

	virtual bool remove_infeasible(bool debug=false) {
		bool affect = false;

		// A transition will never be enabled if it is infeasible.
		// These transitions may be removed while preserving proper nesting, token flow
		// stability, non interference, and deadlock freedom. At this point, it is not
		// possible for this transition to be in the source list.
		for (petri::iterator i(transition::type, 0); i < (int)transitions.size(); i++) {
			if (not is_valid(i)) continue;

			if (transitions[i.index].is_infeasible()) {
				if (debug) cout << "\terasing infeasible transition " << i << endl;
				erase(i);
				affect = true;
				continue;
			}

			vector<petri::iterator> n = next(i, true);
			vector<petri::iterator> p = prev(i, true);

			// If it doesn't have any input places, then we need to add one.
			if (p.empty()) {
				if (debug) cout << "\tno input places for " << i << ", adding one" << endl;
				p.push_back(create(place::type));
				connect(p, i);
				affect = true;
			}

			// If it doesn't have any output places, then we need to add one.
			if (n.empty()) {
				if (debug) cout << "\tno output places for " << i << ", adding one" << endl;
				n.push_back(create(place::type));
				connect(i, n);
				affect = true;
			}
		}

		// We know a place will never be marked if it is not in the initial marking
		// and it has no input arcs. This means that its output transitions will
		// never fire.
		for (petri::iterator i(place::type, 0); i < (int)places.size(); i++) {
			if (not is_valid(i)) continue;

			vector<petri::iterator> n = next(i, true);
			vector<petri::iterator> p = prev(i, true);

			if (p.empty() and (n.empty() or not is_reset(i))) {
				if (debug) cout << "\terasing place with no input arcs " << i << " -> " << to_string(n) << endl;
				erase(n);
				erase(i);
				affect = true;
			}
		}

		return affect;
	}

	// Vacuous transitions may be pinched while preserving token flow, stability,
	// non interference, and deadlock freedom. However, proper nesting is not
	// necessarily preserved. We have to take special precautions if we want to
	// preserver proper nesting.
	virtual bool pinch_vacuous(bool proper_nesting=true, bool debug=false) {
		bool affect = false;
		for (petri::iterator i(transition::type, 0); i < (int)transitions.size(); i++) {
			if (not is_valid(i) or not transitions[i.index].is_vacuous()) {
				continue;
			}

			vector<petri::iterator> n = next(i, true);
			vector<petri::iterator> p = prev(i, true);
			vector<petri::iterator> np = next(p, true);
			vector<petri::iterator> pn = prev(n, true);
			vector<petri::iterator> nn = next(n, true);
			vector<petri::iterator> pp = prev(p, true);

			vector<petri::iterator> c0 = ::vector_intersection(np, pn);
			vector<petri::iterator> c1 = ::vector_intersection(nn, pp);
			c0.insert(c0.end(), c1.begin(), c1.end());
			bool loop = false;
			for (auto j = c0.begin(); j != c0.end() and not loop; j++) {
				loop = *j != i;
			}
			if (loop) {
				continue;
			}

			if (not proper_nesting
				or (p.size() == 1 and n.size() == 1 and (np.size() == 1 or pn.size() == 1))
				or (n.size() == 1 and nn.size() == 1 and next(np).size() == 1 and np.size() == 1)
				or (p.size() == 1 and pp.size() == 1 and prev(pn).size() == 1 and pn.size() == 1)) {
				if (debug) cout << "\tpinching vacuous transition " << i << endl;
				pinch(i);
				affect = true;
			}
		}
		return affect;
	}

	virtual bool compose_internal(bool aggressive=false, bool debug=false) {
		bool affect = false;
		// TODO Once internal parallelism stops assuming isochronic forks we can re-enable this for active transitions
		for (petri::iterator i(transition::type, 0); i < (int)transitions.size(); i++) {
			if (not is_valid(i)) continue;

			vector<petri::iterator> ni = next(i, true);
			vector<petri::iterator> pi = prev(i, true);

			vector<array<vector<petri::iterator>, 2> > nix, pix;
			for (auto k = ni.begin(); k != ni.end(); k++) {
				nix.push_back({prev(*k, true), next(*k, true)});
			}
			for (auto k = pi.begin(); k != pi.end(); k++) {
				pix.push_back({prev(*k, true), next(*k, true)});
			}

			for (petri::iterator j = i-1; j >= 0; j--) {
				if (not is_valid(j)) continue;

				vector<petri::iterator> nj = next(j, true);
				vector<petri::iterator> pj = prev(j, true);

				// Find internally conditioned transitions. Transitions are internally conditioned if they are the same type
				// share all of the same input and output places.
				if (nj == ni and pj == pi and (aggressive
						or (transitions[i.index].is_vacuous() and transitions[j.index].is_vacuous()))) {
					if (debug) cout << "\tmerging internally conditioned transitions " << i << " and " << j << endl;
					transitions[j.index] = transition::merge(Composition::CHOICE, transitions[i.index], transitions[j.index]);
					erase(i);
					affect = true;
					break;
				}

				vector<array<vector<petri::iterator>, 2> > njx, pjx;
				for (auto k = nj.begin(); k != nj.end(); k++) {
					njx.push_back({prev(*k, true), next(*k, true)});
				}
				for (auto k = pj.begin(); k != pj.end(); k++) {
					pjx.push_back({prev(*k, true), next(*k, true)});
				}

				// Find internally parallel transitions. A pair of transitions A and B are internally parallel if
				// they are the same type, have disjoint sets of input and output places that share a single input
				// or output transition and have no output or input transitions other than A or B.
				if (vector_intersection_size(ni, nj) == 0
					and vector_intersection_size(pi, pj) == 0
					and nix == njx and pix == pjx
					and (aggressive
						or (transitions[i.index].is_vacuous() or transitions[j.index].is_vacuous()))) {
					if (debug) cout << "\tmerging internally parallel transitions " << i << " and " << j << endl;
					transitions[j.index] = transition::merge(Composition::PARALLEL, transitions[i.index], transitions[j.index]);
					erase(i);
					erase(ni);
					erase(pi);
					affect = true;
					break;
				}
			}
		}
		return affect;
	}

	vector<strand> compute_strands(set<petri::iterator> from, set<petri::iterator> to, set<petri::iterator> excl=set<petri::iterator>()) const {
		vector<strand> stack;
		for (auto i = from.begin(); i != from.end(); i++) {
			stack.push_back(strand({*i}));
		}

		vector<strand> result;
		while (not stack.empty()) {
			strand curr = stack.back();
			stack.pop_back();
			if (excl.find(curr.back()) != excl.end()) {
				continue;
			} else if (to.find(curr.back()) != to.end()) {
				result.push_back(curr);
				continue;
			}

			vector<petri::iterator> n = next(curr.back());
			for (auto i = n.begin(); i != n.end(); i++) {
				if (not curr.contains(*i) and from.find(*i) == from.end()) {
					stack.push_back(curr);
					stack.back().push_back(*i);
				}
			}
		}

		return result;
	}

	bool is_redundant(petri::iterator r) const {
		// TODO(edward.bingham) check reset

		vector<petri::iterator> from = prev(r, true), to = next(r, true);
		vector<strand> strands = compute_strands(
			set(from.begin(), from.end()),
			set(to.begin(), to.end()),
			{r});

		// check sequencing constraint
		if (strands.empty()) {
			return false;
		}

		vector<array<petri::iterator, 2> > bounds;
		for (auto i = strands.begin(); i != strands.end(); i++) {
			bounds.push_back({i->nodes[0], i->nodes.back()});
		}
		sort(bounds.begin(), bounds.end());
		bounds.erase(unique(bounds.begin(), bounds.end()), bounds.end());
		if (bounds.size() != from.size()*to.size()) {
			return false;
		}
		bounds.clear();

		// check condition constraint
		for (auto i = strands.begin(); i != strands.end(); i++) {
			for (auto j = std::next(i); j != strands.end(); j++) {
				if (i->back() != j->back()) {
					bool found = false;
					vector<petri::iterator> shared = find_last_shared(*i, *j);
					for (auto k = shared.begin(); k != shared.end() and not found; k++) {
						if (k->type == r.type) {
							found = true;
						}
					}
					if (not found) {
						continue;
					}

					if (i->back() < j->back()) {
						bounds.push_back({i->back(), j->back()});
					} else {
						bounds.push_back({j->back(), i->back()});
					}
				}
			}
		}
		sort(bounds.begin(), bounds.end());
		bounds.erase(unique(bounds.begin(), bounds.end()), bounds.end());
		if (bounds.size() != to.size()*(to.size()-1)) {
			return false;
		}

		for (auto i = strands.begin(); i != strands.end(); i++) {
			for (auto j = std::next(i); j != strands.end(); j++) {
				if (i->nodes[0] != j->nodes[0]) {
					bool found = false;
					vector<petri::iterator> shared = find_first_shared(*i, *j);
					for (auto k = shared.begin(); k != shared.end() and not found; k++) {
						if (k->type == r.type) {
							found = true;
						}
					}
					if (not found) {
						continue;
					}

					if (i->nodes[0] < j->nodes[0]) {
						bounds.push_back({i->nodes[0], j->nodes[0]});
					} else {
						bounds.push_back({j->nodes[0], i->nodes[0]});
					}
				}
			}
		}
		sort(bounds.begin(), bounds.end());
		bounds.erase(unique(bounds.begin(), bounds.end()), bounds.end());
		if (bounds.size() != from.size()*(from.size()-1)) {
			return false;
		}
		return true;
	}

	virtual bool remove_redundant() {
		bool affect = false;
		for (petri::iterator i(place::type, 0); i < (int)places.size(); i++) {
			if (not is_valid(i)) continue;

			if (is_redundant(i)) {
				erase(i);
				affect = true;
			}
		}

		return affect;
	}

	// reduce() simplifies the petri net while preserving functional
	// correctness
	//
	// - proper_nesting - preserve hierarchy and proper nesting
	// - aggressive - experimental, apply additional reductions that
	//     may make assumptions about the behavior
	//
	// Apply reduction rules iteratively until no more are possible:
	// - Removes infeasible transitions (can never be enabled)
	// - Removes vacuous transitions (don't affect behavior)
	// - Removes unreachable places
	// - Merges functionally equivalent places
	//
	// Returns whether any reductions were successfully performed
	virtual bool reduce(bool proper_nesting = true, bool aggressive = false, bool debug = false) {
		if (debug) cout << "starting petri::reduce() at " << places.count() << " places and " << transitions.count() << " transitions" << endl;

		bool result = false;
		bool change = true;
		while (change) {
			change = false;

			if (debug) cout << "reducing from " << places.count() << " places and " << transitions.count() << " transitions" << endl;

			change = remove_infeasible(debug) or change;
			change = pinch_vacuous(proper_nesting, debug) or change;
			change = remove_redundant() or change;
			change = compose_internal(aggressive, debug) or change;
			change = remove_vacuous_loops() or change;
			result = result or change;
		}

		if (debug) cout << "ending petri::reduce() at " << places.count() << " places and " << transitions.count() << " transitions" << endl;

		return result;
	}

	virtual bool is_floating(petri::iterator n) const {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < (int)arcs[i].size(); j++) {
				if (arcs[i][j].from == n or arcs[i][j].to == n) {
					return false;
				}
			}
		}
		return true;
	}

	virtual bool is_reset(petri::iterator i) const {
		for (int j = 0; j < (int)reset.size(); j++) {
			for (int k = 0; k < (int)reset[j].tokens.size(); k++) {
				if (reset[j].tokens[k].index == i.index) {
					return true;
				}
			}
		}
		return false;
	}

	Adjacency adjacency() const {
		Adjacency result;
		result.p[place::type].alloc(places.size());
		result.p[transition::type].alloc(transitions.size());
		result.n[place::type].alloc(places.size());
		result.n[transition::type].alloc(transitions.size());
		for (size_t i = 0; i < places.size(); i++) {
			if (not places.is_valid(i)) continue;
			result.n[place::type].emplace_at(i);
			result.p[place::type].emplace_at(i);
		}
		for (size_t i = 0; i < transitions.size(); i++) {
			if (not transitions.is_valid(i)) continue;
			result.n[transition::type].emplace_at(i);
			result.p[transition::type].emplace_at(i);
		}

		for (int type = 0; type < 2; type++) {
			for (int i = 0; i < (int)arcs[type].size(); i++) {
				result.n[type][arcs[type][i].from.index].push_back(arcs[type][i].to);
				result.p[1-type][arcs[type][i].to.index].push_back(arcs[type][i].from);
			}
		}
		for (int i = 0; i < (int)reset.size(); i++) {
			result.reset.push_back(std::vector<petri::iterator>());
			for (int j = 0; j < (int)reset[i].tokens.size(); j++) {
				result.p[place::type][reset[i].tokens[j].index].push_back(petri::iterator(transition::type, -i-1));
				result.reset.back().push_back(petri::iterator(place::type, reset[i].tokens[j].index));
			}
		}
		return result;
	}

	Adjacency forwardAdjacency() const {
		Adjacency result;
		result.n[place::type].alloc(places.size());
		result.n[transition::type].alloc(transitions.size());
		for (size_t i = 0; i < places.size(); i++) {
			if (not places.is_valid(i)) continue;
			result.n[place::type].emplace_at(i);
		}
		for (size_t i = 0; i < transitions.size(); i++) {
			if (not transitions.is_valid(i)) continue;
			result.n[transition::type].emplace_at(i);
		}

		for (int type = 0; type < 2; type++) {
			for (int i = 0; i < (int)arcs[type].size(); i++) {
				result.n[type][arcs[type][i].from.index].push_back(arcs[type][i].to);
			}
		}
		for (int i = 0; i < (int)reset.size(); i++) {
			result.reset.push_back(std::vector<petri::iterator>());
			for (int j = 0; j < (int)reset[i].tokens.size(); j++) {
				result.reset.back().push_back(petri::iterator(place::type, reset[i].tokens[j].index));
			}
		}
		return result;
	}

	Adjacency backwardAdjacency() const {
		Adjacency result;
		result.p[place::type].alloc(places.size());
		result.p[transition::type].alloc(transitions.size());
		for (size_t i = 0; i < places.size(); i++) {
			if (not places.is_valid(i)) continue;
			result.p[place::type].emplace_at(i);
		}
		for (size_t i = 0; i < transitions.size(); i++) {
			if (not transitions.is_valid(i)) continue;
			result.p[transition::type].emplace_at(i);
		}

		for (int type = 0; type < 2; type++) {
			for (int i = 0; i < (int)arcs[type].size(); i++) {
				result.p[1-type][arcs[type][i].to.index].push_back(arcs[type][i].from);
			}
		}
		for (int i = 0; i < (int)reset.size(); i++) {
			result.reset.push_back(std::vector<petri::iterator>());
			for (int j = 0; j < (int)reset[i].tokens.size(); j++) {
				result.p[place::type][reset[i].tokens[j].index].push_back(petri::iterator(transition::type, -i-1));
				result.reset.back().push_back(petri::iterator(place::type, reset[i].tokens[j].index));
			}
		}
		return result;
	}

	virtual void print() const {
		for (int i = 0; i < (int)places.size(); i++) {
			if (not places.is_valid(i)) continue;

			cout << "p" << i << ": " << places[i] << endl;
		}
		for (int i = 0; i < (int)transitions.size(); i++) {
			if (not transitions.is_valid(i)) continue;

			cout << "t" << i << ": " << transitions[i] << endl;
		}
		for (int type = 0; type < 2; type++) {
			for (int i = 0; i < (int)arcs[type].size(); i++) {
				cout << arcs[type][i].from << "->" << arcs[type][i].to << endl;
			}
		}
	}

	//TODO: virtual void unzip_backwards(petri::iterator from, petri::iterator to) {}
	virtual petri::iterator unzip_forwards(petri::iterator node) {
		//TODO: support more than simple linear path to unzip
		vector<petri::iterator> parents = this->prev(node);
		if (parents.size() < 2) { return node; }	// Nothing to unzip!

		//TODO: support depths greater than 1
		vector<petri::iterator> children = this->next(node);
		if (children.size() < 1) { return node; }  // Nowhere to unzip!

		//TODO: assumes completely connected in single direction (all children point to single grandchild)
		// "cousin" is neighbor-in-kind (e.g. place->place)
		vector<petri::iterator> out_cousins = this->next(children[0]);
		petri::iterator &out_cousin = out_cousins[0];

		for (auto parent = std::next(parents.begin()); parent != parents.end(); parent++) {
			petri::iterator parent_out_arc = this->arc_between(*parent, node);
			//TODO: verify arc_between return is valid: if (parent_out_arc == petri::iterator()) {}
			this->erase_arc(parent_out_arc);

			//TODO: careful, don't duplicate the MARKING on this one
			petri::iterator duplicate_node = this->copy(node);
			this->connect(*parent, duplicate_node);

			for (const petri::iterator &child : children) {
				petri::iterator duplicate_child = this->copy(child);
				this->connect(duplicate_node, duplicate_child);
				this->connect(duplicate_child, out_cousin);
			}
		}

		return out_cousin;
	}
};

}
