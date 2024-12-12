#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

using namespace petri;
using namespace std;

TEST(composition, always_choice)
{
	//     ->t0-->p1-->t1-      .
	//    /               \     .
	//  p0                 >p3  .
	//    \               /     .
	//     ->t2-->p2-->t3-      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 4);

	vector<petri::iterator> b0({t[0], p[1], t[1]});
	vector<petri::iterator> b1({t[2], p[2], t[3]});

	g.connect(p[0], t[0]);
	g.connect(t[0], p[1]);
	g.connect(p[1], t[1]);
	g.connect(t[1], p[3]);
	g.connect(p[0], t[2]);
	g.connect(t[2], p[2]);
	g.connect(p[2], t[3]);
	g.connect(t[3], p[3]);

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	for (auto i = b0.begin(); i != b0.end(); i++) {
		for (auto j = b1.begin(); j != b1.end(); j++) {
			EXPECT_TRUE(g.is(choice, *i, *j, true));
			EXPECT_TRUE(g.is(choice, *j, *i, true));
			EXPECT_FALSE(g.is(parallel, *i, *j));
			EXPECT_FALSE(g.is(parallel, *j, *i));
			EXPECT_FALSE(g.is(sequence, *i, *j));
			EXPECT_FALSE(g.is(sequence, *j, *i));
		}
	}

	for (auto i = b0.begin(); i != b0.end(); i++) {
		for (auto j = b0.begin(); j != b0.end(); j++) {
			EXPECT_FALSE(g.is(choice, *i, *j));
			EXPECT_FALSE(g.is(choice, *j, *i));
			EXPECT_FALSE(g.is(parallel, *i, *j));
			EXPECT_FALSE(g.is(parallel, *j, *i));
			if (i == j) {
				EXPECT_FALSE(g.is(sequence, *i, *j));
				EXPECT_FALSE(g.is(sequence, *j, *i));
			} else { 
				EXPECT_TRUE(g.is(sequence, *i, *j, true));
				EXPECT_TRUE(g.is(sequence, *j, *i, true));
			}
		}
	}

	for (auto i = b1.begin(); i != b1.end(); i++) {
		for (auto j = b1.begin(); j != b1.end(); j++) {
			EXPECT_FALSE(g.is(choice, *i, *j));
			EXPECT_FALSE(g.is(choice, *j, *i));
			EXPECT_FALSE(g.is(parallel, *i, *j));
			EXPECT_FALSE(g.is(parallel, *j, *i));
			if (i != j) {
				EXPECT_TRUE(g.is(sequence, *i, *j, true));
				EXPECT_TRUE(g.is(sequence, *j, *i, true));
			 } else {
				EXPECT_FALSE(g.is(sequence, *i, *j));
				EXPECT_FALSE(g.is(sequence, *j, *i));
			}
		}
	}

	for (auto i = b0.begin(); i != b0.end(); i++) {
		EXPECT_FALSE(g.is(choice, *i, p[0]));
		EXPECT_FALSE(g.is(choice, p[0], *i));
		EXPECT_FALSE(g.is(choice, *i, p[3]));
		EXPECT_FALSE(g.is(choice, p[3], *i));
		EXPECT_FALSE(g.is(parallel, *i, p[0]));
		EXPECT_FALSE(g.is(parallel, p[0], *i));
		EXPECT_FALSE(g.is(parallel, *i, p[3]));
		EXPECT_FALSE(g.is(parallel, p[3], *i));
		EXPECT_TRUE(g.is(sequence, *i, p[3], true));
		EXPECT_TRUE(g.is(sequence, p[3], *i, true));
		EXPECT_TRUE(g.is(sequence, *i, p[0], true));
		EXPECT_TRUE(g.is(sequence, p[0], *i, true));
	}

	for (auto j = b1.begin(); j != b1.end(); j++) {
		EXPECT_FALSE(g.is(choice, *j, p[0]));
		EXPECT_FALSE(g.is(choice, p[0], *j));
		EXPECT_FALSE(g.is(choice, *j, p[3]));
		EXPECT_FALSE(g.is(choice, p[3], *j));
		EXPECT_FALSE(g.is(parallel, *j, p[0]));
		EXPECT_FALSE(g.is(parallel, p[0], *j));
		EXPECT_FALSE(g.is(parallel, *j, p[3]));
		EXPECT_FALSE(g.is(parallel, p[3], *j));
		EXPECT_TRUE(g.is(sequence, *j, p[3], true));
		EXPECT_TRUE(g.is(sequence, p[3], *j, true));
		EXPECT_TRUE(g.is(sequence, *j, p[0], true));
		EXPECT_TRUE(g.is(sequence, p[0], *j, true));
	}
}

TEST(composition, always_parallel)
{
	//     ->p0-->t1-->p1-      .
	//    /               \     .
	//  t0                 >t3  .
	//    \               /     .
	//     ->p2-->t2-->p3-      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 4);

	vector<petri::iterator> b0({p[0], t[1], p[1]});
	vector<petri::iterator> b1({p[2], t[2], p[3]});

	g.connect(t[0], p[0]);
	g.connect(p[0], t[1]);
	g.connect(t[1], p[1]);
	g.connect(p[1], t[3]);
	g.connect(t[0], p[2]);
	g.connect(p[2], t[2]);
	g.connect(t[2], p[3]);
	g.connect(p[3], t[3]);

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	for (auto i = b0.begin(); i != b0.end(); i++) {
		for (auto j = b1.begin(); j != b1.end(); j++) {
			EXPECT_TRUE(g.is(parallel, *i, *j, true));
			EXPECT_TRUE(g.is(parallel, *j, *i, true));
			EXPECT_FALSE(g.is(choice, *i, *j));
			EXPECT_FALSE(g.is(choice, *j, *i));
			EXPECT_FALSE(g.is(sequence, *i, *j));
			EXPECT_FALSE(g.is(sequence, *j, *i));
		}
	}

	for (auto i = b0.begin(); i != b0.end(); i++) {
		for (auto j = b0.begin(); j != b0.end(); j++) {
			EXPECT_FALSE(g.is(parallel, *i, *j));
			EXPECT_FALSE(g.is(parallel, *j, *i));
			EXPECT_FALSE(g.is(choice, *i, *j));
			EXPECT_FALSE(g.is(choice, *j, *i));
			if (i == j) {
				EXPECT_FALSE(g.is(sequence, *i, *j));
				EXPECT_FALSE(g.is(sequence, *j, *i));
			} else { 
				EXPECT_TRUE(g.is(sequence, *i, *j, true));
				EXPECT_TRUE(g.is(sequence, *j, *i, true));
			}
		}
	}

	for (auto i = b1.begin(); i != b1.end(); i++) {
		for (auto j = b1.begin(); j != b1.end(); j++) {
			EXPECT_FALSE(g.is(parallel, *i, *j));
			EXPECT_FALSE(g.is(parallel, *j, *i));
			EXPECT_FALSE(g.is(choice, *i, *j));
			EXPECT_FALSE(g.is(choice, *j, *i));
			if (i != j) {
				EXPECT_TRUE(g.is(sequence, *i, *j, true));
				EXPECT_TRUE(g.is(sequence, *j, *i, true));
			 } else {
				EXPECT_FALSE(g.is(sequence, *i, *j));
				EXPECT_FALSE(g.is(sequence, *j, *i));
			}
		}
	}

	for (auto i = b0.begin(); i != b0.end(); i++) {
		EXPECT_FALSE(g.is(parallel, *i, t[0]));
		EXPECT_FALSE(g.is(parallel, t[0], *i));
		EXPECT_FALSE(g.is(parallel, *i, t[3]));
		EXPECT_FALSE(g.is(parallel, t[3], *i));
		EXPECT_FALSE(g.is(choice, *i, t[0]));
		EXPECT_FALSE(g.is(choice, t[0], *i));
		EXPECT_FALSE(g.is(choice, *i, t[3]));
		EXPECT_FALSE(g.is(choice, t[3], *i));
		EXPECT_TRUE(g.is(sequence, *i, t[3], true));
		EXPECT_TRUE(g.is(sequence, t[3], *i, true));
		EXPECT_TRUE(g.is(sequence, *i, t[0], true));
		EXPECT_TRUE(g.is(sequence, t[0], *i, true));
	}

	for (auto j = b1.begin(); j != b1.end(); j++) {
		EXPECT_FALSE(g.is(parallel, *j, t[0]));
		EXPECT_FALSE(g.is(parallel, t[0], *j));
		EXPECT_FALSE(g.is(parallel, *j, t[3]));
		EXPECT_FALSE(g.is(parallel, t[3], *j));
		EXPECT_FALSE(g.is(choice, *j, t[0]));
		EXPECT_FALSE(g.is(choice, t[0], *j));
		EXPECT_FALSE(g.is(choice, *j, t[3]));
		EXPECT_FALSE(g.is(choice, t[3], *j));
		EXPECT_TRUE(g.is(sequence, *j, t[3], true));
		EXPECT_TRUE(g.is(sequence, t[3], *j, true));
		EXPECT_TRUE(g.is(sequence, *j, t[0], true));
		EXPECT_TRUE(g.is(sequence, t[0], *j, true));
	}
}

TEST(composition, compressed_proper_nesting)
{
	//  =->*p0-->t0-->p1-->t1-=  .
	//       \ /  \ /            .
	//        X    X             .
	//       / \  / \            .
	//  =->*p2-->t2-->p3-->t3-=  .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 4);

	vector<petri::iterator> b0({p[0], t[0], p[1], t[1]});
	vector<petri::iterator> b1({p[2], t[2], p[3], t[3]});

	g.connect(p[0], t[0]);
	g.connect(t[0], p[1]);
	g.connect(p[1], t[1]);
	g.connect(t[1], p[0]);

	g.connect(p[2], t[2]);
	g.connect(t[2], p[3]);
	g.connect(p[3], t[3]);
	g.connect(t[3], p[2]);

	g.connect(p[0], t[2]);
	g.connect(p[2], t[0]);

	g.connect(t[0], p[3]);
	g.connect(t[2], p[1]);

	g.reset.push_back(state<token>({token(p[0].index), token(p[2].index)}));

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	EXPECT_TRUE(g.is(parallel, p[0], p[2], true));
	EXPECT_TRUE(g.is(parallel, p[2], p[0], true));
	EXPECT_FALSE(g.is(choice, p[0], p[2]));
	EXPECT_FALSE(g.is(choice, p[2], p[0]));
	EXPECT_FALSE(g.is(sequence, p[0], p[2]));
	EXPECT_FALSE(g.is(sequence, p[2], p[0]));

	EXPECT_TRUE(g.is(sequence, p[0], t[2], true));
	EXPECT_TRUE(g.is(sequence, t[2], p[0], true));
	EXPECT_FALSE(g.is(choice, p[0], t[2]));
	EXPECT_FALSE(g.is(choice, t[2], p[0]));
	EXPECT_FALSE(g.is(parallel, p[0], t[2]));
	EXPECT_FALSE(g.is(parallel, t[2], p[0]));

	EXPECT_TRUE(g.is(parallel, p[0], p[3], true));
	EXPECT_TRUE(g.is(parallel, p[3], p[0], true));
	EXPECT_FALSE(g.is(sequence, p[0], p[3]));
	EXPECT_FALSE(g.is(sequence, p[3], p[0]));
	EXPECT_FALSE(g.is(choice, p[0], p[3]));
	EXPECT_FALSE(g.is(choice, p[3], p[0]));

	EXPECT_TRUE(g.is(parallel, p[0], t[3], true));
	EXPECT_TRUE(g.is(parallel, t[3], p[0], true));
	EXPECT_FALSE(g.is(sequence, p[0], t[3]));
	EXPECT_FALSE(g.is(sequence, t[3], p[0]));
	EXPECT_FALSE(g.is(choice, p[0], t[3]));
	EXPECT_FALSE(g.is(choice, t[3], p[0]));



	EXPECT_FALSE(g.is(choice, t[0], p[2]));
	EXPECT_FALSE(g.is(choice, p[2], t[0]));
	EXPECT_FALSE(g.is(parallel, t[0], p[2]));
	EXPECT_FALSE(g.is(parallel, p[2], t[0]));
	EXPECT_TRUE(g.is(sequence, t[0], p[2], true));
	EXPECT_TRUE(g.is(sequence, p[2], t[0], true));

	EXPECT_TRUE(g.is(choice, t[0], t[2], true));
	EXPECT_TRUE(g.is(choice, t[2], t[0], true));
	EXPECT_FALSE(g.is(parallel, t[0], t[2]));
	EXPECT_FALSE(g.is(parallel, t[2], t[0]));
	EXPECT_FALSE(g.is(sequence, t[0], t[2]));
	EXPECT_FALSE(g.is(sequence, t[2], t[0]));

	EXPECT_FALSE(g.is(choice, t[0], p[3]));
	EXPECT_FALSE(g.is(choice, p[3], t[0]));
	EXPECT_FALSE(g.is(parallel, t[0], p[3]));
	EXPECT_FALSE(g.is(parallel, p[3], t[0]));
	EXPECT_TRUE(g.is(sequence, t[0], p[3], true));
	EXPECT_TRUE(g.is(sequence, p[3], t[0], true));

	EXPECT_FALSE(g.is(choice, t[0], t[3]));
	EXPECT_FALSE(g.is(choice, t[3], t[0]));
	EXPECT_FALSE(g.is(parallel, t[0], t[3]));
	EXPECT_FALSE(g.is(parallel, t[3], t[0]));
	EXPECT_TRUE(g.is(sequence, t[0], t[3], true));
	EXPECT_TRUE(g.is(sequence, t[3], t[0], true));



	EXPECT_FALSE(g.is(choice, p[1], p[2]));
	EXPECT_FALSE(g.is(choice, p[2], p[1]));
	EXPECT_TRUE(g.is(parallel, p[1], p[2], true));
	EXPECT_TRUE(g.is(parallel, p[2], p[1], true));
	EXPECT_FALSE(g.is(sequence, p[1], p[2]));
	EXPECT_FALSE(g.is(sequence, p[2], p[1]));

	EXPECT_TRUE(g.is(sequence, p[1], t[2], true));
	EXPECT_TRUE(g.is(sequence, t[2], p[1], true));
	EXPECT_FALSE(g.is(parallel, p[1], t[2]));
	EXPECT_FALSE(g.is(parallel, t[2], p[1]));
	EXPECT_FALSE(g.is(choice, p[1], t[2]));
	EXPECT_FALSE(g.is(choice, t[2], p[1]));

	EXPECT_FALSE(g.is(choice, p[1], p[3]));
	EXPECT_FALSE(g.is(choice, p[3], p[1]));
	EXPECT_TRUE(g.is(parallel, p[1], p[3], true));
	EXPECT_TRUE(g.is(parallel, p[3], p[1], true));
	EXPECT_FALSE(g.is(sequence, p[1], p[3]));
	EXPECT_FALSE(g.is(sequence, p[3], p[1]));

	EXPECT_FALSE(g.is(choice, p[1], t[3]));
	EXPECT_FALSE(g.is(choice, t[3], p[1]));
	EXPECT_TRUE(g.is(parallel, p[1], t[3], true));
	EXPECT_TRUE(g.is(parallel, t[3], p[1], true));
	EXPECT_FALSE(g.is(sequence, p[1], t[3]));
	EXPECT_FALSE(g.is(sequence, t[3], p[1]));



	EXPECT_FALSE(g.is(choice, t[1], p[2]));
	EXPECT_FALSE(g.is(choice, p[2], t[1]));
	EXPECT_TRUE(g.is(parallel, t[1], p[2], true));
	EXPECT_TRUE(g.is(parallel, p[2], t[1], true));
	EXPECT_FALSE(g.is(sequence, t[1], p[2]));
	EXPECT_FALSE(g.is(sequence, p[2], t[1]));

	EXPECT_TRUE(g.is(sequence, t[1], t[2], true));
	EXPECT_TRUE(g.is(sequence, t[2], t[1], true));
	EXPECT_FALSE(g.is(parallel, t[1], t[2]));
	EXPECT_FALSE(g.is(parallel, t[2], t[1]));
	EXPECT_FALSE(g.is(choice, t[1], t[2]));
	EXPECT_FALSE(g.is(choice, t[2], t[1]));

	EXPECT_FALSE(g.is(choice, t[1], p[3]));
	EXPECT_FALSE(g.is(choice, p[3], t[1]));
	EXPECT_TRUE(g.is(parallel, t[1], p[3], true));
	EXPECT_TRUE(g.is(parallel, p[3], t[1], true));
	EXPECT_FALSE(g.is(sequence, t[1], p[3]));
	EXPECT_FALSE(g.is(sequence, p[3], t[1]));

	EXPECT_FALSE(g.is(choice, t[1], t[3]));
	EXPECT_FALSE(g.is(choice, t[3], t[1]));
	EXPECT_TRUE(g.is(parallel, t[1], t[3], true));
	EXPECT_TRUE(g.is(parallel, t[3], t[1], true));
	EXPECT_FALSE(g.is(sequence, t[1], t[3]));
	EXPECT_FALSE(g.is(sequence, t[3], t[1]));


	for (auto i = b0.begin(); i != b0.end(); i++) {
		for (auto j = b0.begin(); j != b0.end(); j++) {
			EXPECT_FALSE(g.is(parallel, *i, *j));
			EXPECT_FALSE(g.is(parallel, *j, *i));
			EXPECT_FALSE(g.is(choice, *i, *j));
			EXPECT_FALSE(g.is(choice, *j, *i));
			if (i == j) {
				EXPECT_FALSE(g.is(sequence, *i, *j));
				EXPECT_FALSE(g.is(sequence, *j, *i));
			} else { 
				EXPECT_TRUE(g.is(sequence, *i, *j, true));
				EXPECT_TRUE(g.is(sequence, *j, *i, true));
			}
		}
	}

	for (auto i = b1.begin(); i != b1.end(); i++) {
		for (auto j = b1.begin(); j != b1.end(); j++) {
			EXPECT_FALSE(g.is(parallel, *i, *j));
			EXPECT_FALSE(g.is(parallel, *j, *i));
			EXPECT_FALSE(g.is(choice, *i, *j));
			EXPECT_FALSE(g.is(choice, *j, *i));
			if (i != j) {
				EXPECT_TRUE(g.is(sequence, *i, *j, true));
				EXPECT_TRUE(g.is(sequence, *j, *i, true));
			 } else {
				EXPECT_FALSE(g.is(sequence, *i, *j));
				EXPECT_FALSE(g.is(sequence, *j, *i));
			}
		}
	}
}

