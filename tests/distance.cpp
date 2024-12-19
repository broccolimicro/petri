#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <petri/graph.h>
#include <petri/state.h>

using namespace petri;
using namespace std;

void check_distance(const graph<place, transition, token, state<token> > &g, int distance, petri::iterator from, petri::iterator to) {
	EXPECT_EQ(distance, g.distance(from, to)) << "expected " << from.to_string() << "->" << distance << "->" << to.to_string();
}

TEST(distance, choice) {
	//             ->t0-->p1-->t1-        .
	//            /               \       .
	//  =->t3-->p0                 >p2-=  .
	//            \               /       .
	//             ->t2-----------        .

	graph<place, transition, token, state<token> > g;

	auto p = g.create(place(), 3);
	auto t = g.create(transition(), 4);

	g.connect({t[3], p[0], t[0], p[1], t[1], p[2], t[3]});
	g.connect({p[0], t[2], p[2]});

	g.update_node_distances();

	check_distance(g, 0, t[0], t[0]);
	check_distance(g, 1, t[0], p[1]);
	check_distance(g, 2, t[0], t[1]);
	check_distance(g, 3, t[0], p[2]);
	check_distance(g, 4, t[0], t[3]);
	check_distance(g, 5, t[0], p[0]);
	check_distance(g, 6, t[0], t[2]);

	check_distance(g, 0, p[1], p[1]);
	check_distance(g, 1, p[1], t[1]);
	check_distance(g, 2, p[1], p[2]);
	check_distance(g, 3, p[1], t[3]);
	check_distance(g, 4, p[1], p[0]);
	check_distance(g, 5, p[1], t[2]);
	check_distance(g, 5, p[1], t[0]);

	check_distance(g, 0, t[1], t[1]);
	check_distance(g, 1, t[1], p[2]);
	check_distance(g, 2, t[1], t[3]);
	check_distance(g, 3, t[1], p[0]);
	check_distance(g, 4, t[1], t[2]);
	check_distance(g, 4, t[1], t[0]);
	check_distance(g, 5, t[1], p[1]);

	check_distance(g, 0, p[2], p[2]);
	check_distance(g, 1, p[2], t[3]);
	check_distance(g, 2, p[2], p[0]);
	check_distance(g, 3, p[2], t[2]);
	check_distance(g, 3, p[2], t[0]);
	check_distance(g, 4, p[2], p[1]);
	check_distance(g, 5, p[2], t[1]);

	check_distance(g, 0, t[3], t[3]);
	check_distance(g, 1, t[3], p[0]);
	check_distance(g, 2, t[3], t[2]);
	check_distance(g, 2, t[3], t[0]);
	check_distance(g, 3, t[3], p[1]);
	check_distance(g, 4, t[3], t[1]);
	// TODO(edward.bingham) the current distance algorithm just computes a
	// distance and doesn't handle merges correctly.
	//check_distance(g, 5, t[3], p[2]);

	check_distance(g, 0, p[0], p[0]);
	check_distance(g, 1, p[0], t[2]);
	check_distance(g, 1, p[0], t[0]);
	check_distance(g, 2, p[0], p[1]);
	check_distance(g, 3, p[0], t[1]);
	//check_distance(g, 4, p[0], p[2]);
	//check_distance(g, 5, p[0], t[3]);

	check_distance(g, 0, t[2], t[2]);
	check_distance(g, 1, t[2], p[2]);
	check_distance(g, 2, t[2], t[3]);
	check_distance(g, 3, t[2], p[0]);
	check_distance(g, 4, t[2], t[0]);
	check_distance(g, 5, t[2], p[1]);
	check_distance(g, 6, t[2], t[1]);
}

