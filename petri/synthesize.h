#pragma once

namespace petri {

template <class process, class place, class transition, class token, class state>
bool graph_to_tree(graph<place, controlflow::Tree<process>, token, state> &t, const graph<place, transition, token, state> &g) {
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
		t.transitions.emplace_at(i, controlflow::Tree<process>(i));
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
				controlflow::Tree<process> loop = t.transitions[p[0].index].loop();
				t.erase(i);
				t.insert_after(p[0], loop);
				found = true;
				continue;
			}

			std::vector<petri::iterator> pn = t.next(p, true);
			std::vector<petri::iterator> np = t.prev(n, true);
			if (pn != np) {
				continue;
			}

			t.transitions[p[0].index].compose(controlflow::Node::SEQUENCE, t.transitions[n[0].index]);

			for (size_t j = 0; j < t.arcs[n[0].type].size(); j++) {
				if (t.arcs[n[0].type][j].from.index == n[0].index) {
					t.arcs[n[0].type][j].from.index = p[0].index;
				}
			}

			t.erase(n[0]);
			t.erase(i);
			found = true;
		}

		// look for conditional and parallel composition
		for (petri::iterator i = t.begin(transition::type); i != t.end(transition::type); i++) {
			if (not t.is_valid(i)) continue;

			std::vector<petri::iterator> n = t.next(i);
			if (n.size() > 1u) {
				// handle parallel composition
				bool hasParallel = true;
				std::vector<petri::iterator> nn;
				petri::iterator merge;
				std::vector<petri::iterator> toErase;
				// loop through split places
				for (size_t j = 0; j < n.size(); j++) {
					petri::iterator nj = n[j];
					if (t.is_merge(nj)) {
						hasParallel = false;
						break;
					}

					// step through parallel transitions
					std::vector<petri::iterator> nnj = t.next(nj);
					if (nnj.size() != 1u or t.is_merge(nnj[0])) {
						hasParallel = false;
						break;
					}

					// step through merge places
					std::vector<petri::iterator> nnnj = t.next(nnj[0]);
					if (nnnj.size() != 1u or t.is_merge(nnnj[0])) {
						hasParallel = false;
						break;
					}

					// check merge
					std::vector<petri::iterator> nnnnj = t.next(nnnj[0]);
					if (nnnnj.size() != 1u) {
						hasParallel = false;
						break;
					}

					if (not merge.valid()) {
						merge = nnnnj[0];
					} else if (merge != nnnnj[0]) {
						hasParallel = false;
						break;
					}

					if (j != n.size()-1) {
						toErase.push_back(nj);
						toErase.push_back(nnj[0]);
						toErase.push_back(nnnj[0]);
					}
					nn.push_back(nnj[0]);
				}

				if (hasParallel) {
					for (size_t j = 0; j < nn.size()-1; j++) {
						t.transitions[nn.back().index].compose(controlflow::Node::PARALLEL, t.transitions[nn[j].index]);
					}
					t.erase(toErase);
					n[0] = n.back();
					n.erase(n.begin()+1, n.end());
					found = true;
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
				controlflow::Tree<process> loop = t.transitions[i.index].loop();
				t.erase(i);
				t.insert_after(p[0], loop);
				found = true;
				continue;
			}

			std::vector<petri::iterator> pn = t.next(p, true);
			std::vector<petri::iterator> np = t.prev(n, true);
			if (pn != np or pn.size() <= 1u) {
				continue;
			}

			bool hasChoice = true;
			for (size_t i = 0; i < pn.size() and hasChoice; i++) {
				hasChoice = not t.is_merge(pn[i]);
			}

			for (size_t i = 0; i < np.size() and hasChoice; i++) {
				hasChoice = not t.is_split(np[i]);
			}

			if (not hasChoice) {
				continue;
			}

			// found a conditional composition
			for (size_t j = 0; j < pn.size()-1; j++) {
				t.transitions[pn.back().index].compose(controlflow::Node::CHOICE, t.transitions[pn[j].index]);
			}
			pn.pop_back();
			t.erase(pn);
			found = true;
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
				if (proc.type != controlflow::Index::TRANSITION) continue;

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

template <class process, class place, class transition, class token, class state>
void tree_to_graph(graph<place, transition, token, state> &g, const controlflow::Tree<process> &t) {
	if (t.root < 0) {
		return;
	}

	Mapping<size_t> tmap(-1, false);
	for (size_t i = 0; i < t.transitions.size(); i++) {
		t.map.set(i, g.create(transition(t.transitions[i])).index);
	}

	std::map<size_t, petri::segment> nodes;
	std::vector<size_t> stack(1, t.root);
	while (not stack.empty()) {
		std::vector<petri::segment> segments;
		bool found = true;
		for (const auto &proc : t.nodes[stack.back()].procs) {
			if (proc.type == controlflow::Index::TRANSITION) {
				petri::iterator it(transition::type, tmap.map(proc.index));
				segments.push_back(petri::segment({{it}}, {{it}}));
			} else if (proc.type == controlflow::Index::NODE) {
				auto pos = nodes.find(proc.index);
				if (pos == nodes.end()) {
					found = false;
					stack.push_back(proc.index);
				} else {
					segments.push_back(pos->second);
				}
			}
		}

		if (not found) {
			continue;
		}

		size_t curr = stack.back();
		stack.pop_back();

		petri::segment result;
		// TODO(edward.bingham) see interpret_chp/import_expr.h How do
		// we handle features of the graph that petri doesn't know
		// about?

		int composition = -1;
		if (t.nodes[curr].comp == controlflow::Node::CHOICE) {
			composition = petri::choice;
		} else if (t.nodes[curr].comp == controlflow::Node::PARALLEL) {
			composition = petri::parallel;
		} else if (t.nodes[curr].comp == controlflow::Node::LOOP) {
			composition = petri::choice;
			for (auto &segment : segments) {
				segment = g.loop(segment);
			}
		} else { //if (t.nodes[curr].comp == controlflow::Node::SEQUENCE) {
			composition = petri::sequence;
		}

		for (const auto &segment : segments) {
			result = g.compose(composition, result, segment, true);
		}
		nodes.insert({curr, result});
	}
}

}
