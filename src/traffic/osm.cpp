/// MIT License
/// 
/// Copyright (c) 2020 Konstantin Rolf
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
/// 
/// Written by Konstantin Rolf (konstantin.rolf@gmail.com)
/// July 2020

#include "engine.h"

#include <math.h>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include "osm.h"

using namespace std;
using namespace traffic;

const vector<size_t> emptyVec;

// ---- OSM object ---- //

OSMMapObject::OSMMapObject() { }
OSMMapObject::OSMMapObject(int64_t id, int32_t version)
	: id(id), version(version) { }
OSMMapObject::OSMMapObject(
	int64_t id, int32_t version,
	shared_ptr<vector<pair<string, string>>> tags
) : tags(tags), id(id), version(version) { }

OSMMapObject::OSMMapObject(const json& json)
{
	json.at("id").get_to(id);
	json.at("version").get_to(version);
	tags = make_shared<vector<pair<string, string>>>(json.at("tags")
		.get<vector<pair<string, string>>>());
}

size_t OSMMapObject::getSize() const {
	return sizeof(*this) + getManagedSize();
}

size_t OSMMapObject::getManagedSize() const
{
	size_t totalSize = 0;
	if (tags) {
		totalSize += sizeof(*tags);
		totalSize += tags->capacity() * sizeof(pair<string, string>);
		std::for_each(tags->begin(), tags->end(),
			[&](const auto& pair) {
				totalSize += pair.first.size() + pair.second.size();
			}
		);
	}
	return totalSize;
}

void OSMMapObject::toJson(json& json) const
{
	json["id"] = id;
	json["version"] = version;
	json["tags"] = *tags;
}

shared_ptr<vector<pair<string, string>>> OSMMapObject::getData() const noexcept  { return tags; }
int64_t OSMMapObject::getID() const noexcept { return id; }
int32_t OSMMapObject::getVer() const noexcept  { return version; }

bool OSMMapObject::hasTag(const string& key) const noexcept
{
	if (tags) {
		for (const pair<string, string>& vecKey : (*tags)) {
			if (vecKey.first == key) return true;
		}
	}
	return false;
}

bool OSMMapObject::hasTagValue(const string& key, const string& value) const noexcept
{
	if (tags) {
		for (const pair<string, string>& vecKey : (*tags)) {
			if (vecKey.first == key && vecKey.second == value) return true;
		}
	}
	return false;
}

string OSMMapObject::getValue(const string& key) const
{
	if (tags) {
		for (const pair<string, string>& vecKey : (*tags)) {
			if (vecKey.first == key) return vecKey.second;
		}
	}
	throw runtime_error("could not find key " + key);
}



// ---- OSMNode ---- //

OSMNode::OSMNode()
	: lat(0.0), lon(0.0) { }
OSMNode::OSMNode(int64_t id, int32_t ver, float lat, float lon)
	: OSMMapObject(id, ver), lat(lat), lon(lon) { }
OSMNode::OSMNode(int64_t id, int32_t ver,
	shared_ptr<vector<pair<string, string>>> tags,
	float lat, float lon)
	: OSMMapObject(id, ver, tags), lat(lat), lon(lon) { }
OSMNode::OSMNode(const json& json)
	: OSMMapObject(json) {
	json.at("lat").get_to(lat);
	json.at("lon").get_to(lon);
}

size_t OSMNode::getManagedSize() const { return OSMMapObject::getManagedSize(); }
size_t OSMNode::getSize() const { return getManagedSize() + sizeof(*this); }

glm::vec2 OSMNode::asVector() const { return glm::vec2(lon, lat); }

prec_t OSMNode::getLat() const { return lat; }
prec_t OSMNode::getLon() const { return lon; }

void OSMNode::toJson(json& json) const {
	OSMMapObject::toJson(json);
	json["lat"] = lat;
	json["lon"] = lon;
}

// ---- OSMWay ---- //

traffic::OSMWay::OSMWay() : subIndex(0) { }

OSMWay::OSMWay(int64_t id, int32_t version,
	shared_ptr<vector<int64_t>>&& pnodes)
	: OSMMapObject(id, version), nodes(pnodes), subIndex(0) { }

OSMWay::OSMWay(int64_t id, int32_t ver,
	shared_ptr<vector<int64_t>>&& nodes_,
	shared_ptr<vector<pair<string, string>>> tags
) : OSMMapObject(id, ver, tags), nodes(nodes_), subIndex(0) { }

OSMWay::OSMWay(const json& json)
	: OSMMapObject(json) {
	nodes = make_shared<vector<int64_t>>();
	json.at("nodes").get_to<vector<int64_t>>(*nodes);
	json.at("subIndex").get_to<int32_t>(subIndex);
}

void OSMWay::toJson(json& json) const {
	OSMMapObject::toJson(json);
	json["nodes"] = *nodes;
	json["subIndex"] = subIndex;
}

size_t OSMWay::getManagedSize() const {
	size_t size = OSMMapObject::getManagedSize();
	size += sizeof(*nodes) + nodes->capacity() * sizeof(int64_t);
	return size;
}

size_t OSMWay::getSize() const {
	return getManagedSize() + sizeof(*this);
}

void traffic::OSMWay::clear() noexcept
{
	if (nodes) nodes->clear();
}

void traffic::OSMWay::addNode(int64_t node) noexcept
{
	if (nodes) nodes->push_back(node);
}

int32_t traffic::OSMWay::getSubIndex() const { return subIndex; }

void traffic::OSMWay::setSubIndex(int32_t subIndex) { this->subIndex = subIndex; }
vector<int64_t>& OSMWay::getNodes() { return *nodes; }
const vector<int64_t>& OSMWay::getNodes() const { return *nodes; }

// ---- NodeRef ---- //

NodeRef::NodeRef(float value, size_t index)
	: value_(value), index_(index) { }

// ---- OSMRelation ---- //


OSMRelation::OSMRelation() : subIndex(0) { } // TODO initialize
OSMRelation::OSMRelation(
	int64_t id, int32_t ver,
	shared_ptr<vector<RelationMember>> nodes,
	shared_ptr<vector<RelationMember>> ways,
	shared_ptr<vector<RelationMember>> relations
) : OSMMapObject(id, ver), nodes(nodes),
	ways(ways), relations(relations), subIndex(0) { }

OSMRelation::OSMRelation(
	int64_t id, int32_t ver,
	shared_ptr<vector<pair<string, string>>> tags,
	shared_ptr<vector<RelationMember>> nodes,
	shared_ptr<vector<RelationMember>> ways,
	shared_ptr<vector<RelationMember>> relations
) : OSMMapObject(id, ver, tags), nodes(nodes),
	ways(ways), relations(relations), subIndex(0) { }

OSMRelation::OSMRelation(const json& json)
{
	nodes = make_shared<vector<RelationMember>>(json.at("nodes").get<vector<RelationMember>>());
	ways = make_shared<vector<RelationMember>>(json.at("ways").get<vector<RelationMember>>());
	relations = make_shared<vector<RelationMember>>(json.at("relations").get<vector<RelationMember>>());
	json.at("subIndex").get_to<int32_t>(subIndex);
}

size_t OSMRelation::getManagedSize() const {
	size_t size = OSMMapObject::getManagedSize();
	size += sizeof(*nodes) + nodes->capacity() * sizeof(RelationMember);
	size += sizeof(*ways) + ways->capacity() * sizeof(RelationMember);
	size += sizeof(*relations) + relations->capacity() * sizeof(RelationMember);

	for_each(nodes->begin(), nodes->end(),
		[&](RelationMember mem) { size += mem.getSize(); });
	for_each(ways->begin(), ways->end(),
		[&](RelationMember mem) { size += mem.getSize(); });
	for_each(relations->begin(), relations->end(),
		[&](RelationMember mem) { size += mem.getSize(); });
	return size;
}
size_t OSMRelation::getSize() const {
	return getManagedSize() + sizeof(*this);
}


void OSMRelation::toJson(json& json) const {
	json["nodes"] = *nodes;
	json["ways"] = *ways;
	json["relations"] = *relations;
	json["subIndex"] = subIndex;
}

shared_ptr<vector<RelationMember>> OSMRelation::getNodes() const { return nodes; }
shared_ptr<vector<RelationMember>> OSMRelation::getWays() const { return ways; }
shared_ptr<vector<RelationMember>> OSMRelation::getRelations() const { return relations; }

RelationMember::RelationMember() { }
RelationMember::RelationMember(
	int64_t index_, const string& type_
) : index(index_), type(type_) { }

RelationMember::RelationMember(const json& json)
{
	json.at("index").get_to(index);
	json.at("role").get_to(type);
}

size_t RelationMember::getManagedSize() const { return type.size(); }
size_t RelationMember::getSize() const { return getManagedSize() + sizeof(*this); }

void RelationMember::toJson(json& json) const {
	json["index"] = index;
	json["role"] = type;
}

int64_t RelationMember::getIndex() const { return index; }
string& RelationMember::getType() { return type; }
const string& RelationMember::getType() const { return type; }

int32_t traffic::OSMRelation::getSubIndex() const { return subIndex; }
void traffic::OSMRelation::setSubIndex(int32_t subIndex) { this->subIndex = subIndex; }

// ---- OSMMap ---- //

OSMSegment::OSMSegment() {
	nodeList = make_shared<vector<OSMNode>>();
	wayList = make_shared<vector<OSMWay>>();
	relationList = make_shared<vector<OSMRelation>>();

	nodeMap = make_shared<map_t>();
	wayMap = make_shared<mapid_t<vector<size_t>>>();
	relationMap = make_shared<mapid_t<vector<size_t>>>();

	recalculateBoundaries();
}

traffic::OSMSegment::OSMSegment(const Rect& rect)
{
	setBoundingBox(rect);
}

traffic::OSMSegment::OSMSegment(
	const listnode_ptr_t& nodes,
	const listway_ptr_t& ways,
	const listrelation_ptr_t& relations)
{
	nodeList = nodes;
	wayList = ways;
	relationList = relations;
	reindexMap(true);
	recalculateBoundaries();
}

OSMSegment::OSMSegment(
	const listnode_ptr_t& nodes,
	const listway_ptr_t& ways,
	const listrelation_ptr_t& relations,
	const shared_ptr<map_t>& pNodeMap,
	const shared_ptr<mapid_t<vector<size_t>>>& pWayMap,
	const shared_ptr<mapid_t<vector<size_t>>>& pRelationMap)
{
	nodeList = nodes;
	wayList = ways;
	relationList = relations;

	nodeMap = pNodeMap;
	wayMap = pWayMap;
	relationMap = pRelationMap;

	recalculateBoundaries();
}

OSMSegment::OSMSegment(const json& json)
{
	nodeList = make_shared<vector<OSMNode>>(json.at("nodes").get<vector<OSMNode>>());
	wayList = make_shared<vector<OSMWay>>(json.at("ways").get<vector<OSMWay>>());
	relationList = make_shared<vector<OSMRelation>>(json.at("relations").get<vector<OSMRelation>>());
	reindexMap(true);
	recalculateBoundaries();
}

traffic::OSMSegment::~OSMSegment()
{
}

void traffic::OSMSegment::reindexMap(bool merge)
{
	if (!nodeMap) nodeMap = make_shared<map_t>();
	if (!wayMap) wayMap = make_shared<mapid_t<vector<size_t>>>();
	if (!relationMap) relationMap = make_shared<mapid_t<vector<size_t>>>();

	if (!merge) {
		nodeMap->clear();
		wayMap->clear();
		relationMap->clear();
	}

	nodeMap->reserve(nodeMap->size() + nodeList->size());
	wayMap->reserve(wayMap->size() + wayList->size());
	relationMap->reserve(relationMap->size() + relationList->size());

	for (size_t i = 0; i < nodeList->size(); i++)
		(*nodeMap)[(*nodeList)[i].getID()] = i;
	for (size_t i = 0; i < wayList->size(); i++)
		(*wayMap)[(*wayList)[i].getID()].push_back(i);
	for (size_t i = 0; i < relationList->size(); i++)
		(*relationMap)[(*relationList)[i].getID()].push_back(i);
}

void OSMSegment::recalculateBoundaries() {
	if (nodeList->empty()) {
		lowerLat = -90.0;
		lowerLon = -180.0;
		upperLat = 90.0;
		upperLon = 180.0;
	}
	else {
		float latMax = numeric_limits<float>::min();
		float latMin = numeric_limits<float>::max();
		float lonMax = numeric_limits<float>::min();
		float lonMin = numeric_limits<float>::max();
		for (const auto& nd : *nodeList) {
			if (nd.getLat() > latMax) latMax = nd.getLat();
			else if (nd.getLat() < latMin) latMin = nd.getLat();
			if (nd.getLon() > lonMax) lonMax = nd.getLon();
			else if (nd.getLon() < lonMin) lonMin = nd.getLon();
		}
		lowerLat = latMin;
		upperLat = latMax;
		lowerLon = lonMin;
		upperLon = lonMax;
	}
}


void OSMSegment::toJson(json& json) const
{
	json["nodes"] = *nodeList;
	json["ways"] = *wayList;
	json["relations"] = *relationList;
}


bool OSMSegment::hasNodes() const noexcept { return nodeList && !nodeList->empty(); }
bool OSMSegment::hasWays() const noexcept { return wayList && !wayList->empty(); }
bool OSMSegment::hasRelations() const noexcept { return relationList && !relationList->empty(); }
bool OSMSegment::empty() const noexcept { return !hasNodes() && !hasWays() && !hasRelations(); }

template<typename Type>
unordered_map<string, int32_t> createTTagList(const Type& data, unordered_map<string, int32_t>& map) {
	for (const OSMMapObject& nd : data) {
		for (const pair<string, string> tag : (*nd.getData())) {
			auto it = map.find(tag.first);
			if (it == map.end()) {
				map[tag.first] = 1;
			}
			else {
				map[tag.first]++;
			}
		}
	}
	return map;
}

vector<int64_t> OSMSegment::findAdress(
	const string& city, const string& postcode,
	const string& street, const string& housenumber
) const {
	vector<int64_t> nodes;
	for (const OSMNode& nd : (*nodeList)) {
		if ((city.empty() || nd.hasTagValue("addr:city", city)) &&
			(postcode.empty() || nd.hasTagValue("addr:postcode", postcode)) &&
			(street.empty() || nd.hasTagValue("addr:street", street)) &&
			(housenumber.empty() || nd.hasTagValue("addr:housenumber", housenumber))) {
			nodes.push_back(nd.getID());
		}
	}
	return nodes;
}

unordered_map<string, int32_t> OSMSegment::createNodeTagList() const
{
	unordered_map<string, int32_t> map;
	createTTagList(*nodeList, map);
	return map;
}

unordered_map<string, int32_t> OSMSegment::createWayTagList() const
{
	unordered_map<string, int32_t> map;
	createTTagList(*wayList, map);
	return map;
}

unordered_map<string, int32_t> OSMSegment::createTagList() const
{
	unordered_map<string, int32_t> map;
	createTTagList(*nodeList, map);
	createTTagList(*wayList, map);
	return map;
}

size_t OSMSegment::getNodeIndex(int64_t id) const {
	auto it = nodeMap->find(id);
	return it == nodeMap->end() ? numeric_limits<size_t>::max() : it->second;
}
size_t OSMSegment::getWayIndex(int64_t id) const {
	auto it = wayMap->find(id);
	return it == wayMap->end() ? numeric_limits<size_t>::max() : it->second.front();
}
size_t OSMSegment::getRelationIndex(int64_t id) const {
	auto it = relationMap->find(id);
	return it == relationMap->end() ? numeric_limits<size_t>::max() : it->second.front();
}

const std::vector<size_t>& traffic::OSMSegment::getWayIndices(int64_t id) const
{
	auto it = wayMap->find(id);
	return it == wayMap->end() ? emptyVec : it->second;
}

const std::vector<size_t>& traffic::OSMSegment::getRelationIndices(int64_t id) const
{
	auto it = relationMap->find(id);
	return it == relationMap->end() ? emptyVec : it->second;
}

bool OSMSegment::hasNodeIndex(int64_t id) const { return nodeMap->find(id) != nodeMap->end(); }
bool OSMSegment::hasWayIndex(int64_t id) const { return wayMap->find(id) != wayMap->end(); }
bool OSMSegment::hasRelationIndex(int64_t id) const { return relationMap->find(id) != relationMap->end(); }

bool OSMSegment::addNode(const OSMNode& nd, bool updateBoundaries)
{
	auto it = nodeMap->find(nd.getID());
	if (it == nodeMap->end()) return false;
	(*nodeMap)[nd.getID()] = nodeList->size();
	nodeList->push_back(nd);
	if (updateBoundaries) {
		if (nd.getLat() < lowerLat) lowerLat = nd.getLat();
		else if (nd.getLat() > upperLat) upperLat = nd.getLat();
		if (nd.getLon() < lowerLon) lowerLon = nd.getLon();
		else if (nd.getLon() > upperLon) upperLon = nd.getLon();
	}
	return true;
}

bool OSMSegment::addWay(const OSMWay& wd) {
	auto it = wayMap->find(wd.getID());
	if (it != wayMap->end()) {
		// compares and checks if the batch already contains this way
		for (const size_t wayIndex : it->second) {
			if ((*wayList)[wayIndex].getID() == wd.getID() &&
				(*wayList)[wayIndex].getSubIndex() == wd.getSubIndex()) {
				return false;
			}
		}
	}
	(*wayMap)[wd.getID()].push_back(wayList->size());
	wayList->push_back(wd);
	return true;
}

bool OSMSegment::addRelation(const OSMRelation& re) {
	auto it = relationMap->find(re.getID());
	if (it != relationMap->end()) {
		// compares and checks if the batch already contains this way
		for (const size_t rlIndex : it->second) {
			if ((*relationList)[rlIndex].getID() == re.getID() &&
				(*relationList)[rlIndex].getSubIndex() == re.getSubIndex()) {
				return false;
			}
		}
	}
	(*relationMap)[re.getID()].push_back(relationList->size());
	relationList->push_back(re);
	return true;
}

bool traffic::OSMSegment::addWayRecursive(const OSMWay& wd, const OSMSegment& lookup, bool updateBounds)
{
	if (!addWay(wd)) return false;
	for (int64_t id : wd.getNodes()) {
		size_t nodeID = lookup.getNodeIndex(id);
		if (nodeID == numeric_limits<size_t>::max()) {
			continue;
		}
		addNode(lookup.getNode(id), updateBounds);
	}
	return true;
}

bool traffic::OSMSegment::addRelationRecursive(const OSMRelation& re, const OSMSegment& lookup, bool updateBounds)
{
	if (!addRelation(re)) return false;
	for (RelationMember node : (*re.getNodes()))
		addNode(lookup.getNode(node.getIndex()), updateBounds);
	for (RelationMember way : (*re.getWays()))
		addWayRecursive(lookup.getWay(way.getIndex()), lookup, updateBounds);
	for (RelationMember r : (*re.getRelations()))
		addRelationRecursive(lookup.getRelation(r.getIndex()), lookup, updateBounds);
	return true;
}

const OSMNode& OSMSegment::getNode(int64_t id) const { return (*nodeList)[getNodeIndex(id)]; }
const OSMWay& OSMSegment::getWay(int64_t id) const { return (*wayList)[getWayIndex(id)]; }
const OSMRelation& OSMSegment::getRelation(int64_t id) const { return (*relationList)[getRelationIndex(id)]; }

OSMSegment OSMSegment::findSquareNodes(
	float pLowerLat, float pUpperLat,
	float pLowerLon, float pUpperLon
) const {
	return findSquareNodes(Rect::fromBorders(
		pLowerLat, pUpperLat, pLowerLon, pUpperLon));
}

size_t OSMSegment::getManagedSize() const {
	size_t size = 0;

	size += sizeof(*nodeList) + nodeList->capacity() * sizeof(OSMNode);
	size += sizeof(*wayList) + wayList->capacity() * sizeof(OSMWay);
	size += sizeof(*relationList) + relationList->capacity() * sizeof(OSMRelation);

	for_each(nodeList->begin(), nodeList->end(),
		[&](const OSMNode& nd) { size += nd.getManagedSize(); });
	for_each(wayList->begin(), wayList->end(),
		[&](const OSMWay& wd) { size += wd.getManagedSize(); });
	for_each(relationList->begin(), relationList->end(),
		[&](const OSMRelation& rl) { size += rl.getManagedSize(); });
	// TODO
	size += nodeMap->calcNumBytesTotal(nodeMap->mask() + 1);
	size += wayMap->calcNumBytesTotal(wayMap->mask() + 1);
	size += relationMap->calcNumBytesTotal(relationMap->mask() + 1);
	return size;
}


size_t OSMSegment::getSize() const {
	return sizeof(*this) + getManagedSize();
}

OSMSegment OSMSegment::findSquareNodes(const Rect& r) const {
	return findNodes(
		[r](const OSMNode& nd) { return r.contains(Point(nd.getLat(), nd.getLon())); },
		[](const OSMWay&) { return true; },
		[r](const OSMWay&, const OSMNode& nd) { return r.contains(Point(nd.getLat(), nd.getLon())); }
	);
}

OSMSegment OSMSegment::findTagNodes(const string& tag) const {
	return findNodes(
		[&tag](const OSMNode& nd) { return nd.hasTag(tag); },
		[](const OSMWay&) { return true; },
		[&tag](const OSMWay&, const OSMNode& nd) { return nd.hasTag(tag); }
	);
}

OSMSegment OSMSegment::findTagWays(const string& tag) const {
	return findNodes(
		[](const OSMNode&) { return true; },
		[&tag](const OSMWay& wd) { return wd.hasTag(tag); },
		[](const OSMWay&, const OSMNode&) { return true; }
	);
}

OSMSegment OSMSegment::findCircleNode(const Circle& circle) const {
	return findNodes(
		[circle](const OSMNode& nd) { return circle.contains(Point(nd.getLat(), nd.getLon())); },
		[](const OSMWay&) { return true; },
		[circle](const OSMWay&, const OSMNode& nd) { return circle.contains(Point(nd.getLat(), nd.getLon())); }
	);
}

void OSMSegment::summary() const {
	printf("OSMSegment summary:\n");
	printf("    Lat: %f-%f\n", lowerLat, upperLat);
	printf("    Lon: %f-%f\n", lowerLon, upperLon);
	printf("    Nodes: %d\n", nodeList->size());
	printf("    Ways: %d\n", wayList->size());
	printf("    Relations: %d\n", relationList->size());
	printf("    Total size: %d\n", getSize());
	;
}

int64_t OSMSegment::findClosestNode(float lat, float lon) const {
	Point p(lat, lon);
	int64_t currentID = 0;
	float maxDistance = 1000000000;
	for (const OSMNode& nd : (*nodeList)) {
		float dt = p.distanceTo(Point(lat, lon)).getLengthSquared();
		if (dt < maxDistance) {
			maxDistance = dt;
			currentID = nd.getID();
		};
	}
	return currentID;
}

const shared_ptr<vector<OSMNode>>& OSMSegment::getNodes() const noexcept { return nodeList; }
const shared_ptr<vector<OSMWay>>& OSMSegment::getWays() const noexcept { return wayList; }
const shared_ptr<vector<OSMRelation>>& OSMSegment::getRelations() const noexcept { return relationList; }

const std::shared_ptr<map_t>& OSMSegment::getNodeMap() const noexcept { return nodeMap; }
const std::shared_ptr<mapid_t<std::vector<size_t>>>& OSMSegment::getWayMap() const noexcept { return wayMap; }
const std::shared_ptr<mapid_t<std::vector<size_t>>>& OSMSegment::getRelationMap() const noexcept { return relationMap; }

Rect OSMSegment::getBoundingBox() const noexcept {
	return Rect::fromBorders(lowerLat, upperLat, lowerLon, upperLon);
}

void traffic::OSMSegment::setBoundingBox(const Rect& rect) noexcept
{
	lowerLat = rect.lowerLatBorder();
	upperLat = rect.upperLatBorder();
	lowerLon = rect.lowerLonBorder();
	upperLon = rect.upperLonBorder();
}


// TODO
void debugTags(const OSMSegment& map) {
	unordered_map<string, int32_t> tagMap = map.createTagList();
	vector<pair<string, int32_t>> tagVec(tagMap.begin(), tagMap.end());
	sort(tagVec.begin(), tagVec.end(), [](auto& a, auto& b) { return a.second > b.second; });

	for (const auto& it : tagVec) {
		printf("Key %s %d\n", it.first.c_str(), it.second);
	}
}

OSMMap::OSMMap(const std::shared_ptr<OSMSegment>& map, prec_t chunkSize)
{
	this->m_chunkSize = chunkSize;
	boundingBox = map->getBoundingBox();
	recalculateChunks();
	insertSegment(*map);
}

void traffic::OSMMap::insertSegment(const OSMSegment& segment)
{
	for (const OSMNode& node : *segment.getNodes())
		addNode(node);
	//for (const OSMWay &way : *segment.getWays())
	//	addWayRecursive(way, segment.get);
}

void traffic::OSMMap::recalculateChunks()
{
	m_latOffset = latCoordToGlobal(boundingBox.lowerLatBorder());
	m_lonOffset = lonCoordToGlobal(boundingBox.lowerLonBorder());

	m_latChunks = latCoordToGlobal(boundingBox.upperLatBorder()) - m_latOffset + 1;
	m_lonChunks = lonCoordToGlobal(boundingBox.upperLonBorder()) - m_lonOffset + 1;
	m_chunks = std::vector<OSMSegment>(m_latChunks * m_lonChunks);
	for (size_t lat = 0; lat < m_latChunks; lat++)
	{
		for (size_t lon = 0; lon < m_lonChunks; lon++)
		{
			Rect rect = Rect::fromLength(
				latLocalToCoord(lat), lonLocalToCoord(lon),
				m_chunkSize, m_chunkSize
			);
			m_chunks[toStore(lat, lon)].setBoundingBox(rect);
		}
	}

	/*
	for (const OSMNode& nd : *(m_map->getNodes()))
	{
		size_t location = toStore(nd.getLat(), nd.getLon());
		m_chunks[location].addNode(nd.getID());
	}
	*/
}

const OSMSegment& traffic::OSMMap::getSegmentByNode(int64_t id) const
{
	return m_chunks[keyCheck(getSegmentIndexByNode(id))];
}

const OSMSegment& traffic::OSMMap::getSegment(prec_t lat, prec_t lon) const
{
	return m_chunks[keyCheck(getSegmentIndex(lat, lon))];
}

const OSMNode& traffic::OSMMap::getNode(int64_t nodeID) const
{
	return getSegmentByNode(nodeID).getNode(nodeID);
}

const OSMWay& traffic::OSMMap::getWay(int64_t wayID) const
{
	return getSegmentByNode(wayID).getWay(wayID);
}

const OSMRelation& traffic::OSMMap::getRelation(int64_t relationID) const
{
	return getSegmentByNode(relationID).getRelation(relationID);
}

// ---- Index functions ---- //

size_t traffic::OSMMap::getSegmentIndexByNode(int64_t nodeID) const
{
	auto it = m_nodemap.find(nodeID);
	if (it == m_nodemap.end()) return numeric_limits<size_t>::max();
	return it->second;
}

const vector<size_t>& traffic::OSMMap::getSegmentIndexByWay(int64_t wayID) const
{
	auto it = m_waymap.find(wayID);
	if (it == m_waymap.end()) return noValues;
	return it->second;
}

const vector<size_t>& traffic::OSMMap::getSegmentIndexByRelation(int64_t relationID) const
{
	auto it = m_relationmap.find(relationID);
	if (it == m_relationmap.end()) return noValues;
	return it->second;
}

size_t traffic::OSMMap::getSegmentIndex(prec_t lat, prec_t lon) const
{
	size_t index = toStore(lat, lon);
	if (index < 0 || index >= m_chunks.size()) return numeric_limits<size_t>::max();
	return index;
}

bool traffic::OSMMap::addNode(const OSMNode& nd)
{
	size_t index = getSegmentIndex(nd.getLat(), nd.getLon());
	if (index == numeric_limits<size_t>::max()) return false;
	m_chunks[index].addNode(nd, false);
	
	if (m_nodemap.find(nd.getID()) == m_nodemap.end())
		m_nodemap[nd.getID()] = index;
	return true;
}

bool traffic::OSMMap::addWayRecursive(const OSMWay& way, const OSMSegment& lookup)
{
	size_t lastIndex = numeric_limits<size_t>::max();
	int32_t subIndex = 0;
	OSMWay currentWay;
	for (const int64_t nodeID : way.getNodes())
	{
		// finds the node reference and the fitting chunk.
		// continues with the next node if the node does not exist
		const OSMNode& nd = lookup.getNode(nodeID);
		size_t index = getSegmentIndex(nd.getLat(), nd.getLon());
		if (index == numeric_limits<size_t>::max()) continue;

		if (lastIndex == index && lastIndex != numeric_limits<size_t>::max()) {
			// adds the node to the child segment and continues the way.
			// This happens when the current node is in the same chunk as the
			// previous one or if it is generally the first node of the way.
			if (m_chunks[index].addNode(nd, false)) {
				m_nodemap[nd.getID()] = index;
				currentWay.addNode(nd.getID());
			}
		}
		else {
			// stops the way and stores a reference where to continue the way
			//m_nodemap[currentWay]
			currentWay.setSubIndex(subIndex);
			m_chunks[index].addWay(currentWay);
		}
		lastIndex = index;
	}

	/*
	// constructs the ways
	for (auto it = buffer.begin(); it != buffer.end(); ++it) {
		OSMWay newWay;
		for (const OSMNode& node : it->second) {
			newWay
		}
	}
	*/
	return false;
}

bool traffic::OSMMap::addRelationRecursive(const OSMRelation& re, const OSMSegment& lookup)
{
	return false;
}

const std::vector<OSMSegment>& traffic::OSMMap::getChunks() const { return m_chunks; }

size_t traffic::OSMMap::keyCheck(size_t index) const
{
	if (index == numeric_limits<size_t>::max())
		throw runtime_error("Could not find key!");
	return index;
}

size_t traffic::OSMMap::latCoordToGlobal(prec_t coord) const
{
	return (size_t)((coord + 90.0f) / m_chunkSize);
}

prec_t traffic::OSMMap::latGlobalToCoord(size_t global) const
{
	return (prec_t)(global * m_chunkSize) - 90.0f;
}

size_t traffic::OSMMap::latLocalToGlobal(size_t local) const
{
	return local + m_latOffset;
}

size_t traffic::OSMMap::latGlobalToLocal(size_t global) const
{
	return global - m_latOffset;
}

size_t traffic::OSMMap::latCoordToLocal(prec_t coord) const
{
	return latGlobalToLocal(latCoordToGlobal(coord));
}

prec_t traffic::OSMMap::latLocalToCoord(size_t local) const
{
	return latGlobalToCoord(latLocalToGlobal(local));
}

// Longitude //

size_t traffic::OSMMap::lonCoordToGlobal(prec_t coord) const
{ return (size_t)((coord + 180.0f) / m_chunkSize); }

prec_t traffic::OSMMap::lonGlobalToCoord(size_t global) const
{
	return (prec_t)(global * m_chunkSize) - 180.0f;
}

size_t traffic::OSMMap::lonLocalToGlobal(size_t local) const
{
	return local + m_lonOffset;
}

size_t traffic::OSMMap::lonGlobalToLocal(size_t global) const
{
	return global - m_lonOffset;
}

size_t traffic::OSMMap::lonCoordToLocal(prec_t coord) const
{
	return lonGlobalToLocal(lonCoordToGlobal(coord));
}

prec_t traffic::OSMMap::lonLocalToCoord(size_t local) const
{
	return lonGlobalToCoord(lonLocalToGlobal(local));
}

size_t traffic::OSMMap::toStore(size_t localLat, size_t localLon) const
{
	return localLon * m_latChunks + localLat;
}

size_t traffic::OSMMap::toStore(prec_t lat, prec_t lon) const
{
	// Maps each chunk to a unique index.
	size_t localLat = latCoordToLocal(lat);
	size_t localLon = lonCoordToLocal(lon);

	return toStore(localLat, localLon);
}


