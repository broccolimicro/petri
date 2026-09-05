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
				Tree<transition> loop = t.transitions[p[0].index].loop();
				t.erase(i);
				t.insert_after(p[0], loop);
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

			std::vector<petri::iterator> n = t.next(i);
			if (n.size() > 1u) {
				// handle parallel composition
				bool found = true;
				std::vector<petri::iterator> nn;
				std::vector<petri::iterator> toErase;
				// loop through split places
				for (size_t j = 0; j < n.size(); j++) {
					petri::iterator nj = n[j];
					if (t.is_merge(nj)) {
						found = false;
						break;
					}

					// step through parallel transitions
					std::vector<petri::iterator> nnj = t.next(nj);
					if (nnj.size() != 1u or t.is_merge(nnj[0])) {
						found = false;
						break;
					}

					// step through merge places
					std::vector<petri::iterator> nnnj = t.next(nnj[0]);
					if (nnnj.size() != 1u or t.is_merge(nnnj[0])) {
						found = false;
						break;
					}

					// check merge
					std::vector<petri::iterator> nnnnj = t.next(nnnj[0]);
					if (nnnnj.size() != 1u) {
						found = false;
						break;
					}

					if (nnnn.empty()) {
						nnnn.push_back(nnnnj[0]);
					} else if (nnnn[0] != nnnnj[0]) {
						found = false;
						break;
					}

					if (j != n.size()-1) {
						toErase.push_back(nj);
						toErase.push_back(nnj[0]);
						toErase.push_back(nnnj[0]);
					}
					nn.push_back(nnj[0]);
				}

				if (found) {
					for (size_t j = 0; j < nnj.size()-1; j++) {
						t.transitions[nnj.back().index].compose(Node::PARALLEL, t.transitions[nnj[j].index]);
					}
					t.erase(toErase);
					n[0] = n.back();
					n.erase(n.begin()+1, n.end());
				}
			}
			if (n.size() != 1u) {
				continue;
			}

			std::vector<petri::iterator> p = t.prev(i);
			if (p.size() != 1u) {
				continue;
			}

			if (p[0] == n[0]) {
				Tree<transition> loop = t.transitions[i.index].loop();
				t.erase(i);
				t.insert_after(p[0], loop);
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
		}
	}

	for (size_t i = 0; i < t.transitions.size(); i++) {
		if (not t.transitions.is_valid(i)) continue;

		auto &tree = t.transitions[i];

		Mapping<size_t> tmap(-1, false);
		for (size_t j = 0; j < tree.nodes.size(); j++) {
			if (not tree.nodes.is_valid(j)) continue;

			auto &node = tree.nodes[j];

			for (auto &proc : node.procs) {
				if (proc.type != Index::TRANSITION) continue;

				size_t idx = tmap.map(proc.index);
				if (idx == tmap.undef) {
					idx = tree.transitions.insert(g.transitions[proc.index]);
					tmap.set(proc.index, idx);
				}

				proc.index = idx;
			}
		}
	}

	return t.transitions.count() == 1u;
}

}
