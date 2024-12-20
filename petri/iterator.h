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

	string to_string() const;

	static vector<iterator> mark(initializer_list<iterator> n);
	static vector<vector<iterator> > mark(initializer_list<initializer_list<iterator> > n);
};

ostream &operator<<(ostream &os, iterator i);

}
