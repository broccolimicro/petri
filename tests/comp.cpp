#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

using namespace petri;
using namespace std;

string print_splits(const graph<place, transition, token, state<token> > &g, petri::iterator node) {
	return ::to_string(g.split_groups_of(parallel, node)) + ::to_string(g.split_groups_of(choice, node));
}

string should_be(const graph<place, transition, token, state<token> > &g, bool be, int composition, petri::iterator a, petri::iterator b) {
	string comp = "sequence";
	if (composition == parallel) {
		comp = "parallel";
	} else if (composition == choice) {
		comp = "choice";
	}
	return a.to_string() + ":" + print_splits(g, a) + " should " + (not be ? "not " : "") + "be " + comp + " with " + b.to_string() + ":" + print_splits(g, b);
}

void test_parallel(const graph<place, transition, token, state<token> > &g, vector<petri::iterator> a, vector<petri::iterator> b) {
	for (auto i = a.begin(); i != a.end(); i++) {
		for (auto j = b.begin(); j != b.end(); j++) {
			if (*i != *j) {
				EXPECT_TRUE(g.is(parallel, *i, *j, true)) << should_be(g, true, parallel, *i, *j);
				EXPECT_TRUE(g.is(parallel, *j, *i, true)) << should_be(g, true, parallel, *j, *i);
			} else {
				EXPECT_FALSE(g.is(parallel, *i, *j)) << should_be(g, false, parallel, *i, *j);
				EXPECT_FALSE(g.is(parallel, *j, *i)) << should_be(g, false, parallel, *j, *i);
			}
			EXPECT_FALSE(g.is(choice, *i, *j)) << should_be(g, false, choice, *i, *j);
			EXPECT_FALSE(g.is(choice, *j, *i)) << should_be(g, false, choice, *j, *i);
			EXPECT_FALSE(g.is(sequence, *i, *j)) << should_be(g, false, sequence, *i, *j);
			EXPECT_FALSE(g.is(sequence, *j, *i)) << should_be(g, false, sequence, *j, *i);
		}
	}
}

void test_choice(const graph<place, transition, token, state<token> > &g, vector<petri::iterator> a, vector<petri::iterator> b) {
	for (auto i = a.begin(); i != a.end(); i++) {
		for (auto j = b.begin(); j != b.end(); j++) {
			if (*i != *j) {
				EXPECT_TRUE(g.is(choice, *i, *j, true)) << should_be(g, true, choice, *i, *j);
				EXPECT_TRUE(g.is(choice, *j, *i, true)) << should_be(g, true, choice, *j, *i);
			} else {
				EXPECT_FALSE(g.is(choice, *i, *j)) << should_be(g, false, choice, *i, *j);
				EXPECT_FALSE(g.is(choice, *j, *i)) << should_be(g, false, choice, *j, *i);
			}
			EXPECT_FALSE(g.is(parallel, *i, *j)) << should_be(g, false, parallel, *i, *j);
			EXPECT_FALSE(g.is(parallel, *j, *i)) << should_be(g, false, parallel, *j, *i);
			EXPECT_FALSE(g.is(sequence, *i, *j)) << should_be(g, false, sequence, *i, *j);
			EXPECT_FALSE(g.is(sequence, *j, *i)) << should_be(g, false, sequence, *j, *i);
		}
	}
}

void test_sequence(const graph<place, transition, token, state<token> > &g, vector<petri::iterator> a, vector<petri::iterator> b=vector<petri::iterator>()) {
	if (b.empty()) {
		b = a;
	}
	for (auto i = a.begin(); i != a.end(); i++) {
		for (auto j = b.begin(); j != b.end(); j++) {
			if (*i != *j) {
				EXPECT_TRUE(g.is(sequence, *i, *j, true)) << should_be(g, true, sequence, *i, *j);
				EXPECT_TRUE(g.is(sequence, *j, *i, true)) << should_be(g, true, sequence, *j, *i);
			} else {
				EXPECT_FALSE(g.is(sequence, *i, *j)) << should_be(g, false, sequence, *i, *j);
				EXPECT_FALSE(g.is(sequence, *j, *i)) << should_be(g, false, sequence, *j, *i);
			}
			EXPECT_FALSE(g.is(choice, *i, *j)) << should_be(g, false, choice, *i, *j);
			EXPECT_FALSE(g.is(choice, *j, *i)) << should_be(g, false, choice, *j, *i);
			EXPECT_FALSE(g.is(parallel, *i, *j)) << should_be(g, false, parallel, *i, *j);
			EXPECT_FALSE(g.is(parallel, *j, *i)) << should_be(g, false, parallel, *j, *i);
		}
	}
}

void connect_sequence(graph<place, transition, token, state<token> > &g, vector<petri::iterator> a) {
	for (auto i0 = a.begin(); next(i0) != a.end(); i0++) {
		auto i1 = next(i0);
		g.connect(*i0, *i1);
	}
}

TEST(composition, always_choice) {
	//     ->t0-->p1-->t1-      .
	//    /               \     .
	//  p0                 >p3  .
	//    \               /     .
	//     ->t2-->p2-->t3-      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 4);

	connect_sequence(g, {p[0], t[0], p[1], t[1], p[3]});
	connect_sequence(g, {p[0], t[2], p[2], t[3], p[3]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_choice(g, {t[0], p[1], t[1]}, {t[2], p[2], t[3]});
	test_sequence(g, {p[0], t[0], p[1], t[1], p[3]});
	test_sequence(g, {p[0], t[2], p[2], t[3], p[3]});
}

TEST(composition, always_parallel) {
	//     ->p0-->t1-->p1-      .
	//    /               \     .
	//  t0                 >t3  .
	//    \               /     .
	//     ->p2-->t2-->p3-      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 4);

	connect_sequence(g, {t[0], p[0], t[1], p[1], t[3]});
	connect_sequence(g, {t[0], p[2], t[2], p[3], t[3]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_parallel(g, {p[0], t[1], p[1]}, {p[2], t[2], p[3]});
	test_sequence(g, {t[0], p[0], t[1], p[1], t[3]});
	test_sequence(g, {t[0], p[2], t[2], p[3], t[3]});
}

TEST(composition, compressed_proper_nesting) {
	//  =->*p0-->t0-->p1-->t1-=  .
	//       \ /  \ /            .
	//        X    X             .
	//       / \  / \            .
	//  =->*p2-->t2-->p3-->t3-=  .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 4);

	connect_sequence(g, {p[0], t[0], p[1], t[1], p[0]});
	connect_sequence(g, {p[2], t[2], p[3], t[3], p[2]});

	g.connect(p[0], t[2]);
	g.connect(p[2], t[0]);

	g.connect(t[0], p[3]);
	g.connect(t[2], p[1]);

	g.reset.push_back(state<token>({token(p[0].index), token(p[2].index)}));

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_sequence(g, {p[0], t[0], p[1], t[1]});
	test_sequence(g, {p[2], t[2], p[3], t[3]});
	test_sequence(g, {p[0], p[1], t[1]}, {t[2]});
	test_sequence(g, {p[2], p[3], t[3]}, {t[0]});
	test_parallel(g, {p[1], t[1], p[0]}, {p[3], t[3], p[2]});
	test_choice(g, {t[0]}, {t[2]});
}

TEST(composition, choice_parallel) {
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

	connect_sequence(g, {p[0], t[0], p[1], t[1], p[2], t[3]});
	connect_sequence(g, {t[0], p[3], t[2], p[4], t[3], p[6]});
	connect_sequence(g, {p[0], t[4], p[5], t[5], p[6]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_sequence(g, {p[0], t[0], p[1], t[1], p[2], t[3], p[6]});
	test_sequence(g, {p[0], t[0], p[3], t[2], p[4], t[3], p[6]});
	test_sequence(g, {p[0], t[4], p[5], t[5], p[6]});
	test_parallel(g, {p[1], t[1], p[2]}, {p[3], t[2], p[4]});
	test_choice(g, {t[4], p[5], t[5]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3]});

	/* TODO(edward.bingham) write separate tests for this, and add index priority
  to understand not just sequencing, but order as well.

	EXPECT_TRUE(g.is(sequence, {p[1], p[3]}, {t[1], t[2]}, true));
	EXPECT_FALSE(g.is(parallel, {p[1], p[3]}, {t[1], t[2]}));
	EXPECT_FALSE(g.is(choice, {p[1], p[3]}, {t[1], t[2]}, true));
	EXPECT_TRUE(g.is(choice, {p[1], p[3]}, {t[1], t[2]}));

	EXPECT_TRUE(g.is(sequence, {p[1], p[3]}, {p[2], p[4]}, true));
	EXPECT_FALSE(g.is(parallel, {p[1], p[3]}, {p[2], p[4]}));
	EXPECT_FALSE(g.is(choice, {p[1], p[3]}, {p[2], p[4]}, true));
	EXPECT_TRUE(g.is(choice, {p[1], p[3]}, {p[2], p[4]}));

	EXPECT_FALSE(g.is(sequence, {p[1], p[4]}, {p[2], p[3]}));
	EXPECT_TRUE(g.is(choice, {p[1], p[4]}, {p[2], p[3]}, true));
	EXPECT_FALSE(g.is(parallel, {p[1], p[4]}, {p[2], p[3]}));*/
}

TEST(composition, parallel_choice) {
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

	connect_sequence(g, {t[0], p[0], t[1], p[1], t[2], p[3]});
	connect_sequence(g, {p[0], t[3], p[2], t[4], p[3], t[6]});
	connect_sequence(g, {t[0], p[4], t[5], p[5], t[6]});

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_sequence(g, {t[0], p[0], t[1], p[1], t[2], p[3], t[6]});
	test_sequence(g, {t[0], p[0], t[3], p[2], t[4], p[3], t[6]});
	test_sequence(g, {t[0], p[4], t[5], p[5], t[6]});
	test_choice(g, {t[1], p[1], t[2]}, {t[3], p[2], t[4]});
	test_parallel(g, {p[4], t[5], p[5]}, {p[0], t[1], p[1], t[2], t[3], p[2], t[4], p[3]});
}

TEST(composition, nonproper_choice) {
	//     ->t0-->p1-->t1-->p2-->t2-      .
	//    /         \               \     .
	//  p0           ->t6-           >p5  .
	//    \               \         /     .
	//     ->t3-->p3-->t4-->p4-->t5-      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 6);
	auto t = g.create(transition(), 7);

	connect_sequence(g, {p[0], t[0], p[1], t[1], p[2], t[2], p[5]});
	connect_sequence(g, {p[0], t[3], p[3], t[4], p[4], t[5], p[5]});

	g.connect(p[1], t[6]);
	g.connect(t[6], p[4]);

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_choice(g, {t[0], p[1], t[1], p[2], t[2]}, {t[3], p[3], t[4], p[4], t[5]});
	test_choice(g, {t[0], p[1], t[1], p[2], t[2]}, {t[6]});
	test_choice(g, {t[6]}, {t[3], p[3], t[4], p[4], t[5]});
	test_sequence(g, {p[0], t[0], p[1], t[1], p[2], t[2], p[5]});
	test_sequence(g, {p[0], t[3], p[3], t[4], p[4], t[5], p[5]});
	test_sequence(g, {p[0], t[0], p[1], t[6], p[4], t[5], p[5]});
}

TEST(composition, nonproper_parallel) {
	//     ->p0-->t1-->p1-->t2-->p2-      .
	//    /         \               \     .
	//  t0           ->p6-           >t5  .
	//    \               \         /     .
	//     ->p3-->t3-->p4-->t4-->p5-      .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 6);

	connect_sequence(g, {t[0], p[0], t[1], p[1], t[2], p[2], t[5]});
	connect_sequence(g, {t[0], p[3], t[3], p[4], t[4], p[5], t[5]});

	g.connect(t[1], p[6]);
	g.connect(p[6], t[4]);

	g.compute_split_groups(parallel);
	g.compute_split_groups(choice);

	test_parallel(g, {p[0], t[1], p[1], t[2], p[2], p[6]}, {p[3], t[3], p[4]});
	test_parallel(g, {p[1], t[2], p[2]}, {p[6], t[4], p[5]});
	test_sequence(g, {t[0], p[0], t[1], p[1], t[2], p[2], t[5]});
	test_sequence(g, {t[0], p[3], t[3], p[4], t[4], p[5], t[5]});
	test_sequence(g, {t[0], p[0], t[1], p[6], t[4], p[5], t[5]});
}

