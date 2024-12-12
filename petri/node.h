#pragma once

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>

#include <array>

namespace petri
{

enum composition
{
	choice = 0,
	parallel = 1,
	sequence = 2
};

struct split_group
{
	split_group();
	split_group(int split, int branch, int count);
	~split_group();

	enum {
		INTERSECT = 0,
		UNION = 1,
		DIFFERENCE = 2,
		SUBSET = 3,
		SUBSET_EQUAL = 4
	};
	
	int split; // index of place/transition with split
	std::vector<int> branch; // index of transitions/places coming out of split
	int count; // total number of branches out of this split

	string to_string() const;
};

bool operator<(const split_group &g0, const split_group &g1);
bool operator==(const split_group &g0, const split_group &g1);

ostream &operator<<(ostream &os, const split_group &g0);

bool compare(int group_operation, int branch_operation, vector<split_group> g0, vector<split_group> g1);
vector<split_group> merge(int group_operation, int branch_operation, vector<split_group> g0, vector<split_group> g1);
void merge_inplace(int group_operation, int branch_operation, vector<split_group> &g0, const vector<split_group> &g1, set<int> exclude=set<int>());

struct place
{
	place();
	~place();
	
	static const int type = 0;

	// sorted, transition index of parallel split -> place index of parallel branch
	// index with place::type or transition::type
	array<vector<split_group>, 2> splits;
	array<vector<split_group>, 2> merges;

	// Use this to determine if an arc crosses reset
	int priority_index;

	static place merge(int composition, const place &p0, const place &p1);
};

struct transition
{
	transition();
	~transition();
	
	static const int type = 1;

	// sorted, transition index of parallel split -> place index of parallel branch
	// index with place::type or transition::type
	array<vector<split_group>, 2> splits;
	array<vector<split_group>, 2> merges;

	// Use this to determine if an arc crosses reset
	int priority_index;

	bool is_infeasible();
	bool is_vacuous();

	static transition merge(int composition, const transition &t0, const transition &t1);
	static bool mergeable(int composition, const transition &t0, const transition &t1);
};

}