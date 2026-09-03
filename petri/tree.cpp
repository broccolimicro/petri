#include "tree.h"

namespace controlflow {

Index::Index() {
	type = TRANSITION;
	index = -1;
}

Index::Index(Index::Type type, int index) {
	this->type = type;
	this->index = index;
}

Index::~Index() {
}

Node::Node(Node::Composition comp) {
	this->comp = comp;
}

Node::~Node() {
}

}


