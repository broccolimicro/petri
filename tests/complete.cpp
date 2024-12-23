#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

using namespace petri;
using namespace std;

// just to make the tests easier to write
constexpr auto mark = static_cast<vector<vector<petri::iterator> >(*)(initializer_list<initializer_list<petri::iterator> >)>(petri::iterator::mark);

TEST(complete, parallel_choice) {
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

	//EXPECT_EQ(mark({{p[1],t[7]},{p[5],t[3]},{p[1],p[5]}}), g.complete(parallel, mark({{p[1]},{p[5]},{p[1],p[5]}})));
}

/*TEST(complete, choice_parallel) {
	//          ->p1-->t1-->p2-           .
	//         /               \          .
	//     ->t0                 >t3       .
	//    /    \               /   \      .
	//   /      ->p3-->t2-->p4-     \     .
	// p0                            >p9  .
	//   \      ->p5-->t5-->p6-     /     .
	//    \    /               \   /      .
	//     ->t4                 >t7       .
	//         \               /          .
	//          ->p7-->t6-->p8-           .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 10);
	auto t = g.create(transition(), 8);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[3], p[9]});
	g.connect({t[0], p[3], t[2], p[4], t[3]});
	g.connect({p[0], t[4], p[5], t[5], p[6], t[7], p[9]});
	g.connect({t[4], p[7], t[6], p[8], t[7]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	EXPECT_EQ(mark({{t[1]},{t[5]}}), g.group(parallel, mark({{t[1]},{t[5]}}), false, false));
	EXPECT_EQ(mark({{t[1]},{t[5]}}), g.group(parallel, mark({{t[1]},{t[5]}}), false, true));
	EXPECT_EQ(mark({{t[1]},{t[5]}}), g.group(parallel, mark({{t[1]},{t[5]}}), true, false));
	EXPECT_EQ(mark({{t[1]},{t[5]}}), g.group(parallel, mark({{t[1]},{t[5]}}), true, true));
	EXPECT_EQ(mark({{t[1]},{t[5]},{t[1],t[5]}}), g.group(choice, mark({{t[1]},{t[5]}}), false, false));
	EXPECT_EQ(mark({{t[1]},{t[5]},{t[1],t[5]}}), g.group(choice, mark({{t[1]},{t[5]}}), false, true));
	EXPECT_EQ(mark({{t[1]},{t[5]},{t[1],t[5]}}), g.group(choice, mark({{t[1]},{t[5]}}), true, false));
	EXPECT_EQ(mark({{t[1]},{t[5]},{t[1],t[5]}}), g.group(choice, mark({{t[1]},{t[5]}}), true, true));

	EXPECT_EQ(mark({{t[1]},{t[2]},{t[1],t[2]}}), g.group(parallel, mark({{t[1]},{t[2]}}), false, false));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[1],t[2]}}), g.group(parallel, mark({{t[1]},{t[2]}}), false, true));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[1],t[2]}}), g.group(parallel, mark({{t[1]},{t[2]}}), true, false));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[1],t[2]}}), g.group(parallel, mark({{t[1]},{t[2]}}), true, true));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.group(choice, mark({{t[1]},{t[2]}}), false, false));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.group(choice, mark({{t[1]},{t[2]}}), false, true));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.group(choice, mark({{t[1]},{t[2]}}), true, false));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.group(choice, mark({{t[1]},{t[2]}}), true, true));
}*/
