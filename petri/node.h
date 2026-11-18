#pragma once

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>

#include <array>

namespace petri {

// Describes how two nodes relate in the execution of a Petri net.
enum Composition {
	CHOICE = 0, // The nodes belong to alternative branches of a decision.
	PARALLEL = 1, // The nodes belong to concurrent branches that can be active at the same time.
	SEQUENCE = 2, // The nodes occur in order along the same execution, with one preceding the other.
	IMPLIES = 3, // An execution that includes the first node also includes the second.
	EXCLUDES = 4 // An execution can include the first node while leaving out the second.
};

Composition invert(Composition t);

struct place {
	static const int type = 0;

	place();
	~place();

	place &merge(Composition composition, const place &p1);
};

ostream &operator<<(ostream &os, const place &p);

struct transition {
	static const int type = 1;

	transition();
	~transition();

	bool is_infeasible() const;
	bool is_vacuous() const;

	transition &merge(Composition composition, const transition &t1);
	bool mergeable(Composition composition, const transition &t1) const;
};

ostream &operator<<(ostream &os, const transition &t);

}
