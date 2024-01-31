/*
 * simulator.h
 *
 *  Created on: Apr 28, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>
#include "graph.h"

namespace petri
{

template <class graph, class token, class state, class enabled_transition>
struct simulator
{
	simulator()
	{
		base = NULL;
	}

	simulator(const graph *base, state initial)
	{
		//cout << "Reset" << endl;
		this->base = base;
		if (base != NULL)
			for (int k = 0; k < (int)initial.tokens.size(); k++)
				tokens.push_back(initial.tokens[k]);
	}

	~simulator()
	{

	}

	const graph *base;

	vector<token> tokens;
	vector<enabled_transition> ready;

	/** enabled()
	 *
	 * Returns a vector of indices representing the transitions
	 * that this marking enabled and the term of each transition
	 * that's enabled.
	 */
	int enabled(bool sorted = false)
	{
		if (base == NULL)
		{
			internal("", "NULL pointer to simulator::base", __FILE__, __LINE__);
			return 0;
		}

		if (tokens.size() == 0)
			return 0;

		if (!sorted)
			sort(tokens.begin(), tokens.end());

		vector<enabled_transition> preload;
		vector<int> disabled;

		preload.reserve(tokens.size()*2);
		disabled.reserve(base->transitions.size());
		for (auto a = base->arcs[graph::place::type].begin(); a != base->arcs[graph::place::type].end(); a++) {
			// Check to see if we haven't already determined that this transition can't be enabled
			vector<int>::iterator d = lower_bound(disabled.begin(), disabled.end(), a->to.index);
			if (d == disabled.end() || *d != a->to.index)
			{
				// Find the index of this transition (if any) in the preload pool
				typename vector<enabled_transition>::iterator e = lower_bound(preload.begin(), preload.end(), enabled_transition(a->to.index));
				bool e_invalid = (e == preload.end() || e->index != a->to.index);

				// Check to see if there is any token at the input place of this arc and make sure that
				// this token has not already been consumed by this particular transition
				// Also since we only need one token per arc, we can stop once we've found a token
				bool found = false;
				for (int j = 0; j < (int)tokens.size() && !found; j++)
					if (a->from.index == tokens[j].index &&
						(e_invalid || find(e->tokens.begin(), e->tokens.end(), j) == e->tokens.end()))
					{
						// We are safe to add this to the list of possibly enabled transitions
						found = true;
						if (e_invalid)
							e = preload.insert(e, enabled_transition(a->to.index));

						e->tokens.push_back(j);
					}

				// If we didn't find a token at the input place, then we know that this transition can't
				// be enabled. So lets remove this from the list of possibly enabled transitions and
				// remember as much in the disabled list.
				if (!found)
				{
					disabled.insert(d, a->to.index);
					if (!e_invalid)
						preload.erase(e);
				}
			}
		}

		ready = preload;

		return ready.size();
	}

	int renabled(bool sorted = false)
	{
		if (base == NULL)
		{
			internal("", "NULL pointer to simulator::base", __FILE__, __LINE__);
			return 0;
		}

		if (!sorted)
			sort(tokens.begin(), tokens.end());

		vector<enabled_transition> result;
		vector<int> disabled;

		result.reserve(tokens.size()*2);
		disabled.reserve(base->transitions.size());
		for (vector<arc>::const_iterator a = base->arcs[graph::transition::type].begin(); a != base->arcs[graph::transition::type].end(); a++)
		{
			// Check to see if we haven't already determined that this transition can't be enabled
			vector<int>::iterator d = lower_bound(disabled.begin(), disabled.end(), a->from.index);
			if (d == disabled.end() || *d != a->from.index)
			{
				// Find the index of this transition (if any) in the result pool
				typename vector<enabled_transition>::iterator e = lower_bound(result.begin(), result.end(), enabled_transition(a->from.index));
				bool e_invalid = (e == result.end() || e->index != a->from.index);

				// Check to see if there is any token at the input place of this arc and make sure that
				// this token has not already been consumed by this particular transition
				// Also since we only need one token per arc, we can stop once we've found a token
				bool found = false;
				for (int j = 0; j < (int)tokens.size() && !found; j++)
					if (a->to.index == tokens[j].index &&
						(e_invalid || find(e->tokens.begin(), e->tokens.end(), j) == e->tokens.end()))
					{
						// We are safe to add this to the list of possibly enabled transitions
						found = true;
						if (e_invalid)
							e = result.insert(e, enabled_transition(a->from.index));

						e->tokens.push_back(j);
					}

				// If we didn't find a token at the input place, then we know that this transition can't
				// be enabled. So lets remove this from the list of possibly enabled transitions and
				// remember as much in the disabled list.
				if (!found)
				{
					disabled.insert(d, a->from.index);
					if (!e_invalid)
						result.erase(e);
				}
			}
		}

		ready = result;

		return ready.size();
	}

	enabled_transition fire(int index)
	{
		if (base == NULL)
		{
			internal("", "NULL pointer to simulator::base", __FILE__, __LINE__);
			return enabled_transition();
		}

		enabled_transition t = ready[index];

		// disable any transitions that were dependent on at least one of the same local tokens
		// This is only necessary to check for unstable transitions in the enabled() function
		for (int i = (int)ready.size()-1; i >= 0; i--)
			if (vector_intersection_size(ready[i].tokens, t.tokens) > 0)
				ready.erase(ready.begin() + i);

		// Update the tokens
		for (int i = t.tokens.size()-1; i >= 0; i--)
			tokens.erase(tokens.begin() + t.tokens[i]);

		vector<int> n = base->next(graph::transition::type, t.index);
		for (int i = 0; i < (int)n.size(); i++)
			tokens.push_back(token(n[i]));

		return t;
	}

	enabled_transition rfire(int index)
	{
		if (base == NULL)
		{
			internal("", "NULL pointer to simulator::base", __FILE__, __LINE__);
			return enabled_transition();
		}

		enabled_transition t = ready[index];

		// disable any transitions that were dependent on at least one of the same local tokens
		// This is only necessary to check for unstable transitions in the enabled() function
		for (int i = (int)ready.size()-1; i >= 0; i--)
			if (vector_intersection_size(ready[i].tokens, t.tokens) > 0)
				ready.erase(ready.begin() + i);

		// Update the tokens
		for (int i = t.tokens.size()-1; i >= 0; i--)
			tokens.erase(tokens.begin() + t.tokens[i]);

		vector<int> n = base->prev(graph::transition::type, t.index);
		for (int i = 0; i < (int)n.size(); i++)
			tokens.push_back(token(n[i]));

		return t;
	}

	state get_state()
	{
		state result;
		for (int i = 0; i < (int)tokens.size(); i++)
			if (tokens[i].cause < 0)
				result.tokens.push_back(tokens[i]);
		sort(result.tokens.begin(), result.tokens.end());
		return result;
	}

	state get_key()
	{
		state result;
		for (int i = 0; i < (int)ready.size(); i++)
			result.tokens.push_back(token(ready[i].index));
		sort(result.tokens.begin(), result.tokens.end());
		result.tokens.resize(unique(result.tokens.begin(), result.tokens.end()) - result.tokens.begin());
		return result;
	}
};

}
