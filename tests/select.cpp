#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

using namespace petri;
using namespace std;

TEST(select, sequence) {
	//  =-t0-->p0-->t1-->p1-->t2-->p2-=  .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 3);
	auto t = g.create(transition(), 4);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[0]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	vector<vector<petri::iterator> > g0;
	g0.resize(1);
	g0[0].push_back(p[0]);

	EXPECT_EQ(g0, g.select(parallel, {p[0]}, false, false));
	EXPECT_EQ(g0, g.select(parallel, {p[0]}, false, true));
	EXPECT_EQ(g0, g.select(parallel, {p[0]}, true, false));
	EXPECT_EQ(g0, g.select(parallel, {p[0]}, true, true));
	EXPECT_EQ(g0, g.select(choice, {p[0]}, false, false));
	EXPECT_EQ(g0, g.select(choice, {p[0]}, false, true));
	EXPECT_EQ(g0, g.select(choice, {p[0]}, true, false));
	EXPECT_EQ(g0, g.select(choice, {p[0]}, true, true));

	vector<vector<petri::iterator> > g1;
	g1.resize(2);
	g1[0].push_back(p[0]);
	g1[1].push_back(p[2]);

	vector<vector<petri::iterator> > g2;
	g2.resize(1);
	g2[0].push_back(p[0]);
	g2[0].push_back(p[2]);

	EXPECT_EQ(g1, g.select(parallel, {p[0], p[2]}, false, false));
	EXPECT_EQ(g2, g.select(parallel, {p[0], p[2]}, false, true));
	EXPECT_EQ(g1, g.select(parallel, {p[0], p[2]}, true, false));
	EXPECT_EQ(g2, g.select(parallel, {p[0], p[2]}, true, true));
	EXPECT_EQ(g1, g.select(choice, {p[0], p[2]}, false, false));
	EXPECT_EQ(g2, g.select(choice, {p[0], p[2]}, false, true));
	EXPECT_EQ(g1, g.select(choice, {p[0], p[2]}, true, false));
	EXPECT_EQ(g2, g.select(choice, {p[0], p[2]}, true, true));
}

TEST(select, always_choice) {
	//          ->t0-->p1-->t1-           .
	//         /               \          .
	//  t5-->p0                 >p3-->t4  .
	//         \               /          .
	//          ->t2-->p2-->t3-           .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 6);

	g.connect({t[5], p[0], t[0], p[1], t[1], p[3], t[4]});
	g.connect({p[0], t[2], p[2], t[3], p[3]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	vector<vector<petri::iterator> > g1;
	g1.resize(2);
	g1[0].push_back(p[1]);
	g1[1].push_back(p[2]);

	vector<vector<petri::iterator> > g2;
	g2.resize(1);
	g2[0].push_back(p[1]);
	g2[0].push_back(p[2]);

	EXPECT_EQ(g1, g.select(parallel, {p[1], p[2]}, false, false));
	EXPECT_EQ(g1, g.select(parallel, {p[1], p[2]}, false, true));
	EXPECT_EQ(g1, g.select(parallel, {p[1], p[2]}, true, false));
	EXPECT_EQ(g1, g.select(parallel, {p[1], p[2]}, true, true));
	EXPECT_EQ(g2, g.select(choice, {p[1], p[2]}, false, false));
	EXPECT_EQ(g2, g.select(choice, {p[1], p[2]}, false, true));
	EXPECT_EQ(g2, g.select(choice, {p[1], p[2]}, true, false));
	EXPECT_EQ(g2, g.select(choice, {p[1], p[2]}, true, true));

	g1.clear();
	g1.resize(2);
	g1[0].push_back(p[1]);
	g1[1].push_back(t[5]);

	g2.clear();
	g2.resize(1);
	g2[0].push_back(p[1]);
	g2[0].push_back(t[5]);

	EXPECT_EQ(g1, g.select(parallel, {p[1], t[5]}, false, false));
	EXPECT_EQ(g1, g.select(parallel, {p[1], t[5]}, false, true));
	EXPECT_EQ(g1, g.select(parallel, {p[1], t[5]}, true, false));
	EXPECT_EQ(g2, g.select(parallel, {p[1], t[5]}, true, true));
	EXPECT_EQ(g2, g.select(choice, {p[1], t[5]}, false, false));
	EXPECT_EQ(g2, g.select(choice, {p[1], t[5]}, false, true));
	EXPECT_EQ(g1, g.select(choice, {p[1], t[5]}, true, false));
	EXPECT_EQ(g2, g.select(choice, {p[1], t[5]}, true, true));

	g1.clear();
	g1.resize(2);
	g1[0].push_back(p[1]);
	g1[1].push_back(t[4]);

	g2.clear();
	g2.resize(1);
	g2[0].push_back(p[1]);
	g2[0].push_back(t[4]);

	EXPECT_EQ(g1, g.select(parallel, {p[1], t[4]}, false, false));
	EXPECT_EQ(g1, g.select(parallel, {p[1], t[4]}, false, true));
	EXPECT_EQ(g1, g.select(parallel, {p[1], t[4]}, true, false));
	EXPECT_EQ(g2, g.select(parallel, {p[1], t[4]}, true, true));
	EXPECT_EQ(g2, g.select(choice, {p[1], t[4]}, false, false));
	EXPECT_EQ(g2, g.select(choice, {p[1], t[4]}, false, true));
	EXPECT_EQ(g1, g.select(choice, {p[1], t[4]}, true, false));
	EXPECT_EQ(g2, g.select(choice, {p[1], t[4]}, true, true));
}

/*TEST(select, always_parallel) {
	//          ->p0-->t1-->p1-           .
	//         /               \          .
	//  p5-->t0                 >t3-->p4  .
	//         \               /          .
	//          ->p2-->t2-->p3-           .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 6);
	auto t = g.create(transition(), 4);

	g.connect({p[5], t[0], p[0], t[1], p[1], t[3], p[4]});
	g.connect({t[0], p[2], t[2], p[3], t[3]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_always(g, parallel, {p[0], t[1], p[1]}, {p[2], t[2], p[3]}, true);
	test_always(g, sequence, {p[5], t[0], p[0], t[1], p[1], t[3], p[4]}, {}, true);
	test_always(g, sequence, {p[5], t[0], p[2], t[2], p[3], t[3], p[4]}, {}, true);
}

TEST(select, compressed_proper_nesting) {
	//  =->*p0-->t0-->p1-->t1-=  .
	//       \ /  \ /            .
	//        X    X             .
	//       / \  / \            .
	//  =->*p2-->t2-->p3-->t3-=  .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 4);

	g.connect({p[0], t[0], p[1], t[1], p[0]});
	g.connect({p[2], t[2], p[3], t[3], p[2]});

	g.connect(p[0], t[2]);
	g.connect(p[2], t[0]);

	g.connect(t[0], p[3]);
	g.connect(t[2], p[1]);

	g.reset.push_back(state<token>({token(p[0].index), token(p[2].index)}));

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_always(g, sequence, {p[0], p[1], t[1]}, {}, true);
	test_always(g, sequence, {p[2], p[3], t[3]}, {}, true);
	test_always(g, sequence, {t[0], t[2]}, {p[1], t[1], p[0], p[3], t[3], p[2]});
	test_sometimes(g, sequence, {p[1], t[1], p[0], p[3], t[3], p[2]}, {t[0], t[2]});
	test_sometimes(g, choice, {p[1], t[1], p[0], p[3], t[3], p[2]}, {t[0], t[2]});
	test_not(g, parallel, {p[1], t[1], p[0], p[3], t[3], p[2]}, {t[0], t[2]});
	test_always(g, parallel, {p[1], t[1], p[0]}, {p[3], t[3], p[2]}, true);
	test_always(g, choice, {t[0]}, {t[2]}, true);
}

TEST(select, choice_parallel) {
	//          -->p1-->t1-->p2           .
	//         /               \          .
	//     ->t0-->p3-->t2-->p4-->t3-      .
	//    /                         \     .
	//  p0                           >p6  .
	//    \                         /     .
	//     ->t4-->p5-->t5-----------      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 6);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[3]});
	g.connect({t[0], p[3], t[2], p[4], t[3], p[6]});
	g.connect({p[0], t[4], p[5], t[5], p[6]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_always(g, sequence, {t[0], p[1], t[1], p[2], t[3], p[6]});
	test_always(g, sequence, {t[3], p[2], t[1], p[1], t[0], p[0]});
	test_always(g, sequence, {t[0], p[3], t[2], p[4], t[3], p[6]});
	test_always(g, sequence, {t[3], p[4], t[2], p[3], t[0], p[0]});
	test_always(g, sequence, {t[4], p[5], t[5], p[6]});
	test_always(g, sequence, {t[5], p[5], t[4], p[0]});
	test_always(g, parallel, {p[1], t[1], p[2]}, {p[3], t[2], p[4]}, true);
	test_always(g, choice, {t[4], p[5], t[5]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3]}, true);

	test_always(g, sequence, {p[0], p[6]});
	test_always(g, sequence, {p[6], p[0]});
	
	test_sometimes(g, sequence, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});
	test_sometimes(g, choice, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});
	test_not(g, parallel, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});

	// TODO(edward.bingham) write separate tests for this, and add index priority
	// to understand not just sequencing, but order as well.

	// EXPECT_TRUE(g.is(sequence, {p[1], p[3]}, {t[1], t[2]}, true));
	// EXPECT_FALSE(g.is(parallel, {p[1], p[3]}, {t[1], t[2]}));
	// EXPECT_FALSE(g.is(choice, {p[1], p[3]}, {t[1], t[2]}, true));
	// EXPECT_TRUE(g.is(choice, {p[1], p[3]}, {t[1], t[2]}));

	// EXPECT_TRUE(g.is(sequence, {p[1], p[3]}, {p[2], p[4]}, true));
	// EXPECT_FALSE(g.is(parallel, {p[1], p[3]}, {p[2], p[4]}));
	// EXPECT_FALSE(g.is(choice, {p[1], p[3]}, {p[2], p[4]}, true));
	// EXPECT_TRUE(g.is(choice, {p[1], p[3]}, {p[2], p[4]}));

	// EXPECT_FALSE(g.is(sequence, {p[1], p[4]}, {p[2], p[3]}));
	// EXPECT_TRUE(g.is(choice, {p[1], p[4]}, {p[2], p[3]}, true));
	// EXPECT_FALSE(g.is(parallel, {p[1], p[4]}, {p[2], p[3]}));
}

TEST(select, parallel_choice) {
	//          -->t1-->p1-->t2           .
	//         /               \          .
	//     ->p0-->t3-->p2-->t4-->p3-      .
	//    /                         \     .
	//  t0                           >t6  .
	//    \                         /     .
	//     ->p4-->t5-->p5-----------      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 6);
	auto t = g.create(transition(), 7);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[3]});
	g.connect({p[0], t[3], p[2], t[4], p[3], t[6]});
	g.connect({t[0], p[4], t[5], p[5], t[6]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_always(g, sequence, {t[1], p[1], t[2], p[3], t[6]});
	test_always(g, sequence, {t[3], p[2], t[4], p[3], t[6]});
	test_always(g, sequence, {t[0], p[4], t[5], p[5], t[6]});
	test_always(g, choice, {t[1], p[1], t[2]}, {t[3], p[2], t[4]}, true);
	test_always(g, parallel, {p[0], t[1], p[1], t[2], t[3], p[2], t[4], p[3]}, {p[4], t[5], p[5]});
	test_always(g, parallel, {p[4], t[5], p[5]}, {p[0], p[3]});
	test_sometimes(g, parallel, {p[4], t[5], p[5]}, {t[1], p[1], t[2], t[3], p[2], t[4]});
	test_sometimes(g, choice, {p[4], t[5], p[5]}, {t[1], p[1], t[2], t[3], p[2], t[4]});
	test_not(g, sequence, {p[4], t[5], p[5]}, {t[1], p[1], t[2], t[3], p[2], t[4]});

	test_always(g, sequence, {t[2], p[1], t[1], p[0], t[0]});
	test_always(g, sequence, {t[4], p[2], t[3], p[0], t[0]});
	test_always(g, sequence, {t[6], p[5], t[5], p[4], t[0]});
	test_always(g, sequence, {t[0], p[0], p[3], t[6]});
	test_always(g, sequence, {t[6], p[3], p[0], t[0]});

	test_sometimes(g, sequence, {p[0], p[3]}, {t[1], p[1], t[2], t[3], p[2], t[4]});
	test_sometimes(g, choice, {p[0], p[3]}, {t[1], p[1], t[2], t[3], p[2], t[4]});
	test_not(g, parallel, {p[0], p[3]}, {t[1], p[1], t[2], t[3], p[2], t[4]});
}

TEST(select, nonproper_choice) {
	//     ->t0-->p1-->t1-->p2-->t2-      .
	//    /         \               \     .
	//  p0           ->t6-           >p5  .
	//    \               \         /     .
	//     ->t3-->p3-->t4-->p4-->t5-      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 6);
	auto t = g.create(transition(), 7);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[2], p[5]});
	g.connect({p[0], t[3], p[3], t[4], p[4], t[5], p[5]});

	g.connect(p[1], t[6]);
	g.connect(t[6], p[4]);

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_always(g, choice, {t[0], p[1], t[1], p[2], t[2]}, {t[3], p[3], t[4]}, true);
	test_always(g, choice, {t[1], p[2], t[2]}, {t[6], p[4], t[5]}, true);
	test_always(g, sequence, {t[0], p[1], p[5]});
	test_always(g, sequence, {t[1], p[2], t[2], p[5]});
	test_always(g, sequence, {t[6], p[4], t[5], p[5]});

	test_always(g, sequence, {t[2], p[2], t[1], p[1], t[0], p[0]});
	test_always(g, sequence, {t[5], p[4], p[0]});
	test_always(g, sequence, {t[6], p[1], t[0], p[0]});
	test_always(g, sequence, {t[4], p[3], t[3], p[0]});

	// TODO(edward.bingham) It should be this rather than what is listed below
	// test_always(g, sequence, {t[3], p[3], t[4], p[4], t[5], p[5]});
		test_always(g, sequence, {t[3], p[3], t[4], p[5]});
		test_always(g, sequence, {p[4], t[5], p[5]});
		test_sometimes(g, sequence, {t[3], p[3], t[4]}, {p[4], t[5]});

	// TODO(edward.bingham) This test fails as well
	//test_sometimes(g, choice, {t[0], p[1]}, {p[4], t[5]}, true);
		test_sometimes(g, choice, {t[0], p[1]}, {p[4], t[5]});

	test_sometimes(g, sequence, {t[0], p[1]}, {p[4], t[5]}, true);
	test_not(g, parallel, {t[0], p[1]}, {p[4], t[5]});
	

	test_always(g, choice, {t[6]}, {t[3], p[3], t[4], t[1], p[2], t[2]});
	test_sometimes(g, sequence, {t[0], p[1], p[4], t[5]}, {t[6]});

	// TODO(edward.bingham) This test fails as well
	//test_sometimes(g, choice, {t[0], p[1], p[4], t[5]}, {t[6]});
		test_sometimes(g, choice, {t[0], p[1]}, {t[6]});
	
	test_not(g, parallel, {t[0], p[1], p[4], t[5]}, {t[6]});
}

TEST(select, nonproper_parallel) {
	//     ->p0-->t1-->p1-->t2-->p2-      .
	//    /         \               \     .
	//  t0           ->p6-           >t5  .
	//    \               \         /     .
	//     ->p3-->t3-->p4-->t4-->p5-      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 6);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[5]});
	g.connect({t[0], p[3], t[3], p[4], t[4], p[5], t[5]});

	g.connect(t[1], p[6]);
	g.connect(p[6], t[4]);

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_always(g, parallel, {p[0], t[1], p[1], t[2], p[2], p[6]}, {p[3], t[3], p[4]}, true);
	test_always(g, parallel, {p[1], t[2], p[2]}, {p[6], t[4], p[5]}, true);
	test_always(g, sequence, {t[0], p[0], t[1], p[1], t[2], p[2], t[5]});
	test_always(g, sequence, {t[0], p[3], t[3], p[4], t[4], p[5], t[5]});
	test_always(g, sequence, {t[0], p[0], t[1], p[6], t[4], p[5], t[5]});
}*/

