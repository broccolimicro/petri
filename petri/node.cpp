#include "node.h"
#include <common/message.h>
#include <common/text.h>
#include <limits>

namespace petri {

Composition invert(Composition t) {
	if (t == Composition::PARALLEL) {
		return Composition::CHOICE;
	} else if (t == Composition::CHOICE) {
		return Composition::PARALLEL;
	} else if (t == Composition::IMPLIES) {
		return Composition::EXCLUDES;
	} else if (t == Composition::EXCLUDES) {
		return Composition::IMPLIES;
	}
	return t;
}

place::place() {
}

place::~place() {
}

place &place::merge(Composition composition, const place &p1) {
	return *this;
}

ostream &operator<<(ostream &os, const place &p) {
	return os;
}

transition::transition() {
}

transition::~transition() {
}

transition &transition::merge(Composition composition, const transition &t1) {
	return *this;
}

bool transition::mergeable(Composition composition, const transition &t1) const {
	return true;
}

bool transition::is_infeasible() const {
	return false;
}

bool transition::is_vacuous() const {
	return false;
}

ostream &operator<<(ostream &os, const transition &t) {
	return os;
}

}

