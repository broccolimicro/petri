#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

using namespace petri;
using namespace std;

string print_splits(const graph<place, transition, token, state<token> > &g, petri::iterator node) {
	return "t" + ::to_string(g.split_groups_of(parallel, node)) + "p" + ::to_string(g.split_groups_of(choice, node));
}

string should_be(const graph<place, transition, token, state<token> > &g, bool be, int composition, petri::iterator a, petri::iterator b) {
	string comp = "sequence";
	if (composition == parallel) {
		comp = "parallel";
	} else if (composition == choice) {
		comp = "choice";
	} else if (composition == implies) {
		comp = "implies";
	} else if (composition == excludes) {
		comp = "excludes";
	}
	return a.to_string() + ":" + print_splits(g, a) + " should " + (not be ? "not " : "") + "be " + comp + " with " + b.to_string() + ":" + print_splits(g, b);
}

void test_always(const graph<place, transition, token, state<token> > &g, int composition, vector<petri::iterator> a, vector<petri::iterator> b=vector<petri::iterator>(), bool bidir=false) {
	for (auto i = a.begin(); i != a.end(); i++) {
		for (auto j = (b.empty() ? ::next(i) : b.begin()); j != (b.empty() ? a.end() : b.end()); j++) {
			if (*i != *j) {
				EXPECT_TRUE(g.is(composition, *i, *j, true)) << should_be(g, true, composition, *i, *j);
				if (bidir) {
					EXPECT_TRUE(g.is(composition, *j, *i, true)) << should_be(g, true, composition, *j, *i);
				}
				EXPECT_TRUE(g.is(composition, *i, *j, false)) << should_be(g, true, composition, *i, *j);
				if (bidir) {
					EXPECT_TRUE(g.is(composition, *j, *i, false)) << should_be(g, true, composition, *j, *i);
				}
			} else {
				EXPECT_FALSE(g.is(composition, *i, *j, false)) << should_be(g, false, composition, *i, *j);
				if (bidir) {
					EXPECT_FALSE(g.is(composition, *j, *i, false)) << should_be(g, false, composition, *j, *i);
				}
			}
			if (composition < 3) {
				for (int k = 0; k < 3; k++) {
					if (k != composition) {
						EXPECT_FALSE(g.is(k, *i, *j)) << should_be(g, false, k, *i, *j);
						if (bidir) {
							EXPECT_FALSE(g.is(k, *j, *i)) << should_be(g, false, k, *j, *i);
						}
					}
				}
			} else {
				for (int k = 3; k < 5; k++) {
					if (k != composition) {
						EXPECT_FALSE(g.is(k, *i, *j)) << should_be(g, false, k, *i, *j);
						if (bidir) {
							EXPECT_FALSE(g.is(k, *j, *i)) << should_be(g, false, k, *j, *i);
						}
					}
				}
			}
		}
	}
}

void test_sometimes(const graph<place, transition, token, state<token> > &g, int composition, vector<petri::iterator> a, vector<petri::iterator> b=vector<petri::iterator>(), bool bidir=false) {
	for (auto i = a.begin(); i != a.end(); i++) {
		for (auto j = (b.empty() ? ::next(i) : b.begin()); j != (b.empty() ? a.end() : b.end()); j++) {
			if (*i != *j) {
				EXPECT_TRUE(g.is(composition, *i, *j, false)) << should_be(g, true, composition, *i, *j);
				if (bidir) {
					EXPECT_TRUE(g.is(composition, *j, *i, false)) << should_be(g, true, composition, *j, *i);
				}
			} else {
				EXPECT_FALSE(g.is(composition, *i, *j, false)) << should_be(g, false, composition, *i, *j);
				if (bidir) {
					EXPECT_FALSE(g.is(composition, *j, *i, false)) << should_be(g, false, composition, *j, *i);
				}
			}
		}
	}
}

void test_not(const graph<place, transition, token, state<token> > &g, int composition, vector<petri::iterator> a, vector<petri::iterator> b=vector<petri::iterator>(), bool bidir=false) {
	for (auto i = a.begin(); i != a.end(); i++) {
		for (auto j = (b.empty() ? ::next(i) : b.begin()); j != (b.empty() ? a.end() : b.end()); j++) {
			EXPECT_FALSE(g.is(composition, *i, *j, false)) << should_be(g, false, composition, *i, *j);
			if (bidir) {
				EXPECT_FALSE(g.is(composition, *j, *i, false)) << should_be(g, false, composition, *j, *i);
			}
		}
	}
}

void test_not_always(const graph<place, transition, token, state<token> > &g, int composition, vector<petri::iterator> a, vector<petri::iterator> b=vector<petri::iterator>(), bool bidir=false) {
	for (auto i = a.begin(); i != a.end(); i++) {
		for (auto j = (b.empty() ? ::next(i) : b.begin()); j != (b.empty() ? a.end() : b.end()); j++) {
			EXPECT_FALSE(g.is(composition, *i, *j, true)) << should_be(g, false, composition, *i, *j);
			if (bidir) {
				EXPECT_FALSE(g.is(composition, *j, *i, true)) << should_be(g, false, composition, *j, *i);
			}
		}
	}
}

TEST(composition, always_choice) {
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

	test_always(g, choice, {t[0], p[1], t[1]}, {t[2], p[2], t[3]}, true);
	test_always(g, sequence, {t[5], p[0], t[0], p[1], t[1], p[3], t[4]}, {}, true);
	test_always(g, sequence, {t[5], p[0], t[2], p[2], t[3], p[3], t[4]}, {}, true);

	test_always(g, implies, {t[0], p[1], t[1], p[3], t[4]});
	test_always(g, implies, {t[2], p[2], t[3], p[3], t[4]});
	test_always(g, implies, {t[5], p[0]});
	test_always(g, implies, {t[1], p[1], t[0], p[0], t[5]});
	test_always(g, implies, {t[3], p[2], t[2], p[0], t[5]});
	test_always(g, implies, {t[4], p[3]});
	test_sometimes(g, excludes, {t[5], p[0], t[4], p[3]}, {t[0], p[1], t[1]});
	test_sometimes(g, excludes, {t[5], p[0], t[4], p[3]}, {t[2], p[2], t[3]});
	test_sometimes(g, implies, {t[5], p[0], t[4], p[3]}, {t[0], p[1], t[1]});
	test_sometimes(g, implies, {t[5], p[0], t[4], p[3]}, {t[2], p[2], t[3]});
}

TEST(composition, always_parallel) {
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

	test_always(g, parallel, {p[0], t[1], p[1]}, {p[2], t[2], p[3]}, true);
	test_always(g, sequence, {p[5], t[0], p[0], t[1], p[1], t[3], p[4]}, {}, true);
	test_always(g, sequence, {p[5], t[0], p[2], t[2], p[3], t[3], p[4]}, {}, true);

	test_always(g, implies, {p[5], t[0], p[0], t[1], p[1], t[3], p[4]}, {}, true);
	test_always(g, implies, {p[5], t[0], p[2], t[2], p[3], t[3], p[4]}, {}, true);
	test_always(g, implies, {p[0], t[1], p[1]}, {p[2], t[2], p[3]}, true);
	test_not(g, excludes, {p[5], t[0], p[0], t[1], p[1], t[3], p[4]}, {}, true);
	test_not(g, excludes, {p[5], t[0], p[2], t[2], p[3], t[3], p[4]}, {}, true);
	test_not(g, excludes, {p[0], t[1], p[1]}, {p[2], t[2], p[3]}, true);
}

TEST(composition, regular_interleaved) {
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

	test_always(g, sequence, {p[0], t[0], p[1], t[1]}, {}, true);
	test_always(g, sequence, {p[2], t[2], p[3], t[3]}, {}, true);
	test_always(g, sequence, {p[0], t[2], p[1], t[1]}, {}, true);
	test_always(g, sequence, {p[2], t[0], p[3], t[3]}, {}, true);
	test_always(g, parallel, {p[1], t[1], p[0]}, {p[3], t[3], p[2]}, true);
	test_always(g, choice, {t[0]}, {t[2]}, true);
	test_not(g, parallel, {p[1], t[1], p[0], p[3], t[3], p[2]}, {t[0], t[2]});

	test_always(g, implies, {p[0], p[1], t[1]}, {}, true);
	test_always(g, implies, {p[2], p[3], t[3]}, {}, true);
	test_always(g, implies, {t[0], t[2]}, {p[1], t[1], p[0], p[3], t[3], p[2]});
	test_sometimes(g, implies, {p[1], t[1], p[0], p[3], t[3], p[2]}, {t[0], t[2]});
	test_sometimes(g, excludes, {p[1], t[1], p[0], p[3], t[3], p[2]}, {t[0], t[2]});
}

TEST(composition, regular_parallel) {
	//  =>t0-->p0-->t1-->*p1-->=  .
	//     \      /  \       /    .
	//      ->p2 /    ->*p3 /     .
	//          X          X      .
	//      ->p4 \    ->*p5 \     .
	//     /      \  /       \    .
	//  =>t2-->p6-->t3-->*p7-->=  .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 8);
	auto t = g.create(transition(), 4);

	g.connect({t[0], p[0], t[1], p[1], t[0]});
	g.connect({t[2], p[6], t[3], p[7], t[2]});

	g.connect({t[0], p[2], t[3], p[5], t[0]});
	g.connect({t[2], p[4], t[1], p[3], t[2]});

	g.reset.push_back(state<token>({token(p[1].index), token(p[3].index), token(p[5].index), token(p[7].index)}));

	g.compute_split_groups();

	test_always(g, sequence, {t[0], p[0], t[1], p[1]}, {}, true);
	test_always(g, sequence, {t[2], p[6], t[3], p[7]}, {}, true);
	test_always(g, sequence, {t[0], p[2], t[3], p[5]}, {}, true);
	test_always(g, sequence, {t[2], p[4], t[1], p[3]}, {}, true);

	test_always(g, parallel, {t[0]}, {t[2]}, true);
	test_always(g, parallel, {t[1]}, {t[3]}, true);
	test_always(g, parallel, {p[0], p[2], p[4], p[6]}, {}, true);
	test_always(g, parallel, {p[1], p[3], p[5], p[7]}, {}, true);
	test_always(g, parallel, {p[5], p[1], t[0], p[0], p[2]}, {t[2]}, true);
	test_always(g, parallel, {t[0]}, {p[3], p[7], t[2], p[4], p[6]}, true);
	test_always(g, parallel, {p[0], p[4], t[1], p[1], p[3]}, {t[3]}, true);
	test_always(g, parallel, {t[1]}, {p[2], p[6], t[3], p[5], p[7]}, true);

	test_always(g, implies, {t[0], p[0], t[1], p[1]}, {}, true);
	test_always(g, implies, {t[2], p[6], t[3], p[7]}, {}, true);
	test_always(g, implies, {t[0], p[2], t[3], p[5]}, {}, true);
	test_always(g, implies, {t[2], p[4], t[1], p[3]}, {}, true);
}

TEST(composition, regular_choice) {
	//  =>p0*->t0-->p1-->t1-->=  .
	//     \      /  \      /    .
	//      ->t2 /    ->t3 /     .
	//          X         X      .
	//      ->t4 \    ->t5 \     .
	//     /      \  /      \    .
	//  =>p2-->t6-->p3-->t7-->=  .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 8);

	g.connect({p[0], t[0], p[1], t[1], p[0]});
	g.connect({p[2], t[6], p[3], t[7], p[2]});

	g.connect({p[0], t[2], p[3], t[5], p[0]});
	g.connect({p[2], t[4], p[1], t[3], p[2]});

	g.reset.push_back(state<token>({token(p[0].index)}));
	g.reset.push_back(state<token>({token(p[2].index)}));

	g.compute_split_groups();

	test_always(g, sequence, {p[0], t[0], p[1], t[1]}, {}, true);
	test_always(g, sequence, {p[0], t[2], p[3], t[5]}, {}, true);
	test_always(g, sequence, {p[2], t[4], p[1], t[3]}, {}, true);
	test_always(g, sequence, {p[2], t[6], p[3], t[7]}, {}, true);

	test_always(g, choice, {p[0]}, {p[2]}, true);
	test_always(g, choice, {p[1]}, {p[3]}, true);
	test_always(g, choice, {t[0], t[2], t[4], t[6]}, {}, true);
	test_always(g, choice, {t[1], t[3], t[5], t[7]}, {}, true);
	test_always(g, choice, {t[5], t[1], p[0], t[0], t[2]}, {p[2]}, true);
	test_always(g, choice, {p[0]}, {t[3], t[7], p[2], t[4], t[6]}, true);
	test_always(g, choice, {t[0], t[4], p[1], t[1], t[3]}, {p[3]}, true);
	test_always(g, choice, {p[1]}, {t[2], t[6], p[3], t[5], t[7]}, true);

	test_sometimes(g, excludes, {p[0]}, {t[0], t[2], p[1], p[3], t[1], t[5]});
	test_sometimes(g, excludes, {p[2]}, {t[4], t[6], p[1], p[3], t[3], t[7]});
	test_sometimes(g, implies, {p[0]}, {t[0], t[2], p[1], p[3], t[1], t[5]});
	test_sometimes(g, implies, {p[2]}, {t[4], t[6], p[1], p[3], t[3], t[7]});
	
	test_sometimes(g, excludes, {p[1]}, {t[1], t[3], p[0], p[2], t[0], t[4]});
	test_sometimes(g, excludes, {p[3]}, {t[5], t[7], p[0], p[2], t[2], t[6]});
	test_sometimes(g, implies, {p[1]}, {t[1], t[3], p[0], p[2], t[0], t[4]});
	test_sometimes(g, implies, {p[3]}, {t[5], t[7], p[0], p[2], t[2], t[6]});
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

	g.connect({p[0], t[0], p[1], t[1], p[2], t[3]});
	g.connect({t[0], p[3], t[2], p[4], t[3], p[6]});
	g.connect({p[0], t[4], p[5], t[5], p[6]});

	g.compute_split_groups();

	test_always(g, sequence, {p[0], t[0], p[1], t[1], p[2], t[3], p[6]}, {}, true);
	test_always(g, sequence, {p[0], t[0], p[3], t[2], p[4], t[3], p[6]}, {}, true);
	test_always(g, sequence, {p[0], t[4], p[5], t[5], p[6]}, {}, true);
	test_always(g, parallel, {p[1], t[1], p[2]}, {p[3], t[2], p[4]}, true);
	test_always(g, choice, {t[4], p[5], t[5]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3]}, true);

	test_always(g, implies, {t[0], p[1], t[1], p[2], t[3], p[6]});
	test_always(g, implies, {t[3], p[2], t[1], p[1], t[0], p[0]});
	test_always(g, implies, {t[0], p[3], t[2], p[4], t[3], p[6]});
	test_always(g, implies, {t[3], p[4], t[2], p[3], t[0], p[0]});
	test_always(g, implies, {t[4], p[5], t[5], p[6]});
	test_always(g, implies, {t[5], p[5], t[4], p[0]});
	test_always(g, implies, {p[1], t[1], p[2]}, {p[3], t[2], p[4]}, true);
	test_always(g, excludes, {t[4], p[5], t[5]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3]}, true);

	test_always(g, implies, {p[0], p[6]});
	test_always(g, implies, {p[6], p[0]});
	
	test_sometimes(g, implies, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});
	test_not_always(g, excludes, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});
	test_sometimes(g, excludes, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});
	test_not_always(g, implies, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});


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

	g.connect({t[0], p[0], t[1], p[1], t[2], p[3]});
	g.connect({p[0], t[3], p[2], t[4], p[3], t[6]});
	g.connect({t[0], p[4], t[5], p[5], t[6]});

	g.compute_split_groups();

	test_always(g, sequence, {t[0], p[0], t[1], p[1], t[2], p[3], t[6]}, {}, true);
	test_always(g, sequence, {t[0], p[0], t[3], p[2], t[4], p[3], t[6]}, {}, true);
	test_always(g, sequence, {t[0], p[4], t[5], p[5], t[6]}, {}, true);
	test_always(g, choice, {t[1], p[1], t[2]}, {t[3], p[2], t[4]}, true);
	test_always(g, parallel, {p[0], t[1], p[1], t[2], t[3], p[2], t[4], p[3]}, {p[4], t[5], p[5]}, true);
	
	test_always(g, implies, {t[1], p[1], t[2], p[3], t[6]});
	test_always(g, implies, {t[3], p[2], t[4], p[3], t[6]});
	test_always(g, implies, {t[0], p[4], t[5], p[5], t[6]});
	test_always(g, excludes, {t[1], p[1], t[2]}, {t[3], p[2], t[4]}, true);
	test_always(g, implies, {p[0], t[1], p[1], t[2], t[3], p[2], t[4], p[3]}, {p[4], t[5], p[5]});
	test_always(g, implies, {p[4], t[5], p[5]}, {p[0], p[3]});
	test_sometimes(g, implies, {p[4], t[5], p[5]}, {t[1], p[1], t[2], t[3], p[2], t[4]});
	test_sometimes(g, excludes, {p[4], t[5], p[5]}, {t[1], p[1], t[2], t[3], p[2], t[4]});

	test_always(g, implies, {t[2], p[1], t[1], p[0], t[0]});
	test_always(g, implies, {t[4], p[2], t[3], p[0], t[0]});
	test_always(g, implies, {t[6], p[5], t[5], p[4], t[0]});
	test_always(g, implies, {t[0], p[0], p[3], t[6]});
	test_always(g, implies, {t[6], p[3], p[0], t[0]});

	test_sometimes(g, implies, {p[0], p[3]}, {t[1], p[1], t[2], t[3], p[2], t[4]});
	test_sometimes(g, excludes, {p[0], p[3]}, {t[1], p[1], t[2], t[3], p[2], t[4]});
}

TEST(composition, sequence_choice_parallel) {
	//           -->p1-->t1-->p2                    -->t7-->p8-->t7               .
	//          /               \                  /               \              .
	//      ->t0-->p3-->t2-->p4-->t3-          ->p7-->t9-->p9-->t10->p10-         .
	//     /                         \        /                          \        .
	// =>p0                           >p6-->t6                            >t12-=  .
	//     \                         /        \                          /        .
	//      ->t4-->p5-->t5-----------          ->p11->t11->p12-----------         .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 13);
	auto t = g.create(transition(), 13);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[3]});
	g.connect({t[0], p[3], t[2], p[4], t[3], p[6]});
	g.connect({p[0], t[4], p[5], t[5], p[6]});
	g.connect(p[6], t[6]);
	g.connect({t[6], p[7], t[7], p[8], t[8], p[10]});
	g.connect({p[7], t[9], p[9], t[10], p[10], t[12]});
	g.connect({t[6], p[11], t[11], p[12], t[12]});
	g.connect(t[12], p[0]);

	g.reset.push_back(state<token>({token(p[0].index)}));

	g.compute_split_groups();

	test_always(g, sequence, {p[0], t[0], p[1], t[1], p[2], t[3], p[6]}, {}, true);
	test_always(g, sequence, {p[0], t[0], p[3], t[2], p[4], t[3], p[6]}, {}, true);
	test_always(g, sequence, {p[0], t[4], p[5], t[5], p[6]}, {}, true);
	test_always(g, parallel, {p[1], t[1], p[2]}, {p[3], t[2], p[4]}, true);
	test_always(g, choice, {t[4], p[5], t[5]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3]}, true);

	test_always(g, implies, {t[0], p[1], t[1], p[2], t[3], p[6]});
	test_always(g, implies, {t[3], p[2], t[1], p[1], t[0], p[0]});
	test_always(g, implies, {t[0], p[3], t[2], p[4], t[3], p[6]});
	test_always(g, implies, {t[3], p[4], t[2], p[3], t[0], p[0]});
	test_always(g, implies, {t[4], p[5], t[5], p[6]});
	test_always(g, implies, {t[5], p[5], t[4], p[0]});
	test_always(g, implies, {p[1], t[1], p[2]}, {p[3], t[2], p[4]}, true);
	test_always(g, excludes, {t[4], p[5], t[5]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3]}, true);

	test_always(g, implies, {p[0], p[6]});
	test_always(g, implies, {p[6], p[0]});
	
	test_sometimes(g, implies, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});
	test_not_always(g, excludes, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});
	test_sometimes(g, excludes, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});
	test_not_always(g, implies, {p[0], p[6]}, {t[0], p[1], t[1], p[2], p[3], t[2], p[4], t[3], t[4], p[5], t[5]});

	test_always(g, sequence, {t[6], p[7], t[7], p[8], t[8], p[10], t[12]}, {}, true);
	test_always(g, sequence, {t[6], p[7], t[9], p[9], t[10], p[10], t[12]}, {}, true);
	test_always(g, sequence, {t[6], p[11], t[11], p[12], t[12]}, {}, true);
	test_always(g, choice, {t[7], p[8], t[8]}, {t[9], p[9], t[10]}, true);
	test_always(g, parallel, {p[7], t[7], p[8], t[8], t[9], p[9], t[10], p[10]}, {p[11], t[11], p[12]}, true);
	
	test_always(g, implies, {t[7], p[8], t[8], p[10], t[12]});
	test_always(g, implies, {t[9], p[9], t[10], p[10], t[12]});
	test_always(g, implies, {t[6], p[11], t[11], p[12], t[12]});
	test_always(g, excludes, {t[7], p[8], t[8]}, {t[9], p[9], t[10]}, true);
	test_always(g, implies, {p[7], t[7], p[8], t[8], t[9], p[9], t[10], p[10]}, {p[11], t[11], p[12]});
	test_always(g, implies, {p[11], t[11], p[12]}, {p[7], p[10]});
	test_sometimes(g, implies, {p[11], t[11], p[12]}, {t[7], p[8], t[8], t[9], p[9], t[10]});
	test_sometimes(g, excludes, {p[11], t[11], p[12]}, {t[7], p[8], t[8], t[9], p[9], t[10]});

	test_always(g, implies, {t[8], p[8], t[7], p[7], t[12]});
	test_always(g, implies, {t[10], p[9], t[9], p[7], t[12]});
	test_always(g, implies, {t[6], p[12], t[11], p[11], t[12]});
	test_always(g, implies, {t[6], p[7], p[10], t[12]});
	test_always(g, implies, {t[6], p[10], p[7], t[12]});

	test_sometimes(g, implies, {p[7], p[10]}, {t[7], p[8], t[8], t[9], p[9], t[10]});
	test_sometimes(g, excludes, {p[7], p[10]}, {t[7], p[8], t[8], t[9], p[9], t[10]});

	test_always(g, sequence, {p[0], t[0], p[1], t[1], p[2], t[3], p[6], p[3], t[2], p[4], t[4], p[5], t[5]}, {t[6], p[7], t[7], p[8], t[8], p[10], t[12], t[9], p[9], t[10], p[11], t[11], p[12]}, true);
	test_sometimes(g, implies, {p[0], t[0], p[1], t[1], p[2], t[3], p[6], p[3], t[2], p[4], t[4], p[5], t[5]}, {t[6], p[7], t[7], p[8], t[8], p[10], t[12], t[9], p[9], t[10], p[11], t[11], p[12]}, true);
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

	g.connect({p[0], t[0], p[1], t[1], p[2], t[2], p[5]});
	g.connect({p[0], t[3], p[3], t[4], p[4], t[5], p[5]});

	g.connect(p[1], t[6]);
	g.connect(t[6], p[4]);

	g.compute_split_groups();

	test_always(g, choice, {t[0], p[1], t[1], p[2], t[2]}, {t[3], p[3], t[4]}, true);
	test_always(g, choice, {t[1], p[2], t[2]}, {t[6], p[4], t[5]}, true);

	test_always(g, sequence, {p[0], t[0], p[1], t[1], p[2], t[2], p[5]}, {}, true);

	test_always(g, sequence, {p[0], t[0], p[1], t[6], p[5]}, {}, true);
	test_always(g, sequence, {p[0], p[4], t[5], p[5]}, {}, true);
	test_sometimes(g, sequence, {t[0], p[1], t[6]}, {p[4], t[5]}, true);
	test_sometimes(g, choice, {t[0], p[1]}, {p[4], t[5]}, true);

	test_always(g, sequence, {p[0], t[3], p[3], t[4], p[5]}, {}, true);
	test_always(g, sequence, {p[0], p[4], t[5], p[5]}, {}, true);
	test_sometimes(g, sequence, {t[3], p[3], t[4]}, {p[4], t[5]}, true);
	
	test_always(g, choice, {t[6]}, {t[3], p[3], t[4], t[1], p[2], t[2]});

	test_always(g, excludes, {t[0], p[1], t[1], p[2], t[2]}, {t[3], p[3], t[4]}, true);
	test_always(g, excludes, {t[1], p[2], t[2]}, {t[6], p[4], t[5]}, true);
	test_always(g, implies, {t[0], p[1], p[5]});
	test_always(g, implies, {t[1], p[2], t[2], p[5]});
	test_always(g, implies, {t[6], p[4], t[5], p[5]});

	test_always(g, implies, {t[2], p[2], t[1], p[1], t[0], p[0]});
	test_always(g, implies, {t[5], p[4], p[0]});
	test_always(g, implies, {t[6], p[1], t[0], p[0]});
	test_always(g, implies, {t[4], p[3], t[3], p[0]});

	// TODO(edward.bingham) It should be this rather than what is listed below
	//test_always(g, implies, {t[3], p[3], t[4], p[4], t[5], p[5]});
		test_always(g, implies, {t[3], p[3], t[4], p[5]});
		test_always(g, implies, {p[4], t[5], p[5]});
		test_sometimes(g, implies, {t[3], p[3], t[4]}, {p[4], t[5]});

	test_sometimes(g, excludes, {t[0], p[1]}, {p[4], t[5]}, true);
	test_sometimes(g, implies, {t[0], p[1]}, {p[4], t[5]}, true);

	test_always(g, excludes, {t[6]}, {t[3], p[3], t[4], t[1], p[2], t[2]}, true);
	test_sometimes(g, implies, {t[0], p[1], p[4], t[5]}, {t[6]});
	test_sometimes(g, excludes, {t[0], p[1], p[4], t[5]}, {t[6]});
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

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[5]});
	g.connect({t[0], p[3], t[3], p[4], t[4], p[5], t[5]});

	g.connect(t[1], p[6]);
	g.connect(p[6], t[4]);

	g.compute_split_groups();

	test_always(g, parallel, {p[0], t[1], p[1], t[2], p[2], p[6]}, {p[3], t[3], p[4]}, true);
	test_always(g, parallel, {p[1], t[2], p[2]}, {p[6], t[4], p[5]}, true);
	test_always(g, sequence, {t[0], p[0], t[1], p[1], t[2], p[2], t[5]}, {}, true);
	test_always(g, sequence, {t[0], p[3], t[3], p[4], t[4], p[5], t[5]}, {}, true);
	test_always(g, sequence, {t[0], p[0], t[1], p[6], t[4], p[5], t[5]}, {}, true);

	test_always(g, implies, {t[0], p[0], t[1], p[1], t[2], p[2], t[5], p[6], p[3], t[3], p[4], t[4], p[5]}, {}, true);
}

TEST(composition, shared_parallel) {
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

	test_always(g, sequence, {p[0], t[0], p[1], t[1], p[2], t[2], p[3], t[5], p[10]}, {}, true);
	test_always(g, sequence, {p[0], p[4], t[3], p[5], t[4], p[6], p[10]}, {}, true);
	test_sometimes(g, sequence, {p[0], t[0], p[4], t[3], p[5], t[4], p[6], t[5], p[10]}, {}, true);
	test_sometimes(g, sequence, {p[0], t[6], p[4], t[3], p[5], t[4], p[6], t[9], p[10]}, {}, true);
	test_always(g, sequence, {p[0], t[6], p[7], t[7], p[8], t[8], p[9], t[9], p[10]}, {}, true);
	test_always(g, choice, {t[0], p[1], t[1], p[2], t[2], p[3], t[5]}, {t[6], p[7], t[7], p[8], t[8], p[9], t[9]}, true);

	test_sometimes(g, choice, {p[4], t[3], p[5], t[4], p[6]}, {p[7], t[7], p[8], t[8], p[9]});
	test_sometimes(g, parallel, {p[4], t[3], p[5], t[4], p[6]}, {p[7], t[7], p[8], t[8], p[9]});
	
	test_sometimes(g, choice, {p[4], t[3], p[5], t[4], p[6]}, {p[1], t[1], p[2], t[2], p[3]});
	test_sometimes(g, parallel, {p[4], t[3], p[5], t[4], p[6]}, {p[1], t[1], p[2], t[2], p[3]});

	test_always(g, implies, {t[0], p[1], t[1], p[2], t[2], p[3], t[5]});
	test_always(g, implies, {t[0]}, {p[4], t[3], p[5], t[4], p[6], t[5]});
	test_always(g, implies, {t[6]}, {p[4], t[3], p[5], t[4], p[6], t[9]});
	test_always(g, implies, {t[6], p[7], t[7], p[8], t[8], p[9], t[9]});
	test_always(g, implies, {p[0], p[4], t[3], p[5], t[4], p[6], p[10]});
	
	test_always(g, excludes, {t[0], p[1], t[1], p[2], t[2], p[3], t[5]}, {t[6], p[7], t[7], p[8], t[8], p[9], t[9]}, true);

	test_sometimes(g, excludes, {p[4], t[3], p[5], t[4], p[6]}, {p[7], t[7], p[8], t[8], p[9]});
	test_sometimes(g, implies, {p[4], t[3], p[5], t[4], p[6]}, {p[7], t[7], p[8], t[8], p[9]});
	test_always(g, implies, {p[7], t[7], p[8], t[8], p[9]}, {p[4], t[3], p[5], t[4], p[6]});
	
	test_sometimes(g, excludes, {p[4], t[3], p[5], t[4], p[6]}, {p[1], t[1], p[2], t[2], p[3]});
	test_sometimes(g, implies, {p[4], t[3], p[5], t[4], p[6]}, {p[1], t[1], p[2], t[2], p[3]});
	test_always(g, implies, {p[1], t[1], p[2], t[2], p[3]}, {p[4], t[3], p[5], t[4], p[6]});
}

TEST(composition, shared_choice) {
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

	test_always(g, sequence, {t[1], p[1], t[2], p[2], t[3]});
	test_always(g, sequence, {t[4], p[3], t[5], p[4], t[6]});
	test_always(g, sequence, {t[7], p[7], t[8], p[8], t[9]});

	// TODO(edward.bingham) I think this may require recursive logic... which
	// parallel branches is this node a part of in which conditions?
	//test_always(g, parallel, {t[1], p[1], t[2], p[2], t[3]}, {t[7], p[7], t[8], p[8], t[9]}, true);
	//test_always(g, parallel, {p[0]}, {p[6]}, true);
	//test_always(g, parallel, {p[5]}, {p[9]}, true);
	test_always(g, choice, {t[4], p[3], t[5], p[4], t[6]}, {t[7], p[7], t[8], p[8], t[9], t[1], p[1], t[2], p[2], t[3]}, true);

	test_always(g, implies, {t[1], p[1], t[2], p[2], t[3]});
	test_always(g, implies, {t[4], p[3], t[5], p[4], t[6]});
	test_always(g, implies, {t[7], p[7], t[8], p[8], t[9]});
	
	// TODO(edward.bingham) I think this may require recursive logic... which
	// parallel branches is this node a part of in which conditions?
	//test_always(g, implies, {t[1], p[1], t[2], p[2], t[3]}, {t[7], p[7], t[8], p[8], t[9]}, true);
	//test_always(g, implies, {p[0]}, {p[6]}, true);
	//test_always(g, implies, {p[5]}, {p[9]}, true);
	test_always(g, excludes, {t[4], p[3], t[5], p[4], t[6]}, {t[7], p[7], t[8], p[8], t[9], t[1], p[1], t[2], p[2], t[3]}, true);
}

TEST(composition, regular_choice_parallel) {
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

	test_always(g, sequence, {p[0], t[0], p[1], t[1], p[2], t[6]});
	test_always(g, sequence, {p[0], t[0], p[3], t[2], p[5], t[6]});
	test_always(g, sequence, {p[0], t[4], p[4], t[3], p[2], t[6]});
	test_always(g, sequence, {p[0], t[4], p[6], t[5], p[5], t[6]});

	test_always(g, parallel, {p[1], t[1]}, {p[3], t[2]}, true);
	test_always(g, parallel, {p[4], t[3]}, {p[6], t[5]}, true);
	test_always(g, parallel, {p[2]}, {p[5]}, true);
	
	test_always(g, choice, {p[1], t[1]}, {p[4], t[3]}, true);
	test_always(g, choice, {p[1], t[1]}, {p[6], t[5]}, true);
	test_always(g, choice, {p[3], t[2]}, {p[4], t[3]}, true);
	test_always(g, choice, {p[3], t[2]}, {p[6], t[5]}, true);
	test_always(g, choice, {t[0]}, {t[4]}, true);

	test_always(g, implies, {t[0], p[1], t[1]});
	test_always(g, implies, {t[0], p[3], t[2]});
	test_always(g, implies, {t[4], p[4], t[3]});
	test_always(g, implies, {t[4], p[6], t[5]});

	test_always(g, implies, {p[1], t[1]}, {p[3], t[2]}, true);
	test_always(g, implies, {p[4], t[3]}, {p[6], t[5]}, true);
	test_always(g, implies, {p[2]}, {p[5]}, true);
	
	test_always(g, excludes, {p[1], t[1]}, {p[4], t[3]}, true);
	test_always(g, excludes, {p[1], t[1]}, {p[6], t[5]}, true);
	test_always(g, excludes, {p[3], t[2]}, {p[4], t[3]}, true);
	test_always(g, excludes, {p[3], t[2]}, {p[6], t[5]}, true);
	test_always(g, excludes, {t[0]}, {t[4]}, true);

	test_sometimes(g, implies, {p[0]}, {t[0], p[1], t[1], p[3], t[2], t[4], p[4], t[3], p[6], t[5]});
	test_sometimes(g, excludes, {p[0]}, {t[0], p[1], t[1], p[3], t[2], t[4], p[4], t[3], p[6], t[5]});
	test_always(g, implies, {p[0]}, {p[2], p[5], t[6]}, true);
	test_always(g, implies, {p[2], p[5]}, {t[6]}, true);
}

/* This structure violates liveness

TEST(composition, regular_parallel_choice) {
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

	test_always(g, sequence, {t[1], p[1], t[2]});
	test_always(g, sequence, {t[3], p[2], t[5]});
	test_always(g, sequence, {t[4], p[3], t[2]});
	test_always(g, sequence, {t[6], p[5], t[5]});

	test_always(g, choice, {t[1], p[1], t[2]}, {t[3], p[2], t[5]}, true);
	test_always(g, choice, {t[4], p[3], t[2]}, {t[6], p[5], t[5]}, true);
	
	test_always(g, parallel, {t[1], p[1]}, {t[4], p[3]}, true);
	test_always(g, parallel, {t[3], p[2]}, {t[6], p[5]}, true);
	test_always(g, parallel, {p[0]}, {p[4]}, true);

	test_sometimes(g, sequence, {p[0]}, {t[1], p[1], t[2], t[3], p[2], t[5]});
	test_sometimes(g, sequence, {p[4]}, {t[4], p[3], t[2], t[6], p[5], t[5]});
	test_sometimes(g, choice, {p[0]}, {t[1], p[1], t[2], t[3], p[2], t[5]});
	test_sometimes(g, choice, {p[4]}, {t[4], p[3], t[2], t[6], p[5], t[5]});
	test_not(g, parallel, {p[0]}, {t[1], p[1], t[2], t[3], p[2], t[5]}, true);
	test_not(g, parallel, {p[4]}, {t[4], p[3], t[2], t[6], p[5], t[5]}, true);
	test_always(g, sequence, {t[0]}, {p[0], p[4], p[6]}, true);
	test_always(g, sequence, {t[0], p[0], p[4]}, {p[6]}, true);
}*/


