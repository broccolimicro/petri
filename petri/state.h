/*
 * state.h
 *
 *  Created on: Jun 23, 2015
 *      Author: nbingham
 */

#include <common/standard.h>

#ifndef petri_state_h
#define petri_state_h

namespace petri
{

struct enabled_transition
{
	enabled_transition();
	enabled_transition(int index);
	~enabled_transition();

	int index;
	vector<int> tokens;
};

bool operator<(enabled_transition t0, enabled_transition t1);
bool operator>(enabled_transition t0, enabled_transition t1);
bool operator<=(enabled_transition t0, enabled_transition t1);
bool operator>=(enabled_transition t0, enabled_transition t1);
bool operator==(enabled_transition t0, enabled_transition t1);
bool operator!=(enabled_transition t0, enabled_transition t1);

struct token
{
	token();
	token(int index);
	~token();

	int index;

	void hash(hasher &hash) const;
};

bool operator<(token t0, token t1);
bool operator>(token t0, token t1);
bool operator<=(token t0, token t1);
bool operator>=(token t0, token t1);
bool operator==(token t0, token t1);
bool operator!=(token t0, token t1);

template <class token>
struct state
{
	state() {}
	state(vector<token> tokens)
	{
		this->tokens = tokens;
	}
	~state() {}

	vector<token> tokens;

	//static state merge(const state &s0, const state &s1);
	//static state collapse(int index, const state &s);
	//state convert(map<petri::iterator, petri::iterator> translate);
};

template <class token>
bool operator<(state<token> t0, state<token> t1)
{
	return t0.tokens < t1.tokens;
}

template <class token>
bool operator>(state<token> t0, state<token> t1)
{
	return t0.tokens > t1.tokens;
}

template <class token>
bool operator<=(state<token> t0, state<token> t1)
{
	return t0.tokens <= t1.tokens;
}

template <class token>
bool operator>=(state<token> t0, state<token> t1)
{
	return t0.tokens >= t1.tokens;
}

template <class token>
bool operator==(state<token> t0, state<token> t1)
{
	return t0.tokens == t1.tokens;
}

template <class token>
bool operator!=(state<token> t0, state<token> t1)
{
	return t0.tokens != t1.tokens;
}

}

#endif
