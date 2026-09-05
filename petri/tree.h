#pragma once

#include <vector>

#include "node.h"
#include <common/mapping.h>
#include <common/index_vector.h>

namespace controlflow {

struct Index {
	enum Type {
		TRANSITION = 0,
		NODE = 1
	};

	Type type;
	int index;

	Index();
	Index(Type type, int index);
	~Index();
};

struct Node {
	enum Composition {
		CHOICE = 0,
		PARALLEL = 1, 
		SEQUENCE = 2,
		LOOP = 3,
	};

	Composition comp;
	std::vector<Index> procs;

	Node(Composition comp=SEQUENCE);
	~Node();
};

template <class Transition>
struct Tree {
	index_vector<Node> nodes;

	// DESIGN(edward.bingham) if transitions is empty, but the graph is filled,
	// then the transition indices point to an array external to this structure.
	// See isExternal().
	index_vector<Transition> transitions;
	int root;

	Tree(int index=-1) {
		root = -1;
		if (index >= 0) {
			root = nodes.emplace();
			nodes[root].procs.push_back(Index(Index::TRANSITION, index));
		}
	}

	Tree(Transition t) {
		root = nodes.emplace();
		nodes[root].procs.push_back(Index(Index::TRANSITION, transitions.insert(t)));
	}

	~Tree() {
	}

	bool isExternal() const {
		return transitions.empty();
	}

	Tree<Transition> &loop() {
		int newRoot = nodes.emplace(Node::LOOP);
		if (root >= 0) {
			nodes[newRoot].procs.push_back(Index(Index::NODE, root));
		}
		root = newRoot;
		return *this;
	}

	Tree<Transition> &compose(Node::Composition comp, const Tree &tree) {
		if (tree.root < 0) {
			return *this;
		}

		if (root < 0) {
			root = nodes.emplace(comp);
		} else if (nodes[root].comp != comp) {
			int newRoot = nodes.emplace(comp);
			nodes[newRoot].procs.push_back(Index(Index::NODE, root));
			root = newRoot;
		}

		Mapping<int> tMap(-1, true);
		Mapping<int> nMap(-1, false);
		// insert all transitions and nodes
		if (not isExternal() and not tree.isExternal()) {
			// transitions are internally indexed, need to remap them
			tMap.identity = false;

			for (size_t i = 0; i < tree.transitions.size(); i++) {
				if (tree.transitions.is_valid(i)) {
					tMap.set(i, transitions.insert(tree.transitions[i]));
				}
			}
		}

		for (size_t i = 0; i < tree.nodes.size(); i++) {
			nMap.set(i, nodes.insert(tree.nodes[i]));
		}

		for (const auto &p : nMap.fwd) {
			for (Index &idx : nodes[p.second].procs) {
				if (idx.type == Index::TRANSITION) {
					idx.index = tMap.map(idx.index);
				} else {
					idx.index = nMap.map(idx.index);
				}
			}
		}

		nodes[root].procs.push_back(Index(Index::NODE, nMap.map(tree.root)));
		return *this;
	}
};



}
