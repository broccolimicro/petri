/*
 * state.h
 *
 *  Created on: Jun 23, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>
#include "iterator.h"
#include "node.h"

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

	static state<token> merge(const state<token> &s0, const state<token> &s1) {
		state<token> result;

		result.tokens.resize(s0.tokens.size() + s1.tokens.size());
		::merge(s0.tokens.begin(), s0.tokens.end(), s1.tokens.begin(), s1.tokens.end(), result.tokens.begin());
		result.tokens.resize(unique(result.tokens.begin(), result.tokens.end()) - result.tokens.begin());

		return result;
	}

	static state<token> collapse(int index, const state<token> &s) {
		state<token> result;
		result.tokens.push_back(token(index));
		return result;
	}

	state<token> convert(map<petri::iterator, vector<petri::iterator> > translate) const {
		state<token> result;

		for (int i = 0; i < (int)tokens.size(); i++)
		{
			map<petri::iterator, vector<petri::iterator> >::iterator loc = translate.find(petri::iterator(place::type, tokens[i].index));
			if (loc != translate.end()) {
				for (auto j = loc->second.begin(); j != loc->second.end(); j++) {
					result.tokens.push_back(token(j->index));
				}
			}
		}

		return result;
	}
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
