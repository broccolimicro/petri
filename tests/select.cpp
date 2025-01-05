#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

#include <initializer_list>

using namespace petri;
using namespace std;

// just to make the tests easier to write
constexpr auto mark = static_cast<vector<vector<petri::iterator> >(*)(initializer_list<initializer_list<petri::iterator> >)>(petri::iterator::mark);

TEST(select, sequence) {
	//  =-t0-->p0-->t1-->p1-->t2-->p2-=  .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 3);
	auto t = g.create(transition(), 4);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[0]});

	g.compute_split_groups();

	EXPECT_EQ(mark({{p[0]}}), g.select(parallel, {p[0]}, false, false));
	EXPECT_EQ(mark({{p[0]}}), g.select(parallel, {p[0]}, false, true));
	EXPECT_EQ(mark({{p[0]}}), g.select(parallel, {p[0]}, true, false));
	EXPECT_EQ(mark({{p[0]}}), g.select(parallel, {p[0]}, true, true));
	EXPECT_EQ(mark({{p[0]}}), g.select(choice, {p[0]}, false, false));
	EXPECT_EQ(mark({{p[0]}}), g.select(choice, {p[0]}, false, true));
	EXPECT_EQ(mark({{p[0]}}), g.select(choice, {p[0]}, true, false));
	EXPECT_EQ(mark({{p[0]}}), g.select(choice, {p[0]}, true, true));

	EXPECT_EQ(mark({{p[0]},{p[2]}}), g.select(parallel, {p[0], p[2]}, false, false));
	EXPECT_EQ(mark({{p[0],p[2]}}), g.select(parallel, {p[0], p[2]}, false, true));
	EXPECT_EQ(mark({{p[0]},{p[2]}}), g.select(parallel, {p[0], p[2]}, true, false));
	EXPECT_EQ(mark({{p[0],p[2]}}), g.select(parallel, {p[0], p[2]}, true, true));
	EXPECT_EQ(mark({{p[0]},{p[2]}}), g.select(choice, {p[0], p[2]}, false, false));
	EXPECT_EQ(mark({{p[0],p[2]}}), g.select(choice, {p[0], p[2]}, false, true));
	EXPECT_EQ(mark({{p[0]},{p[2]}}), g.select(choice, {p[0], p[2]}, true, false));
	EXPECT_EQ(mark({{p[0],p[2]}}), g.select(choice, {p[0], p[2]}, true, true));

	EXPECT_EQ(mark({{p[0]}}), g.select(implies, {p[0]}, false, false));
	EXPECT_EQ(mark({{p[0]}}), g.select(implies, {p[0]}, false, true));
	EXPECT_EQ(mark({{p[0]}}), g.select(implies, {p[0]}, true, false));
	EXPECT_EQ(mark({{p[0]}}), g.select(implies, {p[0]}, true, true));
	EXPECT_EQ(mark({{p[0]}}), g.select(excludes, {p[0]}, false, false));
	EXPECT_EQ(mark({{p[0]}}), g.select(excludes, {p[0]}, false, true));
	EXPECT_EQ(mark({{p[0]}}), g.select(excludes, {p[0]}, true, false));
	EXPECT_EQ(mark({{p[0]}}), g.select(excludes, {p[0]}, true, true));

	EXPECT_EQ(mark({{p[0],p[2]}}), g.select(implies, {p[0], p[2]}, false, false));
	EXPECT_EQ(mark({{p[0],p[2]}}), g.select(implies, {p[0], p[2]}, false, true));
	EXPECT_EQ(mark({{p[0],p[2]}}), g.select(implies, {p[0], p[2]}, true, false));
	EXPECT_EQ(mark({{p[0],p[2]}}), g.select(implies, {p[0], p[2]}, true, true));
	EXPECT_EQ(mark({{p[0]},{p[2]}}), g.select(excludes, {p[0], p[2]}, false, false));
	EXPECT_EQ(mark({{p[0]},{p[2]}}), g.select(excludes, {p[0], p[2]}, false, true));
	EXPECT_EQ(mark({{p[0]},{p[2]}}), g.select(excludes, {p[0], p[2]}, true, false));
	EXPECT_EQ(mark({{p[0]},{p[2]}}), g.select(excludes, {p[0], p[2]}, true, true));
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

	g.compute_split_groups();

	EXPECT_EQ(mark({{p[1]},{p[2]}}), g.select(parallel, {p[1], p[2]}, false, false));
	EXPECT_EQ(mark({{p[1]},{p[2]}}), g.select(parallel, {p[1], p[2]}, false, true));
	EXPECT_EQ(mark({{p[1]},{p[2]}}), g.select(parallel, {p[1], p[2]}, true, false));
	EXPECT_EQ(mark({{p[1]},{p[2]}}), g.select(parallel, {p[1], p[2]}, true, true));
	EXPECT_EQ(mark({{p[1],p[2]}}), g.select(choice, {p[1], p[2]}, false, false));
	EXPECT_EQ(mark({{p[1],p[2]}}), g.select(choice, {p[1], p[2]}, false, true));
	EXPECT_EQ(mark({{p[1],p[2]}}), g.select(choice, {p[1], p[2]}, true, false));
	EXPECT_EQ(mark({{p[1],p[2]}}), g.select(choice, {p[1], p[2]}, true, true));

	EXPECT_EQ(mark({{p[1]},{t[5]}}), g.select(parallel, {p[1], t[5]}, false, false));
	EXPECT_EQ(mark({{p[1],t[5]}}), g.select(parallel, {p[1], t[5]}, false, true));
	EXPECT_EQ(mark({{p[1]},{t[5]}}), g.select(parallel, {p[1], t[5]}, true, false));
	EXPECT_EQ(mark({{p[1],t[5]}}), g.select(parallel, {p[1], t[5]}, true, true));
	EXPECT_EQ(mark({{p[1]},{t[5]}}), g.select(choice, {p[1], t[5]}, false, false));
	EXPECT_EQ(mark({{p[1],t[5]}}), g.select(choice, {p[1], t[5]}, false, true));
	EXPECT_EQ(mark({{p[1]},{t[5]}}), g.select(choice, {p[1], t[5]}, true, false));
	EXPECT_EQ(mark({{p[1],t[5]}}), g.select(choice, {p[1], t[5]}, true, true));

	EXPECT_EQ(mark({{p[1]},{t[4]}}), g.select(parallel, {p[1], t[4]}, false, false));
	EXPECT_EQ(mark({{p[1],t[4]}}), g.select(parallel, {p[1], t[4]}, false, true));
	EXPECT_EQ(mark({{p[1]},{t[4]}}), g.select(parallel, {p[1], t[4]}, true, false));
	EXPECT_EQ(mark({{p[1],t[4]}}), g.select(parallel, {p[1], t[4]}, true, true));
	EXPECT_EQ(mark({{p[1]},{t[4]}}), g.select(choice, {p[1], t[4]}, false, false));
	EXPECT_EQ(mark({{p[1],t[4]}}), g.select(choice, {p[1], t[4]}, false, true));
	EXPECT_EQ(mark({{p[1]},{t[4]}}), g.select(choice, {p[1], t[4]}, true, false));
	EXPECT_EQ(mark({{p[1],t[4]}}), g.select(choice, {p[1], t[4]}, true, true));


	EXPECT_EQ(mark({{p[1]},{p[2]}}), g.select(implies, {p[1], p[2]}, false, false));
	EXPECT_EQ(mark({{p[1]},{p[2]}}), g.select(implies, {p[1], p[2]}, false, true));
	EXPECT_EQ(mark({{p[1]},{p[2]}}), g.select(implies, {p[1], p[2]}, true, false));
	EXPECT_EQ(mark({{p[1]},{p[2]}}), g.select(implies, {p[1], p[2]}, true, true));
	EXPECT_EQ(mark({{p[1],p[2]}}), g.select(excludes, {p[1], p[2]}, false, false));
	EXPECT_EQ(mark({{p[1],p[2]}}), g.select(excludes, {p[1], p[2]}, false, true));
	EXPECT_EQ(mark({{p[1],p[2]}}), g.select(excludes, {p[1], p[2]}, true, false));
	EXPECT_EQ(mark({{p[1],p[2]}}), g.select(excludes, {p[1], p[2]}, true, true));

	EXPECT_EQ(mark({{p[1],t[5]}}), g.select(implies, {p[1], t[5]}, false, false));
	EXPECT_EQ(mark({{p[1]},{t[5]}}), g.select(implies, {p[1], t[5]}, false, true));
	EXPECT_EQ(mark({{p[1]},{t[5]}}), g.select(implies, {p[1], t[5]}, true, false));
	EXPECT_EQ(mark({{p[1],t[5]}}), g.select(implies, {p[1], t[5]}, true, true));
	EXPECT_EQ(mark({{p[1],t[5]}}), g.select(excludes, {p[1], t[5]}, false, false));
	EXPECT_EQ(mark({{p[1]},{t[5]}}), g.select(excludes, {p[1], t[5]}, false, true));
	EXPECT_EQ(mark({{p[1]},{t[5]}}), g.select(excludes, {p[1], t[5]}, true, false));
	EXPECT_EQ(mark({{p[1],t[5]}}), g.select(excludes, {p[1], t[5]}, true, true));

	EXPECT_EQ(mark({{p[1],t[4]}}), g.select(implies, {p[1], t[4]}, false, false));
	EXPECT_EQ(mark({{p[1]},{t[4]}}), g.select(implies, {p[1], t[4]}, false, true));
	EXPECT_EQ(mark({{p[1]},{t[4]}}), g.select(implies, {p[1], t[4]}, true, false));
	EXPECT_EQ(mark({{p[1],t[4]}}), g.select(implies, {p[1], t[4]}, true, true));
	EXPECT_EQ(mark({{p[1],t[4]}}), g.select(excludes, {p[1], t[4]}, false, false));
	EXPECT_EQ(mark({{p[1]},{t[4]}}), g.select(excludes, {p[1], t[4]}, false, true));
	EXPECT_EQ(mark({{p[1]},{t[4]}}), g.select(excludes, {p[1], t[4]}, true, false));
	EXPECT_EQ(mark({{p[1],t[4]}}), g.select(excludes, {p[1], t[4]}, true, true));
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

	g.compute_split_groups();

	EXPECT_EQ(mark({{t[1],t[2]}}), g.select(parallel, {t[1], t[2]}, false, false));
	EXPECT_EQ(mark({{t[1],t[2]}}), g.select(parallel, {t[1], t[2]}, false, true));
	EXPECT_EQ(mark({{t[1],t[2]}}), g.select(parallel, {t[1], t[2]}, true, false));
	EXPECT_EQ(mark({{t[1],t[2]}}), g.select(parallel, {t[1], t[2]}, true, true));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.select(choice, {t[1], t[2]}, false, false));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.select(choice, {t[1], t[2]}, false, true));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.select(choice, {t[1], t[2]}, true, false));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.select(choice, {t[1], t[2]}, true, true));

	EXPECT_EQ(mark({{t[1]},{p[5]}}), g.select(parallel, {t[1], p[5]}, false, false));
	EXPECT_EQ(mark({{p[5],t[1]}}), g.select(parallel, {t[1], p[5]}, false, true));
	EXPECT_EQ(mark({{t[1]},{p[5]}}), g.select(parallel, {t[1], p[5]}, true, false));
	EXPECT_EQ(mark({{p[5],t[1]}}), g.select(parallel, {t[1], p[5]}, true, true));
	EXPECT_EQ(mark({{t[1]},{p[5]}}), g.select(choice, {t[1], p[5]}, false, false));
	EXPECT_EQ(mark({{p[5],t[1]}}), g.select(choice, {t[1], p[5]}, false, true));
	EXPECT_EQ(mark({{t[1]},{p[5]}}), g.select(choice, {t[1], p[5]}, true, false));
	EXPECT_EQ(mark({{p[5],t[1]}}), g.select(choice, {t[1], p[5]}, true, true));

	EXPECT_EQ(mark({{t[1]},{p[4]}}), g.select(parallel, {t[1], p[4]}, false, false));
	EXPECT_EQ(mark({{p[4],t[1]}}), g.select(parallel, {t[1], p[4]}, false, true));
	EXPECT_EQ(mark({{t[1]},{p[4]}}), g.select(parallel, {t[1], p[4]}, true, false));
	EXPECT_EQ(mark({{p[4],t[1]}}), g.select(parallel, {t[1], p[4]}, true, true));
	EXPECT_EQ(mark({{t[1]},{p[4]}}), g.select(choice, {t[1], p[4]}, false, false));
	EXPECT_EQ(mark({{p[4],t[1]}}), g.select(choice, {t[1], p[4]}, false, true));
	EXPECT_EQ(mark({{t[1]},{p[4]}}), g.select(choice, {t[1], p[4]}, true, false));
	EXPECT_EQ(mark({{p[4],t[1]}}), g.select(choice, {t[1], p[4]}, true, true));

	EXPECT_EQ(mark({{t[1],t[2]}}), g.select(implies, {t[1], t[2]}, false, false));
	EXPECT_EQ(mark({{t[1],t[2]}}), g.select(implies, {t[1], t[2]}, false, true));
	EXPECT_EQ(mark({{t[1],t[2]}}), g.select(implies, {t[1], t[2]}, true, false));
	EXPECT_EQ(mark({{t[1],t[2]}}), g.select(implies, {t[1], t[2]}, true, true));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.select(excludes, {t[1], t[2]}, false, false));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.select(excludes, {t[1], t[2]}, false, true));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.select(excludes, {t[1], t[2]}, true, false));
	EXPECT_EQ(mark({{t[1]},{t[2]}}), g.select(excludes, {t[1], t[2]}, true, true));

	EXPECT_EQ(mark({{p[5],t[1]}}), g.select(implies, {t[1], p[5]}, false, false));
	EXPECT_EQ(mark({{p[5],t[1]}}), g.select(implies, {t[1], p[5]}, false, true));
	EXPECT_EQ(mark({{p[5],t[1]}}), g.select(implies, {t[1], p[5]}, true, false));
	EXPECT_EQ(mark({{p[5],t[1]}}), g.select(implies, {t[1], p[5]}, true, true));
	EXPECT_EQ(mark({{t[1]},{p[5]}}), g.select(excludes, {t[1], p[5]}, false, false));
	EXPECT_EQ(mark({{t[1]},{p[5]}}), g.select(excludes, {t[1], p[5]}, false, true));
	EXPECT_EQ(mark({{t[1]},{p[5]}}), g.select(excludes, {t[1], p[5]}, true, false));
	EXPECT_EQ(mark({{t[1]},{p[5]}}), g.select(excludes, {t[1], p[5]}, true, true));

	EXPECT_EQ(mark({{p[4],t[1]}}), g.select(implies, {t[1], p[4]}, false, false));
	EXPECT_EQ(mark({{p[4],t[1]}}), g.select(implies, {t[1], p[4]}, false, true));
	EXPECT_EQ(mark({{p[4],t[1]}}), g.select(implies, {t[1], p[4]}, true, false));
	EXPECT_EQ(mark({{p[4],t[1]}}), g.select(implies, {t[1], p[4]}, true, true));
	EXPECT_EQ(mark({{t[1]},{p[4]}}), g.select(excludes, {t[1], p[4]}, false, false));
	EXPECT_EQ(mark({{t[1]},{p[4]}}), g.select(excludes, {t[1], p[4]}, false, true));
	EXPECT_EQ(mark({{t[1]},{p[4]}}), g.select(excludes, {t[1], p[4]}, true, false));
	EXPECT_EQ(mark({{t[1]},{p[4]}}), g.select(excludes, {t[1], p[4]}, true, true));
}

TEST(select, regular_interleaved) {
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

	g.compute_split_groups();

	EXPECT_EQ(mark({{t[0]},{t[2]}}), g.select(parallel, {t[0], t[2]}, false, false));
	EXPECT_EQ(mark({{t[0]},{t[2]}}), g.select(parallel, {t[0], t[2]}, false, true));
	EXPECT_EQ(mark({{t[0]},{t[2]}}), g.select(parallel, {t[0], t[2]}, true, false));
	EXPECT_EQ(mark({{t[0]},{t[2]}}), g.select(parallel, {t[0], t[2]}, true, true));
	EXPECT_EQ(mark({{t[0],t[2]}}), g.select(choice, {t[0], t[2]}, false, false));
	EXPECT_EQ(mark({{t[0],t[2]}}), g.select(choice, {t[0], t[2]}, false, true));
	EXPECT_EQ(mark({{t[0],t[2]}}), g.select(choice, {t[0], t[2]}, true, false));
	EXPECT_EQ(mark({{t[0],t[2]}}), g.select(choice, {t[0], t[2]}, true, true));

	EXPECT_EQ(mark({{t[0]},{t[3]}}), g.select(parallel, {t[0], t[3]}, false, false));
	EXPECT_EQ(mark({{t[0],t[3]}}), g.select(parallel, {t[0], t[3]}, false, true));
	EXPECT_EQ(mark({{t[0]},{t[3]}}), g.select(parallel, {t[0], t[3]}, true, false));
	EXPECT_EQ(mark({{t[0],t[3]}}), g.select(parallel, {t[0], t[3]}, true, true));
	EXPECT_EQ(mark({{t[0]},{t[3]}}), g.select(choice, {t[0], t[3]}, false, false));
	EXPECT_EQ(mark({{t[0],t[3]}}), g.select(choice, {t[0], t[3]}, false, true));
	EXPECT_EQ(mark({{t[0]},{t[3]}}), g.select(choice, {t[0], t[3]}, true, false));
	EXPECT_EQ(mark({{t[0],t[3]}}), g.select(choice, {t[0], t[3]}, true, true));

	EXPECT_EQ(mark({{t[1],t[3]}}), g.select(parallel, {t[1], t[3]}, false, false));
	EXPECT_EQ(mark({{t[1],t[3]}}), g.select(parallel, {t[1], t[3]}, false, true));
	EXPECT_EQ(mark({{t[1],t[3]}}), g.select(parallel, {t[1], t[3]}, true, false));
	EXPECT_EQ(mark({{t[1],t[3]}}), g.select(parallel, {t[1], t[3]}, true, true));
	EXPECT_EQ(mark({{t[1]},{t[3]}}), g.select(choice, {t[1], t[3]}, false, false));
	EXPECT_EQ(mark({{t[1]},{t[3]}}), g.select(choice, {t[1], t[3]}, false, true));
	EXPECT_EQ(mark({{t[1]},{t[3]}}), g.select(choice, {t[1], t[3]}, true, false));
	EXPECT_EQ(mark({{t[1]},{t[3]}}), g.select(choice, {t[1], t[3]}, true, true));

	EXPECT_EQ(mark({{p[0]},{p[1]}}), g.select(parallel, {p[0], p[1]}, false, false));
	EXPECT_EQ(mark({{p[0],p[1]}}), g.select(parallel, {p[0], p[1]}, false, true));
	EXPECT_EQ(mark({{p[0]},{p[1]}}), g.select(parallel, {p[0], p[1]}, true, false));
	EXPECT_EQ(mark({{p[0],p[1]}}), g.select(parallel, {p[0], p[1]}, true, true));
	EXPECT_EQ(mark({{p[0]},{p[1]}}), g.select(choice, {p[0], p[1]}, false, false));
	EXPECT_EQ(mark({{p[0],p[1]}}), g.select(choice, {p[0], p[1]}, false, true));
	EXPECT_EQ(mark({{p[0]},{p[1]}}), g.select(choice, {p[0], p[1]}, true, false));
	EXPECT_EQ(mark({{p[0],p[1]}}), g.select(choice, {p[0], p[1]}, true, true));

	EXPECT_EQ(mark({{t[0]},{t[2]}}), g.select(implies, {t[0], t[2]}, false, false));
	EXPECT_EQ(mark({{t[0]},{t[2]}}), g.select(implies, {t[0], t[2]}, false, true));
	EXPECT_EQ(mark({{t[0]},{t[2]}}), g.select(implies, {t[0], t[2]}, true, false));
	EXPECT_EQ(mark({{t[0]},{t[2]}}), g.select(implies, {t[0], t[2]}, true, true));
	EXPECT_EQ(mark({{t[0],t[2]}}), g.select(excludes, {t[0], t[2]}, false, false));
	EXPECT_EQ(mark({{t[0],t[2]}}), g.select(excludes, {t[0], t[2]}, false, true));
	EXPECT_EQ(mark({{t[0],t[2]}}), g.select(excludes, {t[0], t[2]}, true, false));
	EXPECT_EQ(mark({{t[0],t[2]}}), g.select(excludes, {t[0], t[2]}, true, true));

	EXPECT_EQ(mark({{t[0],t[3]}}), g.select(implies, {t[0], t[3]}, false, false));
	EXPECT_EQ(mark({{t[0]},{t[3]}}), g.select(implies, {t[0], t[3]}, false, true));
	EXPECT_EQ(mark({{t[0]},{t[3]}}), g.select(implies, {t[0], t[3]}, true, false));
	EXPECT_EQ(mark({{t[0],t[3]}}), g.select(implies, {t[0], t[3]}, true, true));
	EXPECT_EQ(mark({{t[0],t[3]}}), g.select(excludes, {t[0], t[3]}, false, false));
	EXPECT_EQ(mark({{t[0]},{t[3]}}), g.select(excludes, {t[0], t[3]}, false, true));
	EXPECT_EQ(mark({{t[0]},{t[3]}}), g.select(excludes, {t[0], t[3]}, true, false));
	EXPECT_EQ(mark({{t[0],t[3]}}), g.select(excludes, {t[0], t[3]}, true, true));

	EXPECT_EQ(mark({{t[1],t[3]}}), g.select(implies, {t[1], t[3]}, false, false));
	EXPECT_EQ(mark({{t[1],t[3]}}), g.select(implies, {t[1], t[3]}, false, true));
	EXPECT_EQ(mark({{t[1],t[3]}}), g.select(implies, {t[1], t[3]}, true, false));
	EXPECT_EQ(mark({{t[1],t[3]}}), g.select(implies, {t[1], t[3]}, true, true));
	EXPECT_EQ(mark({{t[1]},{t[3]}}), g.select(excludes, {t[1], t[3]}, false, false));
	EXPECT_EQ(mark({{t[1]},{t[3]}}), g.select(excludes, {t[1], t[3]}, false, true));
	EXPECT_EQ(mark({{t[1]},{t[3]}}), g.select(excludes, {t[1], t[3]}, true, false));
	EXPECT_EQ(mark({{t[1]},{t[3]}}), g.select(excludes, {t[1], t[3]}, true, true));

	EXPECT_EQ(mark({{p[0],p[1]}}), g.select(implies, {p[0], p[1]}, false, false));
	EXPECT_EQ(mark({{p[0],p[1]}}), g.select(implies, {p[0], p[1]}, false, true));
	EXPECT_EQ(mark({{p[0],p[1]}}), g.select(implies, {p[0], p[1]}, true, false));
	EXPECT_EQ(mark({{p[0],p[1]}}), g.select(implies, {p[0], p[1]}, true, true));
	EXPECT_EQ(mark({{p[0]},{p[1]}}), g.select(excludes, {p[0], p[1]}, false, false));
	EXPECT_EQ(mark({{p[0]},{p[1]}}), g.select(excludes, {p[0], p[1]}, false, true));
	EXPECT_EQ(mark({{p[0]},{p[1]}}), g.select(excludes, {p[0], p[1]}, true, false));
	EXPECT_EQ(mark({{p[0]},{p[1]}}), g.select(excludes, {p[0], p[1]}, true, true));

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

	g.compute_split_groups();

	EXPECT_EQ(mark({{t[1],t[2]},{t[5]}}), g.select(parallel, {t[1], t[2], t[5]}, false, false));
	EXPECT_EQ(mark({{t[1],t[2]},{t[5]}}), g.select(parallel, {t[1], t[2], t[5]}, false, true));
	EXPECT_EQ(mark({{t[1],t[2]},{t[5]}}), g.select(parallel, {t[1], t[2], t[5]}, true, false));
	EXPECT_EQ(mark({{t[1],t[2]},{t[5]}}), g.select(parallel, {t[1], t[2], t[5]}, true, true));
	EXPECT_EQ(mark({{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[5]}, false, false));
	EXPECT_EQ(mark({{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[5]}, false, true));
	EXPECT_EQ(mark({{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[5]}, true, false));
	EXPECT_EQ(mark({{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[5]}, true, true));

	EXPECT_EQ(mark({{t[1],t[2]},{t[0]}}), g.select(parallel, {t[1], t[2], t[0]}, false, false));
	EXPECT_EQ(mark({{t[0],t[1],t[2]}}), g.select(parallel, {t[1], t[2], t[0]}, false, true));
	EXPECT_EQ(mark({{t[1],t[2]},{t[0]}}), g.select(parallel, {t[1], t[2], t[0]}, true, false));
	EXPECT_EQ(mark({{t[0],t[1],t[2]}}), g.select(parallel, {t[1], t[2], t[0]}, true, true));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[0]}}), g.select(choice, {t[1], t[2], t[0]}, false, false));
	EXPECT_EQ(mark({{t[0],t[1]},{t[0],t[2]}}), g.select(choice, {t[1], t[2], t[0]}, false, true));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[0]}}), g.select(choice, {t[1], t[2], t[0]}, true, false));
	EXPECT_EQ(mark({{t[0],t[1]},{t[0],t[2]}}), g.select(choice, {t[1], t[2], t[0]}, true, true));

	EXPECT_EQ(mark({{t[1],t[2]},{t[5]}}), g.select(implies, {t[1], t[2], t[5]}, false, false));
	EXPECT_EQ(mark({{t[1],t[2]},{t[5]}}), g.select(implies, {t[1], t[2], t[5]}, false, true));
	EXPECT_EQ(mark({{t[1],t[2]},{t[5]}}), g.select(implies, {t[1], t[2], t[5]}, true, false));
	EXPECT_EQ(mark({{t[1],t[2]},{t[5]}}), g.select(implies, {t[1], t[2], t[5]}, true, true));
	EXPECT_EQ(mark({{t[1],t[5]},{t[2],t[5]}}), g.select(excludes, {t[1], t[2], t[5]}, false, false));
	EXPECT_EQ(mark({{t[1],t[5]},{t[2],t[5]}}), g.select(excludes, {t[1], t[2], t[5]}, false, true));
	EXPECT_EQ(mark({{t[1],t[5]},{t[2],t[5]}}), g.select(excludes, {t[1], t[2], t[5]}, true, false));
	EXPECT_EQ(mark({{t[1],t[5]},{t[2],t[5]}}), g.select(excludes, {t[1], t[2], t[5]}, true, true));

	EXPECT_EQ(mark({{t[0],t[1],t[2]}}), g.select(implies, {t[1], t[2], t[0]}, false, false));
	EXPECT_EQ(mark({{t[0],t[1],t[2]}}), g.select(implies, {t[1], t[2], t[0]}, false, true));
	EXPECT_EQ(mark({{t[0],t[1],t[2]}}), g.select(implies, {t[1], t[2], t[0]}, true, false));
	EXPECT_EQ(mark({{t[0],t[1],t[2]}}), g.select(implies, {t[1], t[2], t[0]}, true, true));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[0]}}), g.select(excludes, {t[1], t[2], t[0]}, false, false));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[0]}}), g.select(excludes, {t[1], t[2], t[0]}, false, true));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[0]}}), g.select(excludes, {t[1], t[2], t[0]}, true, false));
	EXPECT_EQ(mark({{t[1]},{t[2]},{t[0]}}), g.select(excludes, {t[1], t[2], t[0]}, true, true));
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

	g.compute_split_groups();

	EXPECT_EQ(mark({{p[1],p[5]},{p[2],p[5]}}), g.select(parallel, {p[1], p[2], p[5]}, false, false));
	EXPECT_EQ(mark({{p[1],p[5]},{p[2],p[5]}}), g.select(parallel, {p[1], p[2], p[5]}, false, true));
	EXPECT_EQ(mark({{p[1],p[5]},{p[2],p[5]}}), g.select(parallel, {p[1], p[2], p[5]}, true, false));
	EXPECT_EQ(mark({{p[1],p[5]},{p[2],p[5]}}), g.select(parallel, {p[1], p[2], p[5]}, true, true));
	EXPECT_EQ(mark({{p[1],p[2]},{p[5]}}), g.select(choice, {p[1], p[2], p[5]}, false, false));
	EXPECT_EQ(mark({{p[1],p[2]},{p[5]}}), g.select(choice, {p[1], p[2], p[5]}, false, true));
	EXPECT_EQ(mark({{p[1],p[2]},{p[5]}}), g.select(choice, {p[1], p[2], p[5]}, true, false));
	EXPECT_EQ(mark({{p[1],p[2]},{p[5]}}), g.select(choice, {p[1], p[2], p[5]}, true, true));

	EXPECT_EQ(mark({{p[1]},{p[2]},{p[0]}}), g.select(parallel, {p[1], p[2], p[0]}, false, false));
	EXPECT_EQ(mark({{p[0],p[1]},{p[0],p[2]}}), g.select(parallel, {p[1], p[2], p[0]}, false, true));
	EXPECT_EQ(mark({{p[1]},{p[2]},{p[0]}}), g.select(parallel, {p[1], p[2], p[0]}, true, false));
	EXPECT_EQ(mark({{p[0],p[1]},{p[0],p[2]}}), g.select(parallel, {p[1], p[2], p[0]}, true, true));
	EXPECT_EQ(mark({{p[1],p[2]},{p[0]}}), g.select(choice, {p[1], p[2], p[0]}, false, false));
	EXPECT_EQ(mark({{p[0],p[1],p[2]}}), g.select(choice, {p[1], p[2], p[0]}, false, true));
	EXPECT_EQ(mark({{p[1],p[2]},{p[0]}}), g.select(choice, {p[1], p[2], p[0]}, true, false));
	EXPECT_EQ(mark({{p[0],p[1],p[2]}}), g.select(choice, {p[1], p[2], p[0]}, true, true));

	EXPECT_EQ(mark({{p[1],p[5]},{p[2],p[5]}}), g.select(implies, {p[1], p[2], p[5]}, false, false));
	EXPECT_EQ(mark({{p[1]},{p[2]},{p[5]}}), g.select(implies, {p[1], p[2], p[5]}, false, true));
	EXPECT_EQ(mark({{p[1]},{p[2]},{p[5]}}), g.select(implies, {p[1], p[2], p[5]}, true, false));
	EXPECT_EQ(mark({{p[1],p[5]},{p[2],p[5]}}), g.select(implies, {p[1], p[2], p[5]}, true, true));
	EXPECT_EQ(mark({{p[1],p[2],p[5]}}), g.select(excludes, {p[1], p[2], p[5]}, false, false));
	EXPECT_EQ(mark({{p[1],p[2]},{p[5]}}), g.select(excludes, {p[1], p[2], p[5]}, false, true));
	EXPECT_EQ(mark({{p[1],p[2]},{p[5]}}), g.select(excludes, {p[1], p[2], p[5]}, true, false));
	EXPECT_EQ(mark({{p[1],p[2],p[5]}}), g.select(excludes, {p[1], p[2], p[5]}, true, true));

	EXPECT_EQ(mark({{p[0],p[1]},{p[0],p[2]}}), g.select(implies, {p[1], p[2], p[0]}, false, false));
	EXPECT_EQ(mark({{p[1]},{p[2]},{p[0]}}), g.select(implies, {p[1], p[2], p[0]}, false, true));
	EXPECT_EQ(mark({{p[1]},{p[2]},{p[0]}}), g.select(implies, {p[1], p[2], p[0]}, true, false));
	EXPECT_EQ(mark({{p[0],p[1]},{p[0],p[2]}}), g.select(implies, {p[1], p[2], p[0]}, true, true));
	EXPECT_EQ(mark({{p[0],p[1],p[2]}}), g.select(excludes, {p[1], p[2], p[0]}, false, false));
	EXPECT_EQ(mark({{p[1],p[2]},{p[0]}}), g.select(excludes, {p[1], p[2], p[0]}, false, true));
	EXPECT_EQ(mark({{p[1],p[2]},{p[0]}}), g.select(excludes, {p[1], p[2], p[0]}, true, false));
	EXPECT_EQ(mark({{p[0],p[1],p[2]}}), g.select(excludes, {p[1], p[2], p[0]}, true, true));
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

	g.compute_split_groups();

	EXPECT_EQ(mark({{t[1]},{t[6]},{t[4]}}), g.select(parallel, {t[1], t[6], t[4]}, false, false));
	EXPECT_EQ(mark({{t[1]},{t[6]},{t[4]}}), g.select(parallel, {t[1], t[6], t[4]}, false, true));
	EXPECT_EQ(mark({{t[1]},{t[6]},{t[4]}}), g.select(parallel, {t[1], t[6], t[4]}, true, false));
	EXPECT_EQ(mark({{t[1]},{t[6]},{t[4]}}), g.select(parallel, {t[1], t[6], t[4]}, true, true));
	EXPECT_EQ(mark({{t[1],t[4],t[6]}}), g.select(choice, {t[1], t[6], t[4]}, false, false));
	EXPECT_EQ(mark({{t[1],t[4],t[6]}}), g.select(choice, {t[1], t[6], t[4]}, false, true));
	EXPECT_EQ(mark({{t[1],t[4],t[6]}}), g.select(choice, {t[1], t[6], t[4]}, true, false));
	EXPECT_EQ(mark({{t[1],t[4],t[6]}}), g.select(choice, {t[1], t[6], t[4]}, true, true));

	EXPECT_EQ(mark({{t[0]},{t[2]},{t[3]},{t[5]}}), g.select(parallel, {t[0], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(mark({{t[0],t[2]},{t[3]},{t[5]}}), g.select(parallel, {t[0], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(mark({{t[0]},{t[2]},{t[3]},{t[5]}}), g.select(parallel, {t[0], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(mark({{t[0],t[2]},{t[0],t[5]},{t[3],t[5]}}), g.select(parallel, {t[0], t[2], t[3], t[5]}, true, true));
	EXPECT_EQ(mark({{t[0],t[3],t[5]},{t[2],t[3],t[5]}}), g.select(choice, {t[0], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(mark({{t[0],t[2],t[3],t[5]}}), g.select(choice, {t[0], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(mark({{t[0],t[3]},{t[2],t[3]},{t[2],t[5]}}), g.select(choice, {t[0], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(mark({{t[0],t[2],t[3],t[5]}}), g.select(choice, {t[0], t[2], t[3], t[5]}, true, true));

	EXPECT_EQ(mark({{t[1]},{t[6]},{t[4]}}), g.select(implies, {t[1], t[6], t[4]}, false, false));
	EXPECT_EQ(mark({{t[1]},{t[6]},{t[4]}}), g.select(implies, {t[1], t[6], t[4]}, false, true));
	EXPECT_EQ(mark({{t[1]},{t[6]},{t[4]}}), g.select(implies, {t[1], t[6], t[4]}, true, false));
	EXPECT_EQ(mark({{t[1]},{t[6]},{t[4]}}), g.select(implies, {t[1], t[6], t[4]}, true, true));
	EXPECT_EQ(mark({{t[1],t[4],t[6]}}), g.select(excludes, {t[1], t[6], t[4]}, false, false));
	EXPECT_EQ(mark({{t[1],t[4],t[6]}}), g.select(excludes, {t[1], t[6], t[4]}, false, true));
	EXPECT_EQ(mark({{t[1],t[4],t[6]}}), g.select(excludes, {t[1], t[6], t[4]}, true, false));
	EXPECT_EQ(mark({{t[1],t[4],t[6]}}), g.select(excludes, {t[1], t[6], t[4]}, true, true));

	EXPECT_EQ(mark({{t[0],t[2]},{t[0],t[5]},{t[3],t[5]}}), g.select(implies, {t[0], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(mark({{t[0]},{t[2]},{t[3]},{t[5]}}), g.select(implies, {t[0], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(mark({{t[0]},{t[2]},{t[3]},{t[5]}}), g.select(implies, {t[0], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(mark({{t[0],t[2]},{t[0],t[5]},{t[3],t[5]}}), g.select(implies, {t[0], t[2], t[3], t[5]}, true, true));
	EXPECT_EQ(mark({{t[0],t[2],t[3],t[5]}}), g.select(excludes, {t[0], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(mark({{t[0],t[3]},{t[2],t[3]},{t[2],t[5]}}), g.select(excludes, {t[0], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(mark({{t[0],t[3]},{t[2],t[3]},{t[2],t[5]}}), g.select(excludes, {t[0], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(mark({{t[0],t[2],t[3],t[5]}}), g.select(excludes, {t[0], t[2], t[3], t[5]}, true, true));
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

	g.compute_split_groups();

	EXPECT_EQ(mark({{p[1],p[4],p[6]}}), g.select(parallel, {p[1], p[6], p[4]}, false, false));
	EXPECT_EQ(mark({{p[1],p[4],p[6]}}), g.select(parallel, {p[1], p[6], p[4]}, false, true));
	EXPECT_EQ(mark({{p[1],p[4],p[6]}}), g.select(parallel, {p[1], p[6], p[4]}, true, false));
	EXPECT_EQ(mark({{p[1],p[4],p[6]}}), g.select(parallel, {p[1], p[6], p[4]}, true, true));
	EXPECT_EQ(mark({{p[1]},{p[6]},{p[4]}}), g.select(choice, {p[1], p[6], p[4]}, false, false));
	EXPECT_EQ(mark({{p[1]},{p[6]},{p[4]}}), g.select(choice, {p[1], p[6], p[4]}, false, true));
	EXPECT_EQ(mark({{p[1]},{p[6]},{p[4]}}), g.select(choice, {p[1], p[6], p[4]}, true, false));
	EXPECT_EQ(mark({{p[1]},{p[6]},{p[4]}}), g.select(choice, {p[1], p[6], p[4]}, true, true));

	EXPECT_EQ(mark({{p[0],p[3]},{p[2],p[3]},{p[2],p[5]}}), g.select(parallel, {p[0], p[2], p[3], p[5]}, false, false));
	EXPECT_EQ(mark({{p[0],p[2],p[3],p[5]}}), g.select(parallel, {p[0], p[2], p[3], p[5]}, false, true));
	EXPECT_EQ(mark({{p[0],p[3]},{p[2],p[3]},{p[2],p[5]}}), g.select(parallel, {p[0], p[2], p[3], p[5]}, true, false));
	EXPECT_EQ(mark({{p[0],p[2],p[3],p[5]}}), g.select(parallel, {p[0], p[2], p[3], p[5]}, true, true));
	EXPECT_EQ(mark({{p[0]},{p[2]},{p[3]},{p[5]}}), g.select(choice, {p[0], p[2], p[3], p[5]}, false, false));
	EXPECT_EQ(mark({{p[0],p[2]},{p[0],p[5]},{p[3],p[5]}}), g.select(choice, {p[0], p[2], p[3], p[5]}, false, true));
	EXPECT_EQ(mark({{p[0]},{p[2]},{p[3]},{p[5]}}), g.select(choice, {p[0], p[2], p[3], p[5]}, true, false));
	EXPECT_EQ(mark({{p[0],p[2]},{p[0],p[5]},{p[3],p[5]}}), g.select(choice, {p[0], p[2], p[3], p[5]}, true, true));

	EXPECT_EQ(mark({{p[1],p[4],p[6]}}), g.select(implies, {p[1], p[6], p[4]}, false, false));
	EXPECT_EQ(mark({{p[1],p[4],p[6]}}), g.select(implies, {p[1], p[6], p[4]}, false, true));
	EXPECT_EQ(mark({{p[1],p[4],p[6]}}), g.select(implies, {p[1], p[6], p[4]}, true, false));
	EXPECT_EQ(mark({{p[1],p[4],p[6]}}), g.select(implies, {p[1], p[6], p[4]}, true, true));
	EXPECT_EQ(mark({{p[1]},{p[6]},{p[4]}}), g.select(excludes, {p[1], p[6], p[4]}, false, false));
	EXPECT_EQ(mark({{p[1]},{p[6]},{p[4]}}), g.select(excludes, {p[1], p[6], p[4]}, false, true));
	EXPECT_EQ(mark({{p[1]},{p[6]},{p[4]}}), g.select(excludes, {p[1], p[6], p[4]}, true, false));
	EXPECT_EQ(mark({{p[1]},{p[6]},{p[4]}}), g.select(excludes, {p[1], p[6], p[4]}, true, true));

	EXPECT_EQ(mark({{p[0],p[2],p[3],p[5]}}), g.select(implies, {p[0], p[2], p[3], p[5]}, false, false));
	EXPECT_EQ(mark({{p[0],p[2],p[3],p[5]}}), g.select(implies, {p[0], p[2], p[3], p[5]}, false, true));
	EXPECT_EQ(mark({{p[0],p[2],p[3],p[5]}}), g.select(implies, {p[0], p[2], p[3], p[5]}, true, false));
	EXPECT_EQ(mark({{p[0],p[2],p[3],p[5]}}), g.select(implies, {p[0], p[2], p[3], p[5]}, true, true));
	EXPECT_EQ(mark({{p[0]},{p[2]},{p[3]},{p[5]}}), g.select(excludes, {p[0], p[2], p[3], p[5]}, false, false));
	EXPECT_EQ(mark({{p[0]},{p[2]},{p[3]},{p[5]}}), g.select(excludes, {p[0], p[2], p[3], p[5]}, false, true));
	EXPECT_EQ(mark({{p[0]},{p[2]},{p[3]},{p[5]}}), g.select(excludes, {p[0], p[2], p[3], p[5]}, true, false));
	EXPECT_EQ(mark({{p[0]},{p[2]},{p[3]},{p[5]}}), g.select(excludes, {p[0], p[2], p[3], p[5]}, true, true));
}

/*

TODO(edward.bingham) this breaks the flat split group comparison method, may
require recursive algorithms.

TEST(select, shared_parallel) {
	//          ->p1-->t1-->p2-->t2-->p3             .
	//         /                        \            .
	//     ->t0                          ->t5        .
	//    /    \                        /    \       .
	//  p0      ->p4-->t3-->p5-->t4-->p6      ->p10  .
	//    \    /                        \    /       .
	//     ->t6                          ->t9        .
	//         \                        /            .
	//          ->p7-->t7-->p8-->t8-->p9             .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 11);
	auto t = g.create(transition(), 10);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[2], p[3], t[5], p[10]});
	g.connect({t[0], p[4], t[3], p[5], t[4], p[6], t[5]});
	g.connect({p[0], t[6], p[4]});
	g.connect({p[6], t[9], p[10]});
	g.connect({t[6], p[7], t[7], p[8], t[8], p[9], t[9]});

	g.compute_split_groups();

}

TEST(select, shared_choice) {
	//          ->t1-->p1-->t2-->p2-->t3             .
	//         /                        \            .
	//     ->p0                          ->p5        .
	//    /    \                        /    \       .
	//  t0      ->t4-->p3-->t5-->p4-->t6      ->t10  .
	//    \    /                        \    /       .
	//     ->p6                          ->p9        .
	//         \                        /            .
	//          ->t7-->p7-->t8-->p8-->t9             .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 10);
	auto t = g.create(transition(), 11);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[3], p[5], t[10]});
	g.connect({p[0], t[4], p[3], t[5], p[4], t[6], p[5]});
	g.connect({t[0], p[6], t[4]});
	g.connect({t[6], p[9], t[10]});
	g.connect({p[6], t[7], p[7], t[8], p[8], t[9], p[9]});

	g.compute_split_groups();

}*/

TEST(select, regular_choice_parallel) {
	//           ->p1-->t1--          .
	//          /           \         .
	//      ->t0             p2       .
	//     /    \           /  \      .
	//    /      ->p3-->t2 /    \     .
	//  p0                X      >t6  .
	//    \      ->p4-->t3 \    /     .
	//     \    /           \  /      .
	//      ->t4             p5       .
	//          \           /         .
	//           ->p6-->t5--          .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 7);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[6]});
	g.connect({t[0], p[3], t[2], p[5]});
	g.connect({t[4], p[4], t[3], p[2]});
	g.connect({p[0], t[4], p[6], t[5], p[5], t[6]});

	g.compute_split_groups();

	EXPECT_EQ(mark({{t[1],t[2]},{t[3],t[5]}}), g.select(parallel, {t[1], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(mark({{t[1],t[2]},{t[3],t[5]}}), g.select(parallel, {t[1], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(mark({{t[1],t[2]},{t[3],t[5]}}), g.select(parallel, {t[1], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(mark({{t[1],t[2]},{t[3],t[5]}}), g.select(parallel, {t[1], t[2], t[3], t[5]}, true, true));
	EXPECT_EQ(mark({{t[1],t[3]},{t[2],t[3]},{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(mark({{t[1],t[3]},{t[2],t[3]},{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(mark({{t[1],t[3]},{t[2],t[3]},{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(mark({{t[1],t[3]},{t[2],t[3]},{t[1],t[5]},{t[2],t[5]}}), g.select(choice, {t[1], t[2], t[3], t[5]}, true, true));

	EXPECT_EQ(mark({{t[1],t[2]},{t[3],t[5]}}), g.select(implies, {t[1], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(mark({{t[1],t[2]},{t[3],t[5]}}), g.select(implies, {t[1], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(mark({{t[1],t[2]},{t[3],t[5]}}), g.select(implies, {t[1], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(mark({{t[1],t[2]},{t[3],t[5]}}), g.select(implies, {t[1], t[2], t[3], t[5]}, true, true));
	EXPECT_EQ(mark({{t[1],t[3]},{t[2],t[3]},{t[1],t[5]},{t[2],t[5]}}), g.select(excludes, {t[1], t[2], t[3], t[5]}, false, false));
	EXPECT_EQ(mark({{t[1],t[3]},{t[2],t[3]},{t[1],t[5]},{t[2],t[5]}}), g.select(excludes, {t[1], t[2], t[3], t[5]}, false, true));
	EXPECT_EQ(mark({{t[1],t[3]},{t[2],t[3]},{t[1],t[5]},{t[2],t[5]}}), g.select(excludes, {t[1], t[2], t[3], t[5]}, true, false));
	EXPECT_EQ(mark({{t[1],t[3]},{t[2],t[3]},{t[1],t[5]},{t[2],t[5]}}), g.select(excludes, {t[1], t[2], t[3], t[5]}, true, true));
}

/* This structure violates liveness

TEST(select, compressed_parallel_choice) {
	// Without guards, this would deadlock when t3 and t4 or t1 and t6 are
	// executed in parallel.
	//           ->t1-->p1--          .
	//          /           \         .
	//      ->p0             t2       .
	//     /    \           /  \      .
	//    /      ->t3-->p2 /    \     .
	//  t0                X      >p6  .
	//    \      ->t4-->p3 \    /     .
	//     \    /           \  /      .
	//      ->p4             t5       .
	//          \           /         .
	//           ->t6-->p5--          .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 7);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[6]});
	g.connect({p[0], t[3], p[2], t[5]});
	g.connect({p[4], t[4], p[3], t[2]});
	g.connect({t[0], p[4], t[6], p[5], t[5], p[6]});

	g.compute_split_groups();

}*/
