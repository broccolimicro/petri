#include "path.h"
#include <common/standard.h>

namespace petri
{

path::path(int num_places, int num_transitions)
{
	this->num_places = num_places;
	this->num_transitions = num_transitions;
	hops.resize(num_places+num_transitions);
}

path::~path()
{

}

int path::idx(petri::iterator i) const
{
	return (i.type == petri::transition::type) * num_places + i.index;
}

petri::iterator path::iter(int i) const
{
	petri::iterator j;
	if (i >= num_places) {
		j.type = petri::transition::type;
		j.index = i - num_places;
	} else {
		j.type = petri::place::type;
		j.index = i;
	}
	return j;
}

void path::clear()
{
	hops.clear();
	from.clear();
	to.clear();
}

bool path::is_empty()
{
	for (int i = 0; i < (int)hops.size(); i++)
		if (hops[i] != 0)
			return false;
	return true;
}

vector<petri::iterator> path::maxima()
{
	int n = 0;
	vector<petri::iterator> result;
	for (int i = 0; i < (int)hops.size(); i++) {
		if (hops[i] > n) {
			n = hops[i];
			result.clear();
			result.push_back(iter(i));
		} else if (hops[i] == n) {
			result.push_back(iter(i));
		}
	}
	return result;
}

int path::max()
{
	int m = 0;
	for (int j = 0; j < (int)hops.size(); j++)
		if (hops[j] > m)
			m = hops[j];
	return m;
}

int path::max(int i)
{
	return hops[i];
}

int path::max(vector<int> i)
{
	int m = 0;
	for (int j = 0; j < (int)i.size(); j++)
		if (hops[i[j]] > m)
			m = hops[i[j]];
	return m;
}

path path::mask()
{
	path result(num_places, num_transitions);
	result.from = from;
	result.to = to;
	result.hops.resize(hops.size());
	for (int i = 0; i < (int)hops.size(); i++)
		result.hops[i] = (hops[i] > 0);
	return result;
}

path path::inverse_mask()
{
	path result(num_places, num_transitions);
	result.from = from;
	result.to = to;
	result.hops.resize(hops.size());
	for (int i = 0; i < (int)hops.size(); i++)
		result.hops[i] = (hops[i] <= 0);
	return result;
}

void path::zero(petri::iterator i)
{
	hops[idx(i)] = 0;
}

void path::zero(vector<petri::iterator> i)
{
	for (int j = 0; j < (int)i.size(); j++)
		hops[idx(i[j])] = 0;
}

void path::inc(petri::iterator i, int v)
{
	hops[idx(i)] += v;
}

void path::dec(petri::iterator i, int v)
{
	hops[idx(i)] -= v;
}

void path::set(petri::iterator i, int v)
{
	hops[idx(i)] = v;
}

path &path::operator=(path p)
{
	from = p.from;
	to = p.to;
	hops = p.hops;
	return *this;
}

path &path::operator+=(path p)
{
	from.insert(from.end(), p.from.begin(), p.from.end());
	to.insert(to.end(), p.to.begin(), p.to.end());
	hops.resize(std::max(hops.size(), p.hops.size()), 0);
	for (int i = 0; i < (int)p.hops.size(); i++)
		hops[i] += p.hops[i];
	return *this;
}

path &path::operator-=(path p)
{
	sort(from.begin(), from.end());
	sort(to.begin(), to.end());
	sort(p.from.begin(), p.from.end());
	sort(p.to.begin(), p.to.end());
	from.resize(set_difference(from.begin(), from.end(), p.from.begin(), p.from.end(), from.begin()) - from.begin());
	to.resize(set_difference(to.begin(), to.end(), p.to.begin(), p.to.end(), to.begin()) - to.begin());
	hops.resize(std::max(hops.size(), p.hops.size()), 0);
	for (int i = 0; i < (int)p.hops.size(); i++)
		hops[i] -= p.hops[i];
	return *this;
}

path &path::operator*=(path p)
{
	hops.resize(min(hops.size(), p.hops.size()), 0);
	for (int i = 0; i < (int)hops.size(); i++)
		hops[i] *= p.hops[i];
	return *this;
}

path &path::operator&=(path p)
{
	hops.resize(min(hops.size(), p.hops.size()), 0);
	for (int i = 0; i < (int)hops.size(); i++)
		hops[i] = min(hops[i], p.hops[i]);
	return *this;
}

path &path::operator*=(int n)
{
	for (int i = 0; i < (int)hops.size(); i++)
		hops[i] *= n;
	return *this;
}

path &path::operator/=(int n)
{
	for (int i = 0; i < (int)hops.size(); i++)
		hops[i] /= n;
	return *this;
}

int &path::operator[](petri::iterator i)
{
	return hops[idx(i)];
}

int path::operator[](petri::iterator i) const
{
	return hops[idx(i)];
}

path_set::path_set(int num_places, int num_transitions) : total(num_places, num_transitions)
{

}

path_set::~path_set()
{

}

void path_set::push(path p)
{
	paths.push_back(p);
	total += p;
}

void path_set::clear()
{
	paths.clear();
	total.clear();
}

void path_set::repair()
{
	total.clear();
	for (list<path>::iterator p = paths.begin(); p != paths.end(); p++)
		total += *p;
}

list<path>::iterator path_set::erase(list<path>::iterator i)
{
	total -= *i;
	return paths.erase(i);
}

list<path>::iterator path_set::begin()
{
	return paths.begin();
}

list<path>::iterator path_set::end()
{
	return paths.end();
}

void path_set::zero(petri::iterator i)
{
	for (list<path>::iterator p = paths.begin(); p != paths.end();) {
		p->hops[p->idx(i)] = 0;
		if (p->is_empty()) {
			p = paths.erase(p);
		} else {
			p++;
		}
	}
	total.hops[total.idx(i)] = 0;
}

void path_set::zero(vector<petri::iterator> i)
{
	for (list<path>::iterator p = paths.begin(); p != paths.end();) {
		for (int j = 0; j < (int)i.size(); j++) {
			p->hops[p->idx(i[j])] = 0;
		}

		if (p->is_empty()) {
			p = paths.erase(p);
		} else {
			p++;
		}
	}
	for (int j = 0; j < (int)i.size(); j++) {
		total.hops[total.idx(i[j])] = 0;
	}
}

void path_set::inc(petri::iterator i, int v)
{
	total.hops[total.idx(i)] += paths.size()*v;
	for (list<path>::iterator p = paths.begin(); p != paths.end();) {
		p->hops[p->idx(i)] += v;
		if (p->is_empty()) {
			p = paths.erase(p);
		} else {
			p++;
		}
	}
}

void path_set::dec(petri::iterator i, int v)
{
	total.hops[total.idx(i)] -= paths.size()*v;
	for (list<path>::iterator p = paths.begin(); p != paths.end();) {
		p->hops[p->idx(i)] -= v;
		if (p->is_empty()) {
			p = paths.erase(p);
		} else {
			p++;
		}
	}
}

void path_set::inc(list<path>::iterator i, petri::iterator j, int v)
{
	i->hops[i->idx(j)] += v;
	total.hops[total.idx(j)] += v;
}

void path_set::dec(list<path>::iterator i, petri::iterator j, int v)
{
	i->hops[i->idx(j)] -= v;
	total.hops[total.idx(j)] -= v;
}

path_set path_set::mask()
{
	path_set result(total.num_places, total.num_transitions);
	for (list<path>::iterator p = paths.begin(); p != paths.end(); p++)
		result.push(p->mask());
	return result;
}

path_set path_set::inverse_mask()
{
	path_set result(total.num_places, total.num_transitions);
	for (list<path>::iterator p = paths.begin(); p != paths.end(); p++)
		result.push(p->inverse_mask());
	return result;
}

path_set path_set::coverage(petri::iterator i)
{
	path_set result(total.num_places, total.num_transitions);
	for (list<path>::iterator p = paths.begin(); p != paths.end(); p++)
		if (p->hops[p->idx(i)] != 0)
			result.push(*p);
	return result;
}

path_set path_set::avoidance(petri::iterator i)
{
	path_set result(total.num_places, total.num_transitions);
	for (list<path>::iterator p = paths.begin(); p != paths.end(); p++)
		if (p->hops[p->idx(i)] == 0)
			result.push(*p);
	return result;
}

path_set path_set::avoidance(vector<petri::iterator> i)
{
	path_set result(total.num_places, total.num_transitions);
	for (list<path>::iterator p = paths.begin(); p != paths.end(); p++) {
		bool found = true;
		for (auto j = i.begin(); j != i.end() and found; j++) {
			found = p->hops[p->idx(*j)] == 0;
		}

		if (found) {
			result.push(*p);
		}
	}
	return result;
}

bool path_set::covers(petri::iterator i) const {
	return total[i] > 0;
}

bool path_set::covers(vector<petri::iterator> i) const {
	for (int j = 0; j < (int)i.size(); j++) {
		if (total[i[j]] == 0) {
			return false;
		}
	}
	return true;
}

bool path_set::touches(vector<petri::iterator> i) const {
	for (int j = 0; j < (int)i.size(); j++) {
		if (total[i[j]] > 0) {
			return true;
		}
	}
	return false;
}

// Find all transition groups that cut all of the paths in the path_set.
vector<vector<petri::iterator> > path_set::enumerate() {
	vector<vector<petri::iterator> > result(1, vector<petri::iterator>());
	for (auto i = paths.begin(); i != paths.end(); i++) {
		vector<vector<petri::iterator> > found;
		vector<petri::iterator> not_found;
		for (int j = 0; j < (int)i->hops.size(); j++) {
			if (i->hops[j] > 0) {
				petri::iterator k = i->iter(j);
				bool covered = false;
				for (int l = (int)result.size()-1; l >= 0; l--) {
					if (find(result[l].begin(), result[l].end(), k) != result[l].end()) {
						found.push_back(result[l]);
						result.erase(result.begin() + l);
						covered = true;
					}
				}
				for (auto l = found.begin(); not covered and l != found.end(); l++) {
					covered = find(l->begin(), l->end(), k) != l->end();
				}
				if (not covered) {
					not_found.push_back(k);
				}
			}
		}

		for (auto j = result.begin(); j != result.end(); j++) {
			for (auto k = not_found.begin(); k != not_found.end(); k++) {
				vector<petri::iterator> item = *j;
				item.push_back(*k);
				found.push_back(item);
			}
		}
		result = found;
	}
	return result;
}

path_set &path_set::operator=(const path_set &p)
{
	paths = p.paths;
	total = p.total;
	return *this;
}

path_set &path_set::operator+=(const path_set &p)
{
	paths.insert(paths.end(), p.paths.begin(), p.paths.end());
	total += p.total;
	return *this;
}

path_set &path_set::operator*=(const path &p)
{
	for (list<path>::iterator i = paths.begin(); i != paths.end();)
	{
		*i *= p;
		if (i->is_empty())
			i = paths.erase(i);
		else
			i++;
	}
	total *= p;
	return *this;
}

ostream &operator<<(ostream &os, const path &p)
{
	os << "[" << to_string(p.from) << "-";

	for (int i = 0; i < (int)p.hops.size(); i++) {
		if (p.hops[i] == 1) {
			os << p.iter(i) << " ";
		} else if (p.hops[i] > 1) {
			os << p.iter(i) << ":" << p.hops[i] << " ";
		}
	}

	os << ">" << to_string(p.to) << "]";
	return os;
}

path operator+(path p0, path p1)
{
	p0.from.insert(p0.from.end(), p1.from.begin(), p1.from.end());
	p0.to.insert(p0.to.end(), p1.to.begin(), p1.to.end());
	p0.hops.resize(max(p0.hops.size(), p1.hops.size()), 0);
	for (int i = 0; i < (int)p1.hops.size(); i++)
		p0.hops[i] += p1.hops[i];
	return p0;
}

path operator-(path p0, path p1)
{
	sort(p0.from.begin(), p0.from.end());
	sort(p0.to.begin(), p0.to.end());
	sort(p1.from.begin(), p1.from.end());
	sort(p1.to.begin(), p1.to.end());
	p0.from.resize(set_difference(p0.from.begin(), p0.from.end(), p1.from.begin(), p1.from.end(), p0.from.begin()) - p0.from.begin());
	p0.to.resize(set_difference(p0.to.begin(), p0.to.end(), p1.to.begin(), p1.to.end(), p0.to.begin()) - p0.to.begin());
	p0.hops.resize(max(p0.hops.size(), p1.hops.size()), 0);
	for (int i = 0; i < (int)p1.hops.size(); i++)
		p0.hops[i] -= p1.hops[i];
	return p0;
}

path operator*(path p0, path p1)
{
	p0.hops.resize(min(p0.hops.size(), p1.hops.size()), 0);
	for (int i = 0; i < (int)p0.hops.size(); i++)
		p0.hops[i] *= p1.hops[i];
	return p0;
}

path operator*(path p0, int n)
{
	for (int i = 0; i < (int)p0.hops.size(); i++)
		p0.hops[i] *= n;
	return p0;
}

path operator/(path p0, int n)
{
	for (int i = 0; i < (int)p0.hops.size(); i++)
		p0.hops[i] *= n;
	return p0;
}

ostream &operator<<(ostream &os, const path_set &p)
{
	for (auto i = p.paths.begin(); i != p.paths.end(); i++)
		os << *i << endl;
	return os;
}

path_set operator+(path_set p0, path_set p1)
{
	p0.paths.insert(p0.paths.end(), p1.paths.begin(), p1.paths.end());
	p0.total += p1.total;
	return p0;
}

path_set operator*(path_set p0, path p1)
{
	for (list<path>::iterator i = p0.paths.begin(); i != p0.paths.end();)
	{
		*i *= p1;
		if (i->is_empty())
			i = p0.paths.erase(i);
		else
			i++;
	}
	p0.total *= p1;
	return p0;
}

path_set operator*(path p0, path_set p1)
{
	for (list<path>::iterator i = p1.paths.begin(); i != p1.paths.end();)
	{
		*i *= p0;
		if (i->is_empty())
			i = p1.paths.erase(i);
		else
			i++;
	}
	p1.total *= p0;
	return p1;
}

}

