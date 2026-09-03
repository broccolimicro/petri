#pragma once

namespace petri {

template <class place, class transition, class token, class state>
bool graph_to_tree(graph<place, Tree<transition>, token, state> &t, const graph<place, transition, token, state> &g) {
	// first fill the tree
	t.places = g.places;
	t.arcs = g.arcs;
	t.reset = g.reset;
	t.transitions.alloc(g.transitions.size());
	for (size_t i = 0; i < g.transitions.size(); i++) {
		if (not g.transitions.is_valid(i)) {
			continue;
		}
		// We want to fill in the transitions for each tree after we've run the
		// full algorithm. No need to carry that weight around and copy it while
		// we're iterating. See Tree constructor.
		t.transitions.emplace_at(i, Tree<transition>(i));
	}

	// Then iteratively find minimal compositions and merge them into nodes. If
	// there are no more compositions left, then return whether t has one
	// transition.
	bool found = true;
	while (found) {
		found = false;
		// look for sequential composition
		for (petri::iterator i = t.begin(place::type); i != t.end(place::type); i++) {
			if (not t.is_valid(i)) continue;

			std::vector<petri::iterator> p = t.prev(i);
			std::vector<petri::iterator> n = t.next(i);
			if (p.size() != 1u or n.size() != 1u) {
				continue;
			}

			if (p[0] == n[0]) {
				// TODO(edward.bingham) handle loops
				continue;
			}

			std::vector<petri::iterator> pn = t.next(p, true);
			std::vector<petri::iterator> np = t.prev(n, true);
			if (pn != np) {
				continue;
			}

			t.transitions[p[0].index].compose(controlflow::Node::SEQUENCE, t.transitions[n[0].index]);

			for (size_t j = 0; j < arcs[n[0].type].size(); j++) {
				if (arcs[n[0].type][j].from == n[0].index) {
					arcs[n[0].type][j].from = p[0].index;
				}
			}

			erase(n[0]);
			erase(i);
			found = true;
		}

		// look for conditional and parallel composition
		for (petri::iterator i = t.begin(transition::type); i != t.end(transition::type); i++) {
			if (not t.is_valid(i)) continue;

			std::vector<petri::iterator> p = t.prev(i);
			std::vector<petri::iterator> n = t.next(i);
			if (p.size() != 1u or n.size() != 1u) {
				continue;
			}

			if (p[0] == n[0]) {
				// TODO(edward.bingham) handle loops
				continue;
			}

			std::vector<petri::iterator> pn = t.next(p, true);
			std::vector<petri::iterator> np = t.prev(n, true);
			if (pn != np) {
				continue;
			}

			// found a conditional composition
			if (pn.size() > 1u) {
				for (size_t j = 0; j < pn.size()-1; j++) {
					t.transitions[pn.back().index].compose(controlflow::Node::CHOICE, t.transitions[pn[j].index]);
				}
				pn.pop_back();
				erase(pn);
				found = true;
			}

			// TODO(edward.bingham) look for parallel composition
		}
	}

	// TODO(edward.bingham) fill transitions in trees

	return t.transitions.count() == 1u;
}

}
