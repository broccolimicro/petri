#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

using namespace petri;
using namespace std;

TEST(partial_composition, parallel_choice) {
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

	g.compute_split_groups();

	EXPECT_TRUE(g.is(choice, {p[1], p[5]}, {p[2], p[6]}));
	EXPECT_TRUE(g.is(choice, {p[1], p[5]}, {p[2], p[5]}));
	EXPECT_FALSE(g.is(parallel, {p[1], p[5]}, {p[2], p[6]}));
	EXPECT_FALSE(g.is(parallel, {p[1], p[5]}, {p[2], p[5]}));
	EXPECT_FALSE(g.is(sequence, {p[1], p[5]}, {p[2], p[6]}));
	EXPECT_FALSE(g.is(sequence, {p[1], p[5]}, {p[2], p[5]}));
	EXPECT_TRUE(g.is(sequence, {p[0], p[4]}, {p[3], p[7]}));
	EXPECT_FALSE(g.is(choice, {p[0], p[4]}, {p[3], p[7]}));
	EXPECT_FALSE(g.is(parallel, {p[0], p[4]}, {p[3], p[7]}));

	EXPECT_TRUE(g.is(excludes, {p[1], p[5]}, {p[2], p[6]}));
	EXPECT_TRUE(g.is(excludes, {p[1], p[5]}, {p[2], p[5]}));
	EXPECT_FALSE(g.is(implies, {p[1], p[5]}, {p[2], p[6]}));
	EXPECT_FALSE(g.is(implies, {p[1], p[5]}, {p[2], p[5]}));
	EXPECT_FALSE(g.is(excludes, {p[0], p[4]}, {p[3], p[7]}));
	EXPECT_TRUE(g.is(implies, {p[0], p[4]}, {p[3], p[7]}));
}

TEST(partial_composition, parallel_parallel) {
	//               ->p1-->t2-->p2-                .
	//              /               \               .
	//     ->p0-->t1                 >t4-->p5       .
	//    /         \               /        \      .
	//   /           ->p3-->t3-->p4-          \     .
	// t0                                      >t9  .
	//   \           ->p7-->t6-->p8-          /     .
	//    \         /               \        /      .
	//     ->p6-->t5                 >t8-->p11      .
	//              \               /               .
	//               ->p9-->t7-->p10                .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 12);
	auto t = g.create(transition(), 10);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[4], p[5], t[9]});
	g.connect({t[1], p[3], t[3], p[4], t[4]});
	g.connect({t[0], p[6], t[5], p[7], t[6], p[8], t[8], p[11], t[9]});
	g.connect({t[5], p[9], t[7], p[10], t[8]});

	g.compute_split_groups();

	EXPECT_TRUE (g.is(parallel, {t[2], t[3]}, {t[6], t[7]}, true, true));
	EXPECT_FALSE(g.is(choice,   {t[2], t[3]}, {t[6], t[7]}, true, true));
	EXPECT_FALSE(g.is(sequence, {t[2], t[3]}, {t[6], t[7]}, true, true));
	EXPECT_TRUE (g.is(parallel, {t[2], t[6]}, {t[3], t[7]}, true, true));
	EXPECT_FALSE(g.is(choice,   {t[2], t[6]}, {t[3], t[7]}, true, true));
	EXPECT_FALSE(g.is(sequence, {t[2], t[6]}, {t[3], t[7]}, true, true));
	EXPECT_TRUE (g.is(parallel, {t[2], t[6]}, {t[6], t[7]}, true, true));
	EXPECT_FALSE(g.is(choice,   {t[2], t[6]}, {t[6], t[7]}, true, true));
	EXPECT_FALSE(g.is(sequence, {t[2], t[6]}, {t[6], t[7]}, true, true));

	// TODO(edward.bingham) Need to be able to determine ordering before I can
	// answer these questions for cross orderings.
	// EXPECT_TRUE(g.is(choice, {p[1], p[4]}, {p[2], p[3]}));
	// EXPECT_FALSE(g.is(parallel, {p[1], p[4]}, {p[2], p[3]}));
	// EXPECT_FALSE(g.is(sequence, {p[1], p[4]}, {p[2], p[3]}));

	EXPECT_TRUE(g.is(sequence, {p[1], p[3]}, {p[2], p[4]}, true, true));
	EXPECT_FALSE(g.is(choice, {p[1], p[3]}, {p[2], p[4]}, true, true));
	EXPECT_FALSE(g.is(parallel, {p[1], p[3]}, {p[2], p[4]}, true, true));


	EXPECT_TRUE (g.is(implies, {t[2], t[3]}, {t[6], t[7]}, true, true));
	EXPECT_FALSE(g.is(excludes,   {t[2], t[3]}, {t[6], t[7]}, true, true));
	EXPECT_TRUE (g.is(implies, {t[2], t[6]}, {t[3], t[7]}, true, true));
	EXPECT_FALSE(g.is(excludes,   {t[2], t[6]}, {t[3], t[7]}, true, true));
	EXPECT_TRUE (g.is(implies, {t[2], t[6]}, {t[6], t[7]}, true, true));
	EXPECT_FALSE(g.is(excludes,   {t[2], t[6]}, {t[6], t[7]}, true, true));

	// TODO(edward.bingham) Need to be able to determine ordering before I can
	// answer these questions for cross orderings.
	// EXPECT_TRUE(g.is(excludes, {p[1], p[4]}, {p[2], p[3]}));
	// EXPECT_FALSE(g.is(implies, {p[1], p[4]}, {p[2], p[3]}));

	EXPECT_FALSE(g.is(excludes, {p[1], p[3]}, {p[2], p[4]}, true, true));
	EXPECT_TRUE(g.is(implies, {p[1], p[3]}, {p[2], p[4]}, true, true));

}

