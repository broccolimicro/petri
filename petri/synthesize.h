#pragma once

namespace petri {

template <class process, class place, class transition, class state>
bool compress_loops(graph<place, controlflow::Tree<process, place>, state> &t, const graph<place, transition, state> &g) {
	bool found = false;
	for (petri::iterator i = t.begin(place::type); i != t.end(place::type); i++) {
		if (not t.is_valid(i)) continue;

		std::vector<petri::iterator> p = t.prev(i, true);
		std::vector<petri::iterator> n = t.next(i, true);
		std::vector<petri::iterator> loops = vector_intersection(p, n);
		if (loops.empty()) {
			continue;
		}

		p = vector_difference(p, loops);
		if (p.size() > 1u) {
			continue;
		}
		n = vector_difference(n, loops);
		if (n.size() > 1u) {
			continue;
		}

		// found a conditional composition
		controlflow::Tree<process, place> loop = t.transitions[loops[0].index];
		for (size_t j = 1; j < loops.size(); j++) {
			loop.merge(petri::CHOICE, t.transitions[loops[j].index]);
		}
		loop.top().place = i.index;
		loop.loop();
		loop.top().place = i.index;
		t.erase(loops);
		t.insert_after(i, loop);
		found = true;
	}
	return found;
}

template <class process, class place, class transition, class state>
bool compress_sequence(graph<place, controlflow::Tree<process, place>, state> &t, const graph<place, transition, state> &g) {
	bool found = false;
	for (petri::iterator i = t.begin(place::type); i < t.end(place::type); i++) {
		if (not t.is_valid(i)) continue;

		std::vector<petri::iterator> p = t.prev(i);
		std::vector<petri::iterator> n = t.next(i);
		if (p.size() != 1u or n.size() != 1u or n[0] == p[0]) {
			continue;
		}

		std::vector<petri::iterator> pn = t.next(p, true);
		std::vector<petri::iterator> np = t.prev(n, true);
		if (pn != np) {
			continue;
		}

		
		t.erase(i);
		t.transitions[p[0].index].merge(petri::SEQUENCE, t.transitions[n[0].index]);
		for (size_t j = 0; j < t.arcs[n[0].type].size(); j++) {
			if (t.arcs[n[0].type][j].from.index == n[0].index) {
				t.arcs[n[0].type][j].from.index = p[0].index;
			}
		}
		t.erase(n[0]);
		found = true;
	}
	return found;
}

template <class process, class place, class transition, class state>
bool compress_choice(graph<place, controlflow::Tree<process, place>, state> &t, const graph<place, transition, state> &g) {
	bool found = false;
	for (petri::iterator i = t.begin(place::type); i < t.end(place::type); i++) {
		if (not t.is_valid(i)) continue;

		std::vector<petri::iterator> n = t.next(i);
		if (n.size() <= 1u) {
			continue;
		}

		petri::iterator merge;
		bool hasChoice = true;
		for (size_t j = 0; j < n.size(); j++) {
			std::vector<petri::iterator> pj = t.prev(n[j], true);
			std::vector<petri::iterator> nj = t.next(n[j], true);
			if (pj.size() != 1u or nj.size() > 1u) {
				hasChoice = false;
				break;
			}
			if (nj.size() == 1u) {
				std::vector<petri::iterator> nnj = t.next(nj[0]);
				if (not nnj.empty()) {
					if (not merge.valid()) {
						merge = nj[0];
					} else if (nj[0] != merge) {
						hasChoice = false;
						break;
					}
				}
			}
		}
		if (not hasChoice or merge == i) {
			continue;
		}

		// found a conditional composition
		for (size_t j = 1; j < n.size(); j++) {
			t.transitions[n[0].index].merge(petri::CHOICE, t.transitions[n[j].index]);
		}
		t.transitions[n[0].index].top().place = i.index;
		n.erase(n.begin());
		t.erase(n);
		found = true;
	}
	return found;
}

template <class process, class place, class transition, class state>
bool compress_parallel(graph<place, controlflow::Tree<process, place>, state> &t, const graph<place, transition, state> &g) {
	bool found = false;
	for (petri::iterator i = t.begin(transition::type); i != t.end(transition::type); i++) {
		if (not t.is_valid(i)) continue;

		std::vector<petri::iterator> n = t.next(i);
		if (n.size() <= 1u) {
			continue;
		}

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

			if (j != 0) {
				toErase.push_back(nj);
				toErase.push_back(nnj[0]);
				toErase.push_back(nnnj[0]);
			}
			nn.push_back(nnj[0]);
		}
		if (not hasParallel) {
			continue;
		}

		for (size_t j = nn.size()-1; j > 0; j--) {
			t.transitions[nn[0].index].merge(petri::PARALLEL, t.transitions[nn[j].index]);
		}
		t.erase(toErase);
		n.erase(n.begin()+1, n.end());
		found = true;
	}
	return found;
}


template <class process, class place, class transition, class state>
bool graph_to_tree(graph<place, controlflow::Tree<process, place>, state> &t, const graph<place, transition, state> &g) {
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
		t.transitions.emplace_at(i, controlflow::Tree<process, place>(i));
	}

	// Then iteratively find minimal compositions and merge them into nodes. If
	// there are no more compositions left, then return whether t has one
	// transition.
	bool found = true;
	while (found) {
		while (found) {
			while (found) {
				found = false;
				if (compress_choice(t, g)) {
					found = true;
				}

				if (compress_parallel(t, g)) {
					found = true;
				}
			}

			if (compress_sequence(t, g)) {
				found = true;
			}
		}

		if (compress_loops(t, g)) {
			found = true;
		}
	}

	std::vector<petri::iterator> procs;
	for (petri::iterator i = t.begin(transition::type); i != t.end(transition::type); i++) {
		if (not t.is_valid(i)) continue;

		std::vector<petri::iterator> p = t.prev(i);
		std::vector<petri::iterator> n = t.next(i);
		if (p.size() > 1u or n.size() > 1u or (n.size() == 1u and p.size() == 1u and n == p)) {
			continue;
		}

		if (not p.empty()) {
			std::vector<petri::iterator> pn = t.next(p);
			if (pn.size() != 1u or pn[0] != i) {
				continue;
			}
			std::vector<petri::iterator> pp = t.prev(p);
			if (not pp.empty()) {
				continue;
			}
		}

		if (not n.empty()) {
			std::vector<petri::iterator> np = t.prev(n);
			if (np.size() != 1u or np[0] != i) {
				continue;
			}
			std::vector<petri::iterator> nn = t.next(n);
			if (not nn.empty()) {
				continue;
			}
		}

		procs.push_back(i);
	}

	if (procs.size() > 1u) {
		for (size_t j = procs.size()-1; j > 0; j--) {
			t.transitions[procs[0].index].merge(petri::PARALLEL, t.transitions[procs[j].index]);
		}
		procs.erase(procs.begin());
		t.erase(procs);
	}

	for (size_t i = 0; i < t.transitions.size(); i++) {
		if (not t.transitions.is_valid(i)) continue;

		auto &tree = t.transitions[i];

		Mapping<size_t> tmap(-1, false);
		Mapping<size_t> pmap(-1, false);
		for (size_t j = 0; j < tree.nodes.size(); j++) {
			if (not tree.nodes.is_valid(j)) continue;

			auto &node = tree.nodes[j];

			for (auto &proc : node.procs) {
				if (proc.type == controlflow::Index::TRANSITION) {
					size_t idx = tmap.map(proc.index);
					if (idx == tmap.undef) {
						idx = tree.transitions.insert(g.transitions[proc.index]);
						tmap.set(proc.index, idx);
					}

					proc.index = idx;
				}
			}

			if (node.place >= 0) {
				size_t idx = pmap.map(node.place);
				if (idx == pmap.undef) {
					idx = tree.places.insert(g.places[node.place]);
					pmap.set(node.place, idx);
				}

				node.place = idx;
			}
		}
	}

	return t.transitions.count() == 1u;
}

template <class process, class place, class transition, class state>
void tree_to_graph(graph<place, transition, state> &g, const controlflow::Tree<process, place> &t) {
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
			composition = petri::Composition::CHOICE;
		} else if (t.nodes[curr].comp == controlflow::Node::PARALLEL) {
			composition = petri::Composition::PARALLEL;
		} else if (t.nodes[curr].comp == controlflow::Node::LOOP) {
			composition = petri::Composition::CHOICE;
			for (auto &segment : segments) {
				segment = g.loop(segment);
			}
		} else { //if (t.nodes[curr].comp == controlflow::Node::SEQUENCE) {
			composition = petri::Composition::SEQUENCE;
		}

		for (const auto &segment : segments) {
			result = g.merge(composition, result, segment, true);
		}
		nodes.insert({curr, result});
	}
}

}
