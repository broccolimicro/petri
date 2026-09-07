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

Node::Node(Node::Type type) {
	this->type = type;
}

Node::~Node() {
}

}


