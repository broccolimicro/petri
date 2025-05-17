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

	void replace(vector<petri::iterator> from, vector<petri::iterator> to) {
		for (int i = (int)tokens.size()-1; i >= 0; i--) {
			auto loc = find(from.begin(), from.end(), petri::iterator(place::type, tokens[i].index));
			if (loc != from.end()) {
				for (auto j = to.begin(); j != to.end(); j++) {
					if (j->type == place::type) {
						tokens.push_back(tokens[i]);
						tokens.back().index = j->index;
					}
				}	
				tokens.erase(tokens.begin()+i);
			}
		}
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

template <typename state>
struct region {
	region() {
	}

	region(vector<state> states) : states(states) {
	}

	~region() {
	}

	vector<state> states;

	state &operator[](int i) {
		return states[i];
	}

	state operator[](int i) const {
		return states[i];
	}

	bool empty() const {
		return states.empty();
	}

	size_t size() const {
		return states.size();
	}

	vector<state>::iterator begin() {
		return states.begin();
	}

	vector<state>::iterator end() {
		return states.end();
	}

	vector<state>::const_iterator begin() const {
		return states.begin();
	}

	vector<state>::const_iterator end() const {
		return states.end();
	}

	void push_back(state s) {
		states.push_back(s);
	}

	void insert(vector<state>::iterator at, state s) {
		states.insert(at, s);
	}

	void append(region<state> r0) {
		states.insert(states.end(), r0.begin(), r0.end());
	}

	bool erase(petri::iterator i) {
		if (i.type != place::type) {
			return false;
		}

		bool found = false;
		for (auto s = states.begin(); s != states.end(); s++) {
			for (int j = (int)s->tokens.size()-1; j >= 0; j--) {
				if (s->tokens[j].index > i.index) {
					s->tokens[j].index--;
				} else if (s->tokens[j] == i.index) {
					found = true;
					s->tokens.erase(s->tokens.begin() + j);
				}
			}
		}
		return found;
	}

	bool erase(vector<petri::iterator> i, bool rsorted=false) {
		if (not rsorted) {
			sort(i.begin(), i.end());
			i.erase(unique(i.begin(), i.end()), i.end());
			reverse(i.begin(), i.end());
		}

		bool found = false;
		for (auto j = i.begin(); j != i.end(); j++) {
			if (erase(*j)) {
				found = true;
			}
		}
		return found;
	}

	void replace(vector<petri::iterator> from, vector<petri::iterator> to) {
		for (auto i = states.begin(); i != states.end(); i++) {
			i->replace(from, to);
		}
	}

	void merge(int composition, region<state> r1) {
		if (states.empty()) {
			states = r1.states;
		} else if (composition == choice) {
			for (auto i = r1.begin(); i != r1.end(); i++) {
				typename vector<state>::iterator j = lower_bound(states.begin(), states.end(), *i);
				if (j != states.end() and *j == *i) {
					*j = state::merge(*j, *i);
				} else {
					states.insert(j, *i);
				}
			}
		} else if (composition == parallel and not r1.states.empty()) {
			vector<state> r0 = states;
			states.clear();
			for (auto i = r0.begin(); i != r0.end(); i++) {
				for (auto j = r1.begin(); j != r1.end(); j++) {
					states.push_back(state::merge(*i, *j));
				}
			}
		}
	}
};

template <typename state>
struct bound {
	bound() {
	}

	bound(region<state> source, region<state> sink) : source(source), sink(sink) {
	}

	~bound() {
	}

	region<state> source;
	region<state> sink;

	void erase(vector<petri::iterator> i, bool rsorted=false) {
		if (not rsorted) {
			sort(i.begin(), i.end());
			i.erase(unique(i.begin(), i.end()), i.end());
			reverse(i.begin(), i.end());
		}

		source.erase(i, true);
		sink.erase(i, true);
	}
};

}
