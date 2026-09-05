#pragma once

#include <common/standard.h>
#include <common/message.h>
#include <common/text.h>

namespace petri
{

struct iterator
{
	iterator();
	iterator(int type, int index);
	~iterator();

	int type;
	int index;

	iterator &operator=(iterator i);
	iterator &operator--();
	iterator &operator++();
	iterator &operator--(int);
	iterator &operator++(int);

	iterator &operator+=(int i);
	iterator &operator-=(int i);

	iterator operator+(int i);
	iterator operator-(int i);

	bool operator==(iterator i) const;
	bool operator!=(iterator i) const;
	bool operator<(iterator i) const;
	bool operator>(iterator i) const;
	bool operator<=(iterator i) const;
	bool operator>=(iterator i) const;

	bool operator==(int i) const;
	bool operator!=(int i) const;
	bool operator<(int i) const;
	bool operator>(int i) const;
	bool operator<=(int i) const;
	bool operator>=(int i) const;

	bool valid() const;

	string to_string() const;
};

ostream &operator<<(ostream &os, iterator i);

// A region is a collection of nodes in the petri net that represent a partial
// state. This means that all of the nodes in the region must be composed in
// parallel with eachother. In this way, a region is intentionally distinct
// from a vector<petri::iterator> which has no constraint about what nodes may
// coexist in the collection.
struct region {
	region();
	region(std::initializer_list<petri::iterator> nodes);
	~region();

	vector<petri::iterator> nodes;

	// DESIGN(edward.bingham) This isn't a constructor because I explicitly want
	// to separate the vector<petri::iterator> type from the bound and the
	// region. Some things just return a flat list of nodes that don't correspond
	// to a "region" of the petri net or a "bound". Those two types have
	// constraints about which nodes can be in the same collection whereas
	// vector<petri::iterator> does not.
	static region from_nodes(vector<petri::iterator> nodes);

	petri::iterator &operator[](int i);
	petri::iterator operator[](int i) const;

	bool empty() const;
	size_t size() const;

	void clear();

	vector<petri::iterator>::iterator begin();
	vector<petri::iterator>::iterator end();
	vector<petri::iterator>::const_iterator begin() const;
	vector<petri::iterator>::const_iterator end() const;

	void sort();
	void rsort();

	void push_back(petri::iterator s);
	petri::iterator pop_back();
	petri::iterator &back();
	petri::iterator back() const;
	void insert(vector<petri::iterator>::iterator at, petri::iterator s);
	void append(region r0);

	vector<petri::iterator>::iterator find(petri::iterator i);
	vector<petri::iterator>::const_iterator find(petri::iterator i) const;

	bool contains(petri::iterator i) const;
	bool contains(region r0) const;

	bool erase(petri::iterator i);
	bool erase(region r0, bool rsorted=false);

	bool remap(region from, region to, bool rsorted=false);

	region &compose(region r0);

	vector<petri::iterator> flat() const;

	bool operator==(region r0) const;
	bool operator!=(region r0) const;
	bool operator<(region r0) const;
	bool operator>(region r0) const;
	bool operator<=(region r0) const;
	bool operator>=(region r0) const;

	string to_string() const;
};

ostream &operator<<(ostream &os, region r0);

// A bound is a collection of regions compose in choice. One may select one
// region or another. There is no constraint about compositions of nodes across
// regions in the collection.
struct bound {
	bound();
	bound(initializer_list<initializer_list<petri::iterator> > regions);
	bound(std::initializer_list<region> regions);
	bound(vector<region> regions);
	~bound();

	vector<region> regions;

	// DESIGN(edward.bingham) This isn't a constructor because I explicitly want
	// to separate the vector<petri::iterator> type from the bound and the
	// region. Some things just return a flat list of nodes that don't correspond
	// to a "region" of the petri net or a "bound". Those two types have
	// constraints about which nodes can be in the same collection whereas
	// vector<petri::iterator> does not.
	static bound from_nodes(vector<petri::iterator> nodes);

	region &operator[](int i);
	region operator[](int i) const;

	bool empty() const;
	size_t size() const;

	void clear();

	vector<region>::iterator begin();
	vector<region>::iterator end();
	vector<region>::const_iterator begin() const;
	vector<region>::const_iterator end() const;

	void push_back(region s);
	region pop_back();
	region &back();
	region back() const;
	void insert(vector<region>::iterator at, region s);
	void append(bound b0);

	bool contains(region r0) const;
	bool contains(bound b0) const;

	bool erase(petri::iterator i);
	bool erase(region r0, bool rsorted=false);

	bool remap(region from, region to, bool rsorted=false);

	bound &compose(int composition, region r0);
	bound &compose(int composition, bound b0);

	vector<petri::iterator> flat() const;

	bool operator==(bound b0) const;
	bool operator!=(bound b0) const;

	string to_string() const;

	bool isOnlyChoice() const;
	bool isOnlyParallel() const;
};

ostream &operator<<(ostream &os, bound b0);

struct segment {
	segment();
	segment(bound source, bound sink);
	~segment();

	bound source;
	bound sink;
	bound reset;

	void clear();

	void erase(petri::iterator i);
	void erase(region r0, bool rsorted=false);

	bool remap(region from, region to, bool rsorted=false);

	void compose(int composition, segment s0);

	bool empty() const;

	string to_string() const;
};

ostream &operator<<(ostream &os, segment s0);

// A strand is a sequence of nodes through a graph. This can also
// be used to represent a cycle given the context. Keep in mind
// that this is a structural sequence, *not* a temporal sequence.
// A temporal sequence would be called a "trace".
struct strand {
	strand();
	strand(std::initializer_list<petri::iterator> nodes);
	~strand();

	vector<petri::iterator> nodes;

	// DESIGN(edward.bingham) This isn't a constructor because I explicitly want
	// to separate the vector<petri::iterator> type from the strand. Some things just return a flat list of nodes that don't correspond
	// to a "strand" of the petri net. Those two types have
	// constraints about which nodes can be in the same collection whereas
	// vector<petri::iterator> does not.
	static strand from_nodes(vector<petri::iterator> nodes);

	petri::iterator &operator[](int i);
	petri::iterator operator[](int i) const;

	bool empty() const;
	size_t size() const;

	void clear();

	vector<petri::iterator>::iterator begin();
	vector<petri::iterator>::iterator end();
	vector<petri::iterator>::const_iterator begin() const;
	vector<petri::iterator>::const_iterator end() const;

	void sort();
	void rsort();

	void push_back(petri::iterator s);
	petri::iterator pop_back();
	petri::iterator &back();
	petri::iterator back() const;
	void insert(vector<petri::iterator>::iterator at, petri::iterator s);
	void append(strand t0);

	vector<petri::iterator>::iterator find(petri::iterator i);
	vector<petri::iterator>::const_iterator find(petri::iterator i) const;

	bool contains(petri::iterator i) const;
	bool contains(strand r0) const;

	bool erase(petri::iterator i);
	bool erase(strand r0, bool rsorted=false);

	bool remap(strand from, strand to, bool rsorted=false);

	strand &compose(strand r0);

	vector<petri::iterator> flat() const;

	bool operator==(strand r0) const;
	bool operator!=(strand r0) const;
	bool operator<(strand r0) const;
	bool operator>(strand r0) const;
	bool operator<=(strand r0) const;
	bool operator>=(strand r0) const;

	string to_string() const;
};

ostream &operator<<(ostream &os, strand r0);

vector<petri::iterator> find_first_shared(const strand &s0, const strand &s1);
vector<petri::iterator> find_last_shared(const strand &s0, const strand &s1);

}
