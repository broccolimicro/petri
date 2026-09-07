#pragma once

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>

#include <array>

namespace petri {

enum Composition {
	CHOICE = 0,
	PARALLEL = 1,
	SEQUENCE = 2,
	IMPLIES = 3,
	EXCLUDES = 4
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
