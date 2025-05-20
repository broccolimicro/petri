#include "iterator.h"
#include "graph.h"
#include <common/message.h>
#include <common/text.h>

namespace petri
{

iterator::iterator()
{
	type = -1;
	index = -1;
}

iterator::iterator(int type, int index)
{
	this->type = type;
	this->index = index;
}

iterator::~iterator()
{

}

iterator &iterator::operator=(iterator i)
{
	type = i.type;
	index = i.index;
	return *this;
}

iterator &iterator::operator--()
{
	index--;
	return *this;
}

iterator &iterator::operator++()
{
	index++;
	return *this;
}

iterator &iterator::operator--(int)
{
	index--;
	return *this;
}

iterator &iterator::operator++(int)
{
	index++;
	return *this;
}

iterator &iterator::operator+=(int i)
{
	index += i;
	return *this;
}

iterator &iterator::operator-=(int i)
{
	index -= i;
	return *this;
}

iterator iterator::operator+(int i)
{
	iterator result(*this);
	result.index += i;
	return result;
}

iterator iterator::operator-(int i)
{
	iterator result(*this);
	result.index -= i;
	return result;
}

bool iterator::operator==(iterator i) const
{
	return (type == i.type && index == i.index);
}

bool iterator::operator!=(iterator i) const
{
	return (type != i.type || index != i.index);
}

bool iterator::operator<(iterator i) const
{
	return (type < i.type ||
		   (type == i.type && index < i.index));
}

bool iterator::operator>(iterator i) const
{
	return (type > i.type ||
		   (type == i.type && index > i.index));
}

bool iterator::operator<=(iterator i) const
{
	return (type < i.type ||
		   (type == i.type && index <= i.index));
}

bool iterator::operator>=(iterator i) const
{
	return (type > i.type ||
		   (type == i.type && index >= i.index));
}

bool iterator::operator==(int i) const
{
	return index == i;
}

bool iterator::operator!=(int i) const
{
	return index != i;
}

bool iterator::operator<(int i) const
{
	return index < i;
}

bool iterator::operator>(int i) const
{
	return index > i;
}

bool iterator::operator<=(int i) const
{
	return index <= i;
}

bool iterator::operator>=(int i) const
{
	return index >= i;
}

bool iterator::valid() const {
	return type >= 0 and index >= 0;
}

string iterator::to_string() const
{
	return (type == place::type ? "P" : "T") + ::to_string(index);
}

ostream &operator<<(ostream &os, iterator i)
{
	os << (i.type == place::type ? "P" : "T") << i.index;
	return os;
}

region::region() {
}

region::region(std::initializer_list<petri::iterator> nodes) : nodes(nodes) {
}

region::~region() {
}

region region::from_nodes(vector<petri::iterator> nodes) {
	region result;
	result.nodes = nodes;
	return result;
}

petri::iterator &region::operator[](int i) {
	return nodes[i];
}

petri::iterator region::operator[](int i) const {
	return nodes[i];
}

bool region::empty() const {
	return nodes.empty();
}

size_t region::size() const {
	return nodes.size();
}

vector<petri::iterator>::iterator region::begin() {
	return nodes.begin();
}

vector<petri::iterator>::iterator region::end() {
	return nodes.end();
}

vector<petri::iterator>::const_iterator region::begin() const {
	return nodes.begin();
}

vector<petri::iterator>::const_iterator region::end() const {
	return nodes.end();
}

void region::sort() {
	std::sort(nodes.begin(), nodes.end());
	nodes.erase(unique(nodes.begin(), nodes.end()), nodes.end());
}

void region::rsort() {
	std::sort(nodes.begin(), nodes.end());
	nodes.erase(unique(nodes.begin(), nodes.end()), nodes.end());
	reverse(nodes.begin(), nodes.end());
}

void region::push_back(petri::iterator s) {
	nodes.push_back(s);
}

petri::iterator &region::back() {
	return nodes.back();
}

petri::iterator region::back() const {
	return nodes.back();
}

void region::insert(vector<petri::iterator>::iterator at, petri::iterator s) {
	nodes.insert(at, s);
}

void region::append(region r0) {
	nodes.insert(nodes.end(), r0.begin(), r0.end());
}

vector<petri::iterator>::iterator region::find(petri::iterator i) {
	return std::find(nodes.begin(), nodes.end(), i);
}

vector<petri::iterator>::const_iterator region::find(petri::iterator i) const {
	return std::find(nodes.begin(), nodes.end(), i);
}

bool region::contains(petri::iterator i) const {
	return std::find(nodes.begin(), nodes.end(), i) != nodes.end();
}

bool region::contains(region r0) const {
	for (auto i = r0.begin(); i != r0.end(); i++) {
		if (not contains(*i)) {
			return false;
		}
	}
	return true;
}

bool region::erase(petri::iterator i) {
	bool found = false;
	for (int j = (int)nodes.size()-1; j >= 0; j--) {
		if (nodes[j].type == i.type and nodes[j].index > i.index) {
			nodes[j].index--;
		} else if (nodes[j] == i) {
			nodes.erase(nodes.begin()+j);
			found = true;
		}
	}
	return found;
}

bool region::erase(region r0, bool rsorted) {
	if (not rsorted) {
		r0.rsort();
	}

	bool found = false;
	for (auto j = r0.begin(); j != r0.end(); j++) {
		found = erase(*j) or found;
	}
	return found;
}

bool region::remap(region from, region to, bool rsorted) {
	if (erase(from, rsorted)) {
		compose(to);
		return true;
	}
	return false;
}

region &region::compose(region r0) {
	nodes.insert(nodes.end(), r0.begin(), r0.end());
	std::sort(nodes.begin(), nodes.end());
	nodes.erase(unique(nodes.begin(), nodes.end()));
	return *this;
}

bool region::operator==(region r0) const {
	if (nodes.size() != r0.nodes.size()) {
		return false;
	}
	for (int i = 0; i < (int)nodes.size(); i++) {
		if (nodes[i] != r0.nodes[i]) {
			return false;
		}
	}
	return true;
}

bool region::operator!=(region r0) const {
	return not (*this == r0);
}

bool region::operator<(region r0) const {
	int m = (int)min(nodes.size(), r0.nodes.size());
	for (int i = 0; i < m; i++) {
		if (nodes[i] != r0.nodes[i]) {
			return nodes[i] < r0.nodes[i];
		}
	}
	return (nodes.size() < r0.nodes.size());
}

bool region::operator>(region r0) const {
	return not (*this <= r0);
}

bool region::operator<=(region r0) const {
	int m = (int)min(nodes.size(), r0.nodes.size());
	for (int i = 0; i < m; i++) {
		if (nodes[i] != r0.nodes[i]) {
			return nodes[i] < r0.nodes[i];
		}
	}
	return (nodes.size() <= r0.nodes.size());
}

bool region::operator>=(region r0) const {
	return not (*this < r0);
}

string region::to_string() const {
	string result = "(";
	for (auto i = nodes.begin(); i != nodes.end(); i++) {
		if (i != nodes.begin()) {
			result += ", ";
		}
		result += i->to_string();
	}
	result += ")";
	return result;
}

ostream &operator<<(ostream &os, region r0) {
	os << "(";
	for (auto i = r0.nodes.begin(); i != r0.nodes.end(); i++) {
		if (i != r0.nodes.begin()) {
			os << ", ";
		}
		os << *i;
	}
	os << ")";
	return os;
}

bound::bound() {
}

bound::bound(std::initializer_list<std::initializer_list<petri::iterator> > regions) {
	for (auto i = regions.begin(); i != regions.end(); i++) {
		this->regions.push_back(region(*i));
	}
}

bound::bound(std::initializer_list<region> regions) : regions(regions) {
}

bound::bound(vector<region> regions) : regions(regions) {
}

bound::~bound() {
}

bound bound::from_nodes(vector<petri::iterator> nodes) {
	bound result;
	for (auto i = nodes.begin(); i != nodes.end(); i++) {
		result.regions.push_back(region({*i}));
	}
	return result;
}

region &bound::operator[](int i) {
	return regions[i];
}

region bound::operator[](int i) const {
	return regions[i];
}

bool bound::empty() const {
	return regions.empty();
}

size_t bound::size() const {
	return regions.size();
}

vector<region>::iterator bound::begin() {
	return regions.begin();
}

vector<region>::iterator bound::end() {
	return regions.end();
}

vector<region>::const_iterator bound::begin() const {
	return regions.begin();
}

vector<region>::const_iterator bound::end() const {
	return regions.end();
}

void bound::push_back(region s) {
	regions.push_back(s);
}

region &bound::back() {
	return regions.back();
}

region bound::back() const {
	return regions.back();
}

void bound::insert(vector<region>::iterator at, region s) {
	regions.insert(at, s);
}

void bound::append(bound b0) {
	regions.insert(regions.end(), b0.begin(), b0.end());
}

bool bound::contains(region r0) const {
	for (auto i = regions.begin(); i != regions.end(); i++) {
		if (i->contains(r0)) {
			return true;
		}
	}
	return false;
}

bool bound::contains(bound b0) const {
	for (auto i = b0.begin(); i != b0.end(); i++) {
		if (not contains(*i)) {
			return false;
		}
	}
	return true;
}

bool bound::erase(petri::iterator i) {
	bool found = false;
	for (auto j = regions.begin(); j != regions.end(); j++) {
		found = j->erase(i) or found;
	}
	return found;
}

bool bound::erase(region r0, bool rsorted) {
	if (not rsorted) {
		r0.rsort();
	}

	bool found = false;
	for (auto j = regions.begin(); j != regions.end(); j++) {
		found = j->erase(r0, true) or found;
	}
	return found;
}

bool bound::remap(region from, region to, bool rsorted) {
	if (not rsorted) {
		from.rsort();
	}

	bool found = false;
	for (auto j = regions.begin(); j != regions.end(); j++) {
		found = j->remap(from, to, true) or found;
	}
	return found;
}

bound &bound::compose(int composition, region r0) {
	if (regions.empty()) {
		regions.push_back(r0);
	} else if (composition == choice) {
		if (not contains(r0)) {
			for (auto j = regions.begin(); j != regions.end(); j++) {
				if (r0.contains(*j)) {
					*j = r0;
					return *this;
				}
			}
			regions.push_back(r0);
		}
	} else if (composition == parallel) {
		for (auto i = regions.begin(); i != regions.end(); i++) {
			i->compose(r0);
		}
	}
	return *this;
}

bound &bound::compose(int composition, bound b0) {
	if (regions.empty()) {
		regions = b0.regions;
	} else if (b0.regions.empty()) {
		// skip
	} else if (composition == choice) {
		for (auto i = b0.begin(); i != b0.end(); i++) {
			compose(composition, *i);
		}
	} else if (composition == parallel) {
		bound b1;
		for (auto i = regions.begin(); i != regions.end(); i++) {
			for (auto j = b0.begin(); j != b0.end(); j++) {
				region r0 = *i;
				r0.compose(*j);
				b1.compose(choice, r0);
			}
		}
		regions.swap(b1.regions);
	}
	return *this;
}

bool bound::operator==(bound b0) const {
	if (regions.size() != b0.regions.size()) {
		return false;
	}
	for (int i = 0; i < (int)regions.size(); i++) {
		if (regions[i] != b0.regions[i]) {
			return false;
		}
	}
	return true;
}

bool bound::operator!=(bound b0) const {
	return not (*this == b0);
}

string bound::to_string() const {
	string result = "[";
	for (auto i = regions.begin(); i != regions.end(); i++) {
		if (i != regions.begin()) {
			result += ":";
		}
		result += i->to_string();
	}
	result += "]";
	return result;
}

bool bound::isOnlyChoice() const {
	for (auto i = regions.begin(); i != regions.end(); i++) {
		if (i->nodes.size() != 1u) {
			return false;
		}
	}
	return true;
}

bool bound::isOnlyParallel() const {
	return regions.size() == 1u;
}

ostream &operator<<(ostream &os, bound b0) {
	os << "[";
	for (auto i = b0.regions.begin(); i != b0.regions.end(); i++) {
		if (i != b0.regions.begin()) {
			os << ":";
		}
		os << *i;
	}
	os << "]";
	return os;
}

segment::segment() {
}

segment::segment(bound source, bound sink) : source(source), sink(sink) {
}

segment::~segment() {
}

void segment::erase(petri::iterator i) {
	source.erase(i);
	sink.erase(i);
}

void segment::erase(region r0, bool rsorted) {
	if (not rsorted) {
		r0.rsort();
	}

	source.erase(r0, true);
	sink.erase(r0, true);
}

bool segment::remap(region from, region to, bool rsorted) {
	if (not rsorted) {
		from.rsort();
	}

	bool found = false;
	found = source.remap(from, to, true) or found;
	found = sink.remap(from, to, true) or found;
	return found;
}

void segment::compose(int composition, segment s0) {
	source.compose(composition, s0.source);
	sink.compose(composition, s0.sink);
}

string segment::to_string() const {
	return source.to_string() + "..." + sink.to_string();
}

ostream &operator<<(ostream &os, segment s0) {
	os << s0.source << "..." << s0.sink;
	return os;
}

mapping::mapping(bool isIdentity) {
	this->isIdentity = isIdentity;
}

mapping::mapping(int places, int transitions) {
	identity(places, transitions);
}

mapping::~mapping() {
}

petri::iterator mapping::unmap(petri::iterator node) const {
	if (isIdentity) {
		return node;
	}

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < (int)nodes[i].size(); j++) {
			if (nodes[i][j] == node) {
				return petri::iterator(i, j);
			}
		}
	}
	return petri::iterator();
}

petri::iterator mapping::map(petri::iterator node) const {
	if (isIdentity) {
		return node;
	}

	if ((node.type == place::type or node.type == transition::type) and node.index >= 0 and node.index < (int)nodes[node.type].size()) {
		return nodes[node.type][node.index];
	}
	return petri::iterator();
}

void mapping::identity(int places, int transitions) {
	nodes[place::type].clear();
	nodes[transition::type].clear();
	isIdentity = false;

	nodes[place::type].reserve(places);
	nodes[transition::type].reserve(transitions);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < (int)nodes[i].size(); j++) {
			nodes[i].push_back(petri::iterator(i, j));
		}
	}
}

void mapping::apply(const mapping &m) {
	if (isIdentity) {
		isIdentity = m.isIdentity;
		nodes = m.nodes;
	} else if (m.isIdentity) {
		return;
	}

	array<vector<petri::iterator>, 2> updated;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < (int)m.nodes[i].size(); j++) {
			updated[i].push_back(map(m.map(petri::iterator(i, j))));
		}
	}
	nodes = updated;
}

void mapping::set(petri::iterator from, petri::iterator to) {
	if (isIdentity) {
		printf("error: set() not supported for identity mapping\n");
		return;
	}

	if (from.index >= (int)nodes[from.type].size()) {
		nodes[from.type].resize(from.index+1, petri::iterator());
	}
	if (from.index >= 0) {
		nodes[from.type][from.index] = to;
	}
}

void mapping::set(vector<petri::iterator> from, petri::iterator to) {
	if (isIdentity) {
		printf("error: set() not supported for identity mapping\n");
		return;
	}

	for (int i = 0; i < 2; i++) {
		int m = (int)nodes[i].size()-1;
		for (int j = 0; j < (int)from.size(); j++) {
			if (from[j].type == i and from[j].index > m) {
				m = from[j].index;
			}
		}
		nodes[i].resize(m+1, petri::iterator());
	}
	for (int i = 0; i < (int)from.size(); i++) {
		if ((from[i].type == place::type or from[i].type == transition::type) and from[i].index >= 0) {
			nodes[from[i].type][from[i].index] = to;
		}
	}
}

bool mapping::has(petri::iterator from) const {
	return (isIdentity and from.valid()) or ((from.type == place::type or from.type == transition::type) and from.index >= 0 and from.index < (int)nodes[from.type].size() and nodes[from.type][from.index].valid());
}

void mapping::erase(petri::iterator n) {
	if (isIdentity) {
		printf("error: erase() not supported for identity mapping\n");
		return;
	}

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < (int)nodes[i].size(); j++) {
			if (nodes[i][j].type == n.type and nodes[i][j].index > n.index) {
				nodes[i][j].index--;
			} else if (nodes[i][j] == n) {
				nodes[i][j] = petri::iterator();
			}
		}
	}
}

void mapping::erase(vector<petri::iterator> n, bool rsorted) {
	if (isIdentity) {
		printf("error: erase() not supported for identity mapping\n");
		return;
	}

	if (not rsorted) {
		sort(n.begin(), n.end());
		n.erase(unique(n.begin(), n.end()), n.end());
		std::reverse(n.begin(), n.end());
	}

	for (auto i = n.begin(); i != n.end(); i++) {
		erase(*i);
	}
}

mapping mapping::reverse() const {
	if (isIdentity) {
		// reverse of identity is identity
		return *this;
	}

	mapping result = *this;
	result.reverse_inplace();	
	return result;
}

void mapping::reverse_inplace() {
	if (isIdentity) {
		// reverse of identity is identity
		return;
	}

	array<int, 2> hi = {0,0};
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < (int)nodes.size(); j++) {
			if (nodes[i][j].index > hi[nodes[i][j].type]) {
				hi[nodes[i][j].type] = nodes[i][j].index;
			}
		}
	}
	
	array<vector<petri::iterator>, 2> updated;
	updated[0].resize(hi[0]+1, petri::iterator());
	updated[1].resize(hi[1]+1, petri::iterator());
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < (int)nodes.size(); j++) {
			if (nodes[i][j].valid()) {
				updated[nodes[i][j].type][nodes[i][j].index] = petri::iterator(i, j);
			}
		}
	}
	nodes = updated;
}

void mapping::print() const {
	printf("map{%s", (isIdentity ? "identity" : ""));
	bool first = true;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < (int)nodes[i].size(); j++) {
			if (first) {
				first = false;
			} else {
				printf(", ");
			}
			printf("%s -> %s", petri::iterator(i, j).to_string().c_str(), nodes[i][j].to_string().c_str());
		}
	}
	printf("}\n");
}

}

