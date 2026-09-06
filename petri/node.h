#pragma once

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>

#include <array>

namespace petri {

struct place
{
	place();
	~place();

	static const int type = 0;

	static place merge(int composition, const place &p0, const place &p1);
};

ostream &operator<<(ostream &os, const place &p);

struct transition
{
	transition();
	~transition();

	static const int type = 1;

	bool is_infeasible();
	bool is_vacuous();

	static transition merge(int composition, const transition &t0, const transition &t1);
	static bool mergeable(int composition, const transition &t0, const transition &t1);
};

ostream &operator<<(ostream &os, const transition &t);

}
