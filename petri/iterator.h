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

	vector<petri::iterator>::iterator begin();
	vector<petri::iterator>::iterator end();
	vector<petri::iterator>::const_iterator begin() const;
	vector<petri::iterator>::const_iterator end() const;

	void sort();
	void rsort();

	void push_back(petri::iterator s);
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

	bool operator==(region r0) const;
	bool operator!=(region r0) const;
	bool operator<(region r0) const;
	bool operator>(region r0) const;
	bool operator<=(region r0) const;
	bool operator>=(region r0) const;

	string to_string() const;
};

ostream &operator<<(ostream &os, region r0);

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

	vector<region>::iterator begin();
	vector<region>::iterator end();
	vector<region>::const_iterator begin() const;
	vector<region>::const_iterator end() const;
	
	void push_back(region s);
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

	void erase(petri::iterator i);
	void erase(region r0, bool rsorted=false);

	bool remap(region from, region to, bool rsorted=false);

	void compose(int composition, segment s0);

	string to_string() const;
};

ostream &operator<<(ostream &os, segment s0);

struct mapping {
	mapping(bool isIdentity=false);
	mapping(int places, int transitions);
	~mapping();

	array<vector<petri::iterator>, 2> nodes;
	bool isIdentity;

	petri::iterator unmap(petri::iterator node) const;
	petri::iterator map(petri::iterator node) const;
	void identity(int places, int transitions);
	void apply(const mapping &m);

	void set(petri::iterator from, petri::iterator to);
	void set(vector<petri::iterator> from, petri::iterator to);
	bool has(petri::iterator from) const;

	void erase(petri::iterator n);
	void erase(vector<petri::iterator> n, bool rsorted=false);

	mapping reverse() const;
	void reverse_inplace();

	void print() const;	
};

}
