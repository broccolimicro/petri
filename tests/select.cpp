#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

#include <initializer_list>

using namespace petri;
using namespace std;

vector<vector<petri::iterator> > lst(initializer_list<initializer_list<petri::iterator> > n) {
	vector<vector<petri::iterator> > result;
	for (auto i = n.begin(); i != n.end(); i++) {
		result.push_back(vector<petri::iterator>(*i));
	}
	return result;
}

TEST(select, sequence) {
	//  =-t0-->p0-->t1-->p1-->t2-->p2-=  .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 3);
	auto t = g.create(transition(), 4);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[0]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	EXPECT_EQ(lst({{p[0]}}), g.select(parallel, {p[0]}, false, false));
	EXPECT_EQ(lst({{p[0]}}), g.select(parallel, {p[0]}, false, true));
	EXPECT_EQ(lst({{p[0]}}), g.select(parallel, {p[0]}, true, false));
	EXPECT_EQ(lst({{p[0]}}), g.select(parallel, {p[0]}, true, true));
	EXPECT_EQ(lst({{p[0]}}), g.select(choice, {p[0]}, false, false));
	EXPECT_EQ(lst({{p[0]}}), g.select(choice, {p[0]}, false, true));
	EXPECT_EQ(lst({{p[0]}}), g.select(choice, {p[0]}, true, false));
	EXPECT_EQ(lst({{p[0]}}), g.select(choice, {p[0]}, true, true));

	EXPECT_EQ(lst({{p[0]},{p[2]}}), g.select(parallel, {p[0], p[2]}, false, false));
	EXPECT_EQ(lst({{p[0],p[2]}}), g.select(parallel, {p[0], p[2]}, false, true));
	EXPECT_EQ(lst({{p[0]},{p[2]}}), g.select(parallel, {p[0], p[2]}, true, false));
	EXPECT_EQ(lst({{p[0],p[2]}}), g.select(parallel, {p[0], p[2]}, true, true));
	EXPECT_EQ(lst({{p[0]},{p[2]}}), g.select(choice, {p[0], p[2]}, false, false));
	EXPECT_EQ(lst({{p[0],p[2]}}), g.select(choice, {p[0], p[2]}, false, true));
	EXPECT_EQ(lst({{p[0]},{p[2]}}), g.select(choice, {p[0], p[2]}, true, false));
	EXPECT_EQ(lst({{p[0],p[2]}}), g.select(choice, {p[0], p[2]}, true, true));
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

	EXPECT_EQ(lst({{p[1]},{p[2]}}), g.select(parallel, {p[1], p[2]}, false, false));
	EXPECT_EQ(lst({{p[1]},{p[2]}}), g.select(parallel, {p[1], p[2]}, false, true));
	EXPECT_EQ(lst({{p[1]},{p[2]}}), g.select(parallel, {p[1], p[2]}, true, false));
	EXPECT_EQ(lst({{p[1]},{p[2]}}), g.select(parallel, {p[1], p[2]}, true, true));
	EXPECT_EQ(lst({{p[1],p[2]}}), g.select(choice, {p[1], p[2]}, false, false));
	EXPECT_EQ(lst({{p[1],p[2]}}), g.select(choice, {p[1], p[2]}, false, true));
	EXPECT_EQ(lst({{p[1],p[2]}}), g.select(choice, {p[1], p[2]}, true, false));
	EXPECT_EQ(lst({{p[1],p[2]}}), g.select(choice, {p[1], p[2]}, true, true));

	EXPECT_EQ(lst({{p[1]},{t[5]}}), g.select(parallel, {p[1], t[5]}, false, false));
	EXPECT_EQ(lst({{p[1]},{t[5]}}), g.select(parallel, {p[1], t[5]}, false, true));
	EXPECT_EQ(lst({{p[1]},{t[5]}}), g.select(parallel, {p[1], t[5]}, true, false));
	EXPECT_EQ(lst({{p[1],t[5]}}), g.select(parallel, {p[1], t[5]}, true, true));
	EXPECT_EQ(lst({{p[1],t[5]}}), g.select(choice, {p[1], t[5]}, false, false));
	EXPECT_EQ(lst({{p[1],t[5]}}), g.select(choice, {p[1], t[5]}, false, true));
	EXPECT_EQ(lst({{p[1]},{t[5]}}), g.select(choice, {p[1], t[5]}, true, false));
	EXPECT_EQ(lst({{p[1],t[5]}}), g.select(choice, {p[1], t[5]}, true, true));

	EXPECT_EQ(lst({{p[1]},{t[4]}}), g.select(parallel, {p[1], t[4]}, false, false));
	EXPECT_EQ(lst({{p[1]},{t[4]}}), g.select(parallel, {p[1], t[4]}, false, true));
	EXPECT_EQ(lst({{p[1]},{t[4]}}), g.select(parallel, {p[1], t[4]}, true, false));
	EXPECT_EQ(lst({{p[1],t[4]}}), g.select(parallel, {p[1], t[4]}, true, true));
	EXPECT_EQ(lst({{p[1],t[4]}}), g.select(choice, {p[1], t[4]}, false, false));
	EXPECT_EQ(lst({{p[1],t[4]}}), g.select(choice, {p[1], t[4]}, false, true));
	EXPECT_EQ(lst({{p[1]},{t[4]}}), g.select(choice, {p[1], t[4]}, true, false));
	EXPECT_EQ(lst({{p[1],t[4]}}), g.select(choice, {p[1], t[4]}, true, true));
}

TEST(select, always_parallel) {
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

	EXPECT_EQ(lst({{t[1],t[2]}}), g.select(parallel, {t[1], t[2]}, false, false));
	EXPECT_EQ(lst({{t[1],t[2]}}), g.select(parallel, {t[1], t[2]}, false, true));
	EXPECT_EQ(lst({{t[1],t[2]}}), g.select(parallel, {t[1], t[2]}, true, false));
	EXPECT_EQ(lst({{t[1],t[2]}}), g.select(parallel, {t[1], t[2]}, true, true));
	EXPECT_EQ(lst({{t[1]},{t[2]}}), g.select(choice, {t[1], t[2]}, false, false));
	EXPECT_EQ(lst({{t[1]},{t[2]}}), g.select(choice, {t[1], t[2]}, false, true));
	EXPECT_EQ(lst({{t[1]},{t[2]}}), g.select(choice, {t[1], t[2]}, true, false));
	EXPECT_EQ(lst({{t[1]},{t[2]}}), g.select(choice, {t[1], t[2]}, true, true));

	EXPECT_EQ(lst({{t[1]},{p[5]}}), g.select(parallel, {t[1], p[5]}, false, false));
	EXPECT_EQ(lst({{p[5],t[1]}}), g.select(parallel, {t[1], p[5]}, false, true));
	EXPECT_EQ(lst({{t[1]},{p[5]}}), g.select(parallel, {t[1], p[5]}, true, false));
	EXPECT_EQ(lst({{p[5],t[1]}}), g.select(parallel, {t[1], p[5]}, true, true));
	EXPECT_EQ(lst({{t[1]},{p[5]}}), g.select(choice, {t[1], p[5]}, false, false));
	EXPECT_EQ(lst({{p[5],t[1]}}), g.select(choice, {t[1], p[5]}, false, true));
	EXPECT_EQ(lst({{t[1]},{p[5]}}), g.select(choice, {t[1], p[5]}, true, false));
	EXPECT_EQ(lst({{p[5],t[1]}}), g.select(choice, {t[1], p[5]}, true, true));

	EXPECT_EQ(lst({{t[1]},{p[4]}}), g.select(parallel, {t[1], p[4]}, false, false));
	EXPECT_EQ(lst({{p[4],t[1]}}), g.select(parallel, {t[1], p[4]}, false, true));
	EXPECT_EQ(lst({{t[1]},{p[4]}}), g.select(parallel, {t[1], p[4]}, true, false));
	EXPECT_EQ(lst({{p[4],t[1]}}), g.select(parallel, {t[1], p[4]}, true, true));
	EXPECT_EQ(lst({{t[1]},{p[4]}}), g.select(choice, {t[1], p[4]}, false, false));
	EXPECT_EQ(lst({{p[4],t[1]}}), g.select(choice, {t[1], p[4]}, false, true));
	EXPECT_EQ(lst({{t[1]},{p[4]}}), g.select(choice, {t[1], p[4]}, true, false));
	EXPECT_EQ(lst({{p[4],t[1]}}), g.select(choice, {t[1], p[4]}, true, true));
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

	EXPECT_EQ(lst({{t[0]},{t[2]}}), g.select(parallel, {t[0], t[2]}, false, false));
	EXPECT_EQ(lst({{t[0]},{t[2]}}), g.select(parallel, {t[0], t[2]}, false, true));
	EXPECT_EQ(lst({{t[0]},{t[2]}}), g.select(parallel, {t[0], t[2]}, true, false));
	EXPECT_EQ(lst({{t[0]},{t[2]}}), g.select(parallel, {t[0], t[2]}, true, true));
	EXPECT_EQ(lst({{t[0],t[2]}}), g.select(choice, {t[0], t[2]}, false, false));
	EXPECT_EQ(lst({{t[0],t[2]}}), g.select(choice, {t[0], t[2]}, false, true));
	EXPECT_EQ(lst({{t[0],t[2]}}), g.select(choice, {t[0], t[2]}, true, false));
	EXPECT_EQ(lst({{t[0],t[2]}}), g.select(choice, {t[0], t[2]}, true, true));

	EXPECT_EQ(lst({{t[0]},{t[3]}}), g.select(parallel, {t[0], t[3]}, false, false));
	EXPECT_EQ(lst({{t[0]},{t[3]}}), g.select(parallel, {t[0], t[3]}, false, true));
	EXPECT_EQ(lst({{t[0]},{t[3]}}), g.select(parallel, {t[0], t[3]}, true, false));
	EXPECT_EQ(lst({{t[0],t[3]}}), g.select(parallel, {t[0], t[3]}, true, true));
	EXPECT_EQ(lst({{t[0],t[3]}}), g.select(choice, {t[0], t[3]}, false, false));
	EXPECT_EQ(lst({{t[0],t[3]}}), g.select(choice, {t[0], t[3]}, false, true));
	EXPECT_EQ(lst({{t[0]},{t[3]}}), g.select(choice, {t[0], t[3]}, true, false));
	EXPECT_EQ(lst({{t[0],t[3]}}), g.select(choice, {t[0], t[3]}, true, true));

	EXPECT_EQ(lst({{t[1],t[3]}}), g.select(parallel, {t[1], t[3]}, false, false));
	EXPECT_EQ(lst({{t[1],t[3]}}), g.select(parallel, {t[1], t[3]}, false, true));
	EXPECT_EQ(lst({{t[1],t[3]}}), g.select(parallel, {t[1], t[3]}, true, false));
	EXPECT_EQ(lst({{t[1],t[3]}}), g.select(parallel, {t[1], t[3]}, true, true));
	EXPECT_EQ(lst({{t[1]},{t[3]}}), g.select(choice, {t[1], t[3]}, false, false));
	EXPECT_EQ(lst({{t[1]},{t[3]}}), g.select(choice, {t[1], t[3]}, false, true));
	EXPECT_EQ(lst({{t[1]},{t[3]}}), g.select(choice, {t[1], t[3]}, true, false));
	EXPECT_EQ(lst({{t[1]},{t[3]}}), g.select(choice, {t[1], t[3]}, true, true));

	EXPECT_EQ(lst({{p[0]},{p[1]}}), g.select(parallel, {p[0], p[1]}, false, false));
	EXPECT_EQ(lst({{p[0],p[1]}}), g.select(parallel, {p[0], p[1]}, false, true));
	EXPECT_EQ(lst({{p[0]},{p[1]}}), g.select(parallel, {p[0], p[1]}, true, false));
	EXPECT_EQ(lst({{p[0],p[1]}}), g.select(parallel, {p[0], p[1]}, true, true));
	EXPECT_EQ(lst({{p[0]},{p[1]}}), g.select(choice, {p[0], p[1]}, false, false));
	EXPECT_EQ(lst({{p[0],p[1]}}), g.select(choice, {p[0], p[1]}, false, true));
	EXPECT_EQ(lst({{p[0]},{p[1]}}), g.select(choice, {p[0], p[1]}, true, false));
	EXPECT_EQ(lst({{p[0],p[1]}}), g.select(choice, {p[0], p[1]}, true, true));
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

	EXPECT_EQ(lst({{t[1],t[2]},{t[5]}}), g.select(parallel, {t[1], t[2], t[5]}, false, false));
	EXPECT_EQ(lst({{t[1],t[2]},{t[5]}}), g.select(parallel, {t[1], t[2], t[5]}, false, true));
	EXPECT_EQ(lst({{t[1],t[2]},{t[5]}}), g.select(parallel, {t[1], t[2], t[5]}, true, false));
	EXPECT_EQ(lst({{t[1],t[2]},{t[5]}}), g.select(parallel, {t[1], t[2], t[5]}, true, true));
	EXPECT_EQ(lst({{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[5]}, false, false));
	EXPECT_EQ(lst({{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[5]}, false, true));
	EXPECT_EQ(lst({{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[5]}, true, false));
	EXPECT_EQ(lst({{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[5]}, true, true));

	EXPECT_EQ(lst({{t[1],t[2]},{t[0]}}), g.select(parallel, {t[1], t[2], t[0]}, false, false));
	EXPECT_EQ(lst({{t[0],t[1],t[2]}}), g.select(parallel, {t[1], t[2], t[0]}, false, true));
	EXPECT_EQ(lst({{t[1],t[2]},{t[0]}}), g.select(parallel, {t[1], t[2], t[0]}, true, false));
	EXPECT_EQ(lst({{t[0],t[1],t[2]}}), g.select(parallel, {t[1], t[2], t[0]}, true, true));
	EXPECT_EQ(lst({{t[1]},{t[2]},{t[0]}}), g.select(choice, {t[1], t[2], t[0]}, false, false));
	EXPECT_EQ(lst({{t[0],t[1]},{t[0],t[2]}}), g.select(choice, {t[1], t[2], t[0]}, false, true));
	EXPECT_EQ(lst({{t[1]},{t[2]},{t[0]}}), g.select(choice, {t[1], t[2], t[0]}, true, false));
	EXPECT_EQ(lst({{t[0],t[1]},{t[0],t[2]}}), g.select(choice, {t[1], t[2], t[0]}, true, true));
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

	EXPECT_EQ(lst({{p[1],p[5]},{p[2],p[5]}}), g.select(parallel, {p[1], p[2], p[5]}, false, false));
	EXPECT_EQ(lst({{p[1]},{p[2]},{p[5]}}), g.select(parallel, {p[1], p[2], p[5]}, false, true));
	EXPECT_EQ(lst({{p[1]},{p[2]},{p[5]}}), g.select(parallel, {p[1], p[2], p[5]}, true, false));
	EXPECT_EQ(lst({{p[1],p[5]},{p[2],p[5]}}), g.select(parallel, {p[1], p[2], p[5]}, true, true));
	EXPECT_EQ(lst({{p[1],p[2],p[5]}}), g.select(choice, {p[1], p[2], p[5]}, false, false));
	EXPECT_EQ(lst({{p[1],p[2]},{p[5]}}), g.select(choice, {p[1], p[2], p[5]}, false, true));
	EXPECT_EQ(lst({{p[1],p[2]},{p[5]}}), g.select(choice, {p[1], p[2], p[5]}, true, false));
	EXPECT_EQ(lst({{p[1],p[2],p[5]}}), g.select(choice, {p[1], p[2], p[5]}, true, true));
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

	EXPECT_EQ(lst({{t[1]},{t[6]},{t[4]}}), g.select(parallel, {t[1], t[6], t[4]}, false, false));
	EXPECT_EQ(lst({{t[1]},{t[6]},{t[4]}}), g.select(parallel, {t[1], t[6], t[4]}, false, true));
	EXPECT_EQ(lst({{t[1]},{t[6]},{t[4]}}), g.select(parallel, {t[1], t[6], t[4]}, true, false));
	EXPECT_EQ(lst({{t[1]},{t[6]},{t[4]}}), g.select(parallel, {t[1], t[6], t[4]}, true, true));
	EXPECT_EQ(lst({{t[1],t[4],t[6]}}), g.select(choice, {t[1], t[6], t[4]}, false, false));
	EXPECT_EQ(lst({{t[1],t[4],t[6]}}), g.select(choice, {t[1], t[6], t[4]}, false, true));
	EXPECT_EQ(lst({{t[1],t[4],t[6]}}), g.select(choice, {t[1], t[6], t[4]}, true, false));
	EXPECT_EQ(lst({{t[1],t[4],t[6]}}), g.select(choice, {t[1], t[6], t[4]}, true, true));

	EXPECT_EQ(lst({{t[0]},{t[2]},{t[3]},{t[5]}}), g.select(parallel, {t[0], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(lst({{t[0]},{t[2]},{t[3]},{t[5]}}), g.select(parallel, {t[0], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(lst({{t[0]},{t[2]},{t[3]},{t[5]}}), g.select(parallel, {t[0], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(lst({{t[0],t[2]},{t[0],t[5]},{t[3],t[5]}}), g.select(parallel, {t[0], t[2], t[3], t[5]}, true, true));
	EXPECT_EQ(lst({{t[0],t[2],t[3],t[5]}}), g.select(choice, {t[0], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(lst({{t[0],t[2],t[3],t[5]}}), g.select(choice, {t[0], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(lst({{t[0],t[3]},{t[2],t[3]},{t[2],t[5]}}), g.select(choice, {t[0], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(lst({{t[0],t[2],t[3],t[5]}}), g.select(choice, {t[0], t[2], t[3], t[5]}, true, true));
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

	EXPECT_EQ(lst({{p[1],p[4],p[6]}}), g.select(parallel, {p[1], p[6], p[4]}, false, false));
	EXPECT_EQ(lst({{p[1],p[4],p[6]}}), g.select(parallel, {p[1], p[6], p[4]}, false, true));
	EXPECT_EQ(lst({{p[1],p[4],p[6]}}), g.select(parallel, {p[1], p[6], p[4]}, true, false));
	EXPECT_EQ(lst({{p[1],p[4],p[6]}}), g.select(parallel, {p[1], p[6], p[4]}, true, true));
	EXPECT_EQ(lst({{p[1]},{p[6]},{p[4]}}), g.select(choice, {p[1], p[6], p[4]}, false, false));
	EXPECT_EQ(lst({{p[1]},{p[6]},{p[4]}}), g.select(choice, {p[1], p[6], p[4]}, false, true));
	EXPECT_EQ(lst({{p[1]},{p[6]},{p[4]}}), g.select(choice, {p[1], p[6], p[4]}, true, false));
	EXPECT_EQ(lst({{p[1]},{p[6]},{p[4]}}), g.select(choice, {p[1], p[6], p[4]}, true, true));

	EXPECT_EQ(lst({{p[0],p[3]},{p[2],p[3]},{p[2],p[5]}}), g.select(parallel, {p[0], p[2], p[3], p[5]}, false, false));
	EXPECT_EQ(lst({{p[0],p[2],p[3],p[5]}}), g.select(parallel, {p[0], p[2], p[3], p[5]}, false, true));
	EXPECT_EQ(lst({{p[0],p[3]},{p[2],p[3]},{p[2],p[5]}}), g.select(parallel, {p[0], p[2], p[3], p[5]}, true, false));
	EXPECT_EQ(lst({{p[0],p[2],p[3],p[5]}}), g.select(parallel, {p[0], p[2], p[3], p[5]}, true, true));
	EXPECT_EQ(lst({{p[0]},{p[2]},{p[3]},{p[5]}}), g.select(choice, {p[0], p[2], p[3], p[5]}, false, false));
	EXPECT_EQ(lst({{p[0],p[2]},{p[0],p[5]},{p[3],p[5]}}), g.select(choice, {p[0], p[2], p[3], p[5]}, false, true));
	EXPECT_EQ(lst({{p[0]},{p[2]},{p[3]},{p[5]}}), g.select(choice, {p[0], p[2], p[3], p[5]}, true, false));
	EXPECT_EQ(lst({{p[0],p[2]},{p[0],p[5]},{p[3],p[5]}}), g.select(choice, {p[0], p[2], p[3], p[5]}, true, true));
}

