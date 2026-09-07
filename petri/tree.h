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
	enum Type {
		CHOICE = 0,
		PARALLEL = 1, 
		SEQUENCE = 2,
		LOOP = 3,
	};

	Type type;
	std::vector<Index> procs;

	Node(Type type=SEQUENCE);
	~Node();
};

template <class Transition>
struct Tree : petri::transition {
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

	bool mergeable(petri::Composition composition, const Tree<Transition> &t1) const {
		return composition == petri::CHOICE
			or composition == petri::PARALLEL
			or composition == petri::SEQUENCE;
	}

	Tree<Transition> &merge(petri::Composition composition, const Tree<Transition> &t1) {
		if (t1.root < 0) {
			return *this;
		}

		Node::Type type = Node::SEQUENCE;
		if (composition == petri::CHOICE) {
			type = Node::CHOICE;
		} else if (composition == petri::PARALLEL) {
			type = Node::PARALLEL;
		} else if (composition == petri::SEQUENCE) {
			type = Node::SEQUENCE;
		} else {
			internal("", "unsupported composition for tree merge", __FILE__, __LINE__);
			return *this;
		}

		if (root < 0) {
			root = nodes.emplace(type);
		} else if (nodes[root].type != type) {
			int newRoot = nodes.emplace(type);
			nodes[newRoot].procs.push_back(Index(Index::NODE, root));
			root = newRoot;
		}

		Mapping<int> tMap(-1, true);
		Mapping<int> nMap(-1, false);
		// insert all transitions and nodes
		if (not isExternal() and not t1.isExternal()) {
			// transitions are internally indexed, need to remap them
			tMap.identity = false;

			for (size_t i = 0; i < t1.transitions.size(); i++) {
				if (t1.transitions.is_valid(i)) {
					tMap.set(i, transitions.insert(t1.transitions[i]));
				}
			}
		}

		for (size_t i = 0; i < t1.nodes.size(); i++) {
			nMap.set(i, nodes.insert(t1.nodes[i]));
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

		nodes[root].procs.push_back(Index(Index::NODE, nMap.map(t1.root)));
		return *this;
	}

	void print() const {
		if (root < 0) {
			return;
		}

		struct Frame {
			int index;
			int process;
		};

		std::vector<Frame> stack(1, {root, 0});
		while (not stack.empty()) {
			int curr = stack.size()-1;
			int index = stack[curr].index;
			int proc = stack[curr].process;

			if (proc == 0) {
				if (nodes[index].type == Node::CHOICE) {
					printf("[");
				} else if (nodes[index].type == Node::PARALLEL) {
					printf("(");
				} else if (nodes[index].type == Node::SEQUENCE) {
				} else if (nodes[index].type == Node::LOOP) {
					printf("*[");
				}
			} else if (proc < (int)nodes[index].procs.size()) {
				if (nodes[index].type == Node::CHOICE) {
					printf(":");
				} else if (nodes[index].type == Node::PARALLEL) {
					printf("||");
				} else if (nodes[index].type == Node::SEQUENCE) {
					printf(";");
				} else if (nodes[index].type == Node::LOOP) {
					printf(":");
				}
			} else {
				if (nodes[index].type == Node::CHOICE) {
					printf("]");
				} else if (nodes[index].type == Node::PARALLEL) {
					printf(")");
				} else if (nodes[index].type == Node::SEQUENCE) {
				} else if (nodes[index].type == Node::LOOP) {
					printf("]");
				}
				stack.pop_back();
				continue;
			}

			if (nodes[index].procs[proc].type == Index::TRANSITION) {
				printf("T%d", nodes[index].procs[proc].index);
			} else if (nodes[index].procs[proc].type == Index::NODE) {
				stack.push_back({nodes[index].procs[proc].index, 0});
			}

			stack[curr].process++;
		}
	}
};



}
