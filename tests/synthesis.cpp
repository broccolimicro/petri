#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>
#include <petri/tree.h>
#include <petri/synthesize.h>

using namespace petri;
using namespace std;

void run_test(const graph<place, transition, state<token> > &g, bool isProper) {
	graph<place, controlflow::Tree<transition>, state<token> > tree;
	EXPECT_EQ(graph_to_tree(tree, g), isProper);

	tree.print();
	for (size_t i = 0; i < tree.transitions.size(); i++) {
		if (not tree.transitions.is_valid(i)) continue;

		printf("T%d: ", (int)i);
		tree.transitions[i].print();
		printf("\n");
	}
}

TEST(controlflow, always_choice) {
	//          ->t0-->p1-->t1-           .
	//         /               \          .
	//  t5-->p0                 >p3-->t4  .
	//         \               /          .
	//          ->t2-->p2-->t3-           .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 6);

	g.connect({t[5], p[0], t[0], p[1], t[1], p[3], t[4]});
	g.connect({p[0], t[2], p[2], t[3], p[3]});

	run_test(g, true);
}

TEST(controlflow, always_parallel) {
	//          ->p0-->t1-->p1-           .
	//         /               \          .
	//  p5-->t0                 >t3-->p4  .
	//         \               /          .
	//          ->p2-->t2-->p3-           .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 6);
	auto t = g.create(transition(), 4);

	g.connect({p[5], t[0], p[0], t[1], p[1], t[3], p[4]});
	g.connect({t[0], p[2], t[2], p[3], t[3]});

	run_test(g, true);
}

TEST(controlflow, choice_parallel) {
	//          -->p1-->t1-->p2           .
	//         /               \          .
	//     ->t0-->p3-->t2-->p4-->t3-      .
	//    /                         \     .
	//  p0                           >p6  .
	//    \                         /     .
	//     ->t4-->p5-->t5-----------      .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 6);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[3]});
	g.connect({t[0], p[3], t[2], p[4], t[3], p[6]});
	g.connect({p[0], t[4], p[5], t[5], p[6]});

	run_test(g, true);
}

TEST(controlflow, parallel_choice) {
	//          -->t1-->p1-->t2           .
	//         /               \          .
	//     ->p0-->t3-->p2-->t4-->p3-      .
	//    /                         \     .
	//  t0                           >t6  .
	//    \                         /     .
	//     ->p4-->t5-->p5-----------      .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 6);
	auto t = g.create(transition(), 7);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[3]});
	g.connect({p[0], t[3], p[2], t[4], p[3], t[6]});
	g.connect({t[0], p[4], t[5], p[5], t[6]});

	run_test(g, true);
}

TEST(controlflow, sequence_choice_parallel) {
	//           -->p1-->t1-->p2                    -->t7-->p8-->t7               .
	//          /               \                  /               \              .
	//      ->t0-->p3-->t2-->p4-->t3-          ->p7-->t9-->p9-->t10->p10-         .
	//     /                         \        /                          \        .
	// =>p0                           >p6-->t6                            >t12-=  .
	//     \                         /        \                          /        .
	//      ->t4-->p5-->t5-----------          ->p11->t11->p12-----------         .

	graph<place, transition, state<token> > g;

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

	run_test(g, true);
}

TEST(controlflow, regular_interleaved) {
	//  =->*p0-->t0-->p1-->t1-=  .
	//       \ /  \ /            .
	//        X    X             .
	//       / \  / \            .
	//  =->*p2-->t2-->p3-->t3-=  .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 4);

	g.connect({p[0], t[0], p[1], t[1], p[0]});
	g.connect({p[2], t[2], p[3], t[3], p[2]});

	g.connect(p[0], t[2]);
	g.connect(p[2], t[0]);

	g.connect(t[0], p[3]);
	g.connect(t[2], p[1]);

	g.reset.push_back(state<token>({token(p[0].index), token(p[2].index)}));

	run_test(g, false);
}

TEST(controlflow, regular_parallel) {
	//  =>t0-->p0-->t1-->*p1-->=  .
	//     \      /  \       /    .
	//      ->p2 /    ->*p3 /     .
	//          X          X      .
	//      ->p4 \    ->*p5 \     .
	//     /      \  /       \    .
	//  =>t2-->p6-->t3-->*p7-->=  .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 8);
	auto t = g.create(transition(), 4);

	g.connect({t[0], p[0], t[1], p[1], t[0]});
	g.connect({t[2], p[6], t[3], p[7], t[2]});

	g.connect({t[0], p[2], t[3], p[5], t[0]});
	g.connect({t[2], p[4], t[1], p[3], t[2]});

	g.reset.push_back(state<token>({token(p[1].index), token(p[3].index), token(p[5].index), token(p[7].index)}));

	run_test(g, false);
}

TEST(controlflow, regular_choice) {
	//  =>p0*->t0-->p1-->t1-->=  .
	//     \      /  \      /    .
	//      ->t2 /    ->t3 /     .
	//          X         X      .
	//      ->t4 \    ->t5 \     .
	//     /      \  /      \    .
	//  =>p2-->t6-->p3-->t7-->=  .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 4);
	auto t = g.create(transition(), 8);

	g.connect({p[0], t[0], p[1], t[1], p[0]});
	g.connect({p[2], t[6], p[3], t[7], p[2]});

	g.connect({p[0], t[2], p[3], t[5], p[0]});
	g.connect({p[2], t[4], p[1], t[3], p[2]});

	g.reset.push_back(state<token>({token(p[0].index)}));
	g.reset.push_back(state<token>({token(p[2].index)}));

	run_test(g, false);
}

TEST(controlflow, nonproper_choice) {
	//     ->t0-->p1-->t1-->p2-->t2-      .
	//    /         \               \     .
	//  p0           ->t6-           >p5  .
	//    \               \         /     .
	//     ->t3-->p3-->t4-->p4-->t5-      .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 6);
	auto t = g.create(transition(), 7);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[2], p[5]});
	g.connect({p[0], t[3], p[3], t[4], p[4], t[5], p[5]});

	g.connect(p[1], t[6]);
	g.connect(t[6], p[4]);

	run_test(g, false);
}

TEST(controlflow, nonproper_parallel) {
	//     ->p0-->t1-->p1-->t2-->p2-      .
	//    /         \               \     .
	//  t0           ->p6-           >t5  .
	//    \               \         /     .
	//     ->p3-->t3-->p4-->t4-->p5-      .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 6);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[5]});
	g.connect({t[0], p[3], t[3], p[4], t[4], p[5], t[5]});

	g.connect(t[1], p[6]);
	g.connect(p[6], t[4]);

	run_test(g, false);
}

TEST(controlflow, shared_parallel) {
	//          ->p1-->t1-->p2-->t2-->p3             .
	//         /                        \            .
	//     ->t0                          ->t5        .
	//    /    \                        /    \       .
	//  p0      ->p4-->t3-->p5-->t4-->p6      ->p10  .
	//    \    /                        \    /       .
	//     ->t6                          ->t9        .
	//         \                        /            .
	//          ->p7-->t7-->p8-->t8-->p9             .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 11);
	auto t = g.create(transition(), 10);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[2], p[3], t[5], p[10]});
	g.connect({t[0], p[4], t[3], p[5], t[4], p[6], t[5]});
	g.connect({p[0], t[6], p[4]});
	g.connect({p[6], t[9], p[10]});
	g.connect({t[6], p[7], t[7], p[8], t[8], p[9], t[9]});

	run_test(g, false);
}

TEST(controlflow, shared_choice) {
	//          ->t1-->p1-->t2-->p2-->t3             .
	//         /                        \            .
	//     ->p0                          ->p5        .
	//    /    \                        /    \       .
	//  t0      ->t4-->p3-->t5-->p4-->t6      ->t10  .
	//    \    /                        \    /       .
	//     ->p6                          ->p9        .
	//         \                        /            .
	//          ->t7-->p7-->t8-->p8-->t9             .

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 10);
	auto t = g.create(transition(), 11);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[2], t[3], p[5], t[10]});
	g.connect({p[0], t[4], p[3], t[5], p[4], t[6], p[5]});
	g.connect({t[0], p[6], t[4]});
	g.connect({t[6], p[9], t[10]});
	g.connect({p[6], t[7], p[7], t[8], p[8], t[9], p[9]});

	run_test(g, false);
}

TEST(controlflow, regular_choice_parallel) {
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

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 7);

	g.connect({p[0], t[0], p[1], t[1], p[2], t[6]});
	g.connect({t[0], p[3], t[2], p[5]});
	g.connect({t[4], p[4], t[3], p[2]});
	g.connect({p[0], t[4], p[6], t[5], p[5], t[6]});

	run_test(g, false);
}

/* This structure violates liveness

TEST(controlflow, regular_parallel_choice) {
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

	graph<place, transition, state<token> > g;

	auto p = g.create(place(), 7);
	auto t = g.create(transition(), 7);

	g.connect({t[0], p[0], t[1], p[1], t[2], p[6]});
	g.connect({p[0], t[3], p[2], t[5]});
	g.connect({p[4], t[4], p[3], t[2]});
	g.connect({t[0], p[4], t[6], p[5], t[5], p[6]});

	run_test(g, false);
}*/

