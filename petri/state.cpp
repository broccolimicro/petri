/*
 * state.cpp
 *
 *  Created on: Jun 23, 2015
 *      Author: nbingham
 */

#include "state.h"

namespace petri
{

term_index::term_index()
{
	index = -1;
}

term_index::term_index(int index)
{
	this->index = index;
}

term_index::~term_index()
{

}

void term_index::hash(hasher &hash) const
{
	hash.put(&index);
}

bool operator<(term_index i, term_index j)
{
	return i.index < j.index;
}

bool operator>(term_index i, term_index j)
{
	return i.index > j.index;
}

bool operator<=(term_index i, term_index j)
{
	return i.index <= j.index;
}

bool operator>=(term_index i, term_index j)
{
	return i.index >= j.index;
}

bool operator==(term_index i, term_index j)
{
	return i.index == j.index;
}

bool operator!=(term_index i, term_index j)
{
	return i.index != j.index;
}

token::token()
{
	this->index = 0;
}

token::token(int index)
{
	this->index = index;
}

token::~token()
{
}

void token::hash(hasher &hash) const
{
	hash.put(&index);
}

bool operator<(token i, token j)
{
	return i.index < j.index;
}

bool operator>(token i, token j)
{
	return i.index > j.index;
}

bool operator<=(token i, token j)
{
	return i.index < j.index;
}

bool operator>=(token i, token j)
{
	return i.index > j.index;
}

bool operator==(token i, token j)
{
	return i.index == j.index;
}

bool operator!=(token i, token j)
{
	return i.index != j.index;
}

}
