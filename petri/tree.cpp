#include "tree.h"

namespace controlflow {

Index::Index() {
	type = TRANSITION;
	index = -1;
}

Index::Index(int type, int index) {
	this->type = type;
	this->index = index;
}

Index::~Index() {
}

Node::Node(petri::composition comp) {
	this->comp = comp;
}

~Node() {
}

}


