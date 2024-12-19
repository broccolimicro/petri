#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

using namespace petri;
using namespace std;

TEST(group_composition, parallel_choice) {
	//          ->t1-->p1-->t2-           .
	//         /               \          .
	//     ->p0                 >p3       .
	//    /    \               /   \      .
	//   /      ->t3-->p2-->t4-     \     .
	// t0                            >t9  .
	//   \      ->t5-->p5-->t6-     /     .
	//    \    /               \   /      .
	//     ->p4                 >p7       .
	//         \               /          .
	//          ->t7-->p6-->t8-           .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 8);
	auto t = g.create(transition(), 10);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[3], t[9]});
	g.connect({p[0], t[3], p[2], t[4], p[3]});
	g.connect({t[0], p[4], t[5], p[5], t[6], p[7], t[9]});
	g.connect({p[4], t[7], p[6], t[8], p[7]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	EXPECT_TRUE(g.is(choice, {p[1], p[5]}, {p[2], p[6]}));
	EXPECT_TRUE(g.is(choice, {p[1], p[5]}, {p[2], p[5]}));
}

