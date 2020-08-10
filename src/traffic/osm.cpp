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

// ---- OSM object ---- //

OSMMapObject::OSMMapObject() { }
OSMMapObject::OSMMapObject(int64_t id, int32_t version)
{
	this->id = id;
	this->version = version;
}

OSMMapObject::OSMMapObject(
	int64_t id_, int32_t version_,
	shared_ptr<vector<pair<string, string>>> tags_
) : tags(tags_)
{
	this->id = id_;
	this->version = version_;
}

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

void OSMMapObject::toJson(json& json) const {
	json["id"] = id;
	json["version"] = version;
	json["tags"] = *tags;
}

shared_ptr<vector<pair<string, string>>>
OSMMapObject::getData() const { return tags; }

bool OSMMapObject::hasTag(const string& key) const {
	if (tags) {
		for (const pair<string, string>& vecKey : (*tags)) {
			if (vecKey.first == key) return true;
		}
	}
	return false;
}

bool OSMMapObject::hasTagValue(
	const string& key, const string& value
) const {
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
	// TODO change output
	return "";
}

int64_t OSMMapObject::getID() const { return id; }
int32_t OSMMapObject::getVer() const { return version; }


// ---- OSMNode ---- //

OSMNode::OSMNode()
	: lat(0.0), lon(0.0) { }
OSMNode::OSMNode(int64_t id, int32_t ver, float _lat, float _lon)
	: OSMMapObject(id, ver), lat(_lat), lon(_lon) { }
OSMNode::OSMNode(int64_t id, int32_t ver,
	shared_ptr<vector<pair<string, string>>> tags,
	float _lat, float _lon)
	: OSMMapObject(id, ver, tags), lat(_lat), lon(_lon) { }
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

OSMWay::OSMWay(int64_t id, int32_t version,
	shared_ptr<vector<int64_t>>&& pnodes)
	: OSMMapObject(id, version), nodes(pnodes) { }

OSMWay::OSMWay(int64_t id, int32_t ver,
	shared_ptr<vector<int64_t>>&& nodes_,
	shared_ptr<vector<pair<string, string>>> tags
) : OSMMapObject(id, ver, tags), nodes(nodes_) { }

OSMWay::OSMWay(const json& json)
	: OSMMapObject(json) {
	nodes = make_shared<vector<int64_t>>();
	json.at("nodes").get_to<vector<int64_t>>(*nodes);
}

void OSMWay::toJson(json& json) const {
	OSMMapObject::toJson(json);
	json["nodes"] = *nodes;
}

size_t OSMWay::getManagedSize() const {
	size_t size = OSMMapObject::getManagedSize();
	size += sizeof(*nodes) + nodes->capacity() * sizeof(int64_t);
	return size;
}

size_t OSMWay::getSize() const {
	return getManagedSize() + sizeof(*this);
}

vector<int64_t>& OSMWay::getNodes() { return *nodes; }
const vector<int64_t>& OSMWay::getNodes() const { return *nodes; }

// ---- NodeRef ---- //

NodeRef::NodeRef(float value, size_t index)
	: value_(value), index_(index) { }

// ---- OSMRelation ---- //


OSMRelation::OSMRelation() { } // TODO initialize
OSMRelation::OSMRelation(
	int64_t id, int32_t ver,
	shared_ptr<vector<RelationMember>> nodes_,
	shared_ptr<vector<RelationMember>> ways_,
	shared_ptr<vector<RelationMember>> relations_
) : OSMMapObject(id, ver), nodes(nodes_),
ways(ways_), relations(relations_) { }

OSMRelation::OSMRelation(
	int64_t id, int32_t ver,
	shared_ptr<vector<pair<string, string>>> tags,
	shared_ptr<vector<RelationMember>> nodes_,
	shared_ptr<vector<RelationMember>> ways_,
	shared_ptr<vector<RelationMember>> relations_
) : OSMMapObject(id, ver, tags), nodes(nodes_),
ways(ways_), relations(relations_) { }

OSMRelation::OSMRelation(const json& json)
{
	nodes = make_shared<vector<RelationMember>>(json.at("nodes").get<vector<RelationMember>>());
	ways = make_shared<vector<RelationMember>>(json.at("ways").get<vector<RelationMember>>());
	relations = make_shared<vector<RelationMember>>(json.at("relations").get<vector<RelationMember>>());
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

// ---- OSMMap ---- //

XMLMap::XMLMap() {
	nodeList = make_shared<vector<OSMNode>>();
	wayList = make_shared<vector<OSMWay>>();
	relationList = make_shared<vector<OSMRelation>>();

	nodeMap = make_shared<map_t>();
	wayMap = make_shared<map_t>();
	relationMap = make_shared<map_t>();

	recalculateBoundaries();
}

XMLMap::XMLMap(
	const shared_ptr<vector<OSMNode>>& nodes,
	const shared_ptr<vector<OSMWay>>& ways,
	const shared_ptr<vector<OSMRelation>>& relations,
	const shared_ptr<map_t>& pNodeMap,
	const shared_ptr<map_t>& pWayMap,
	const shared_ptr<map_t>& pRelationMap)
{
	if (!nodes) throw "OSMNode list must not be nullptr";
	if (!ways) throw "Ways must not be nullptr";
	if (!relations) throw "Relations must not be nullptr";
	nodeList = nodes;
	wayList = ways;
	relationList = relations;

	if (!pNodeMap) throw "NodeMap must not be nullptr";
	if (!pWayMap) throw "WayMap must not be nullptr";
	if (!pRelationMap) throw "RelationMap must not be nullptr";
	nodeMap = pNodeMap;
	wayMap = pWayMap;
	relationMap = pRelationMap;

	recalculateBoundaries();
}

void XMLMap::recalculateBoundaries() {
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
			if (nd.getLat() < latMin) latMin = nd.getLat();
			if (nd.getLon() > lonMax) lonMax = nd.getLon();
			if (nd.getLon() < lonMin) lonMin = nd.getLon();
		}
		lowerLat = latMin;
		upperLat = latMax;
		lowerLon = lonMin;
		upperLon = lonMax;
	}
}

XMLMap::XMLMap(const json& json)
{
	nodeList = make_shared<vector<OSMNode>>(json.at("nodes").get<vector<OSMNode>>());
	wayList = make_shared<vector<OSMWay>>(json.at("ways").get<vector<OSMWay>>());
	relationList = make_shared<vector<OSMRelation>>(json.at("relations").get<vector<OSMRelation>>());
	recalculateBoundaries();
	//nodeMap = make_shared<map_t>(json.at("node_map").get<map_t>());
	//wayMap = make_shared<map_t>(json.at("way_map").get<map_t>());
	//relationMap = make_shared<map_t>(json.at("relation_map").get<map_t>());
}

traffic::XMLMap::~XMLMap()
{
	printf("Deallocating map\n");
}

void XMLMap::toJson(json& json) const {
	json["nodes"] = *nodeList;
	json["ways"] = *wayList;
	json["relations"] = *relationList;

	// TODO
	//json["node_map"] = *nodeMap;
	//json["way_map"] = *wayMap;
	//json["relation_map"] = *relationMap;
}


bool XMLMap::hasNodes() const { return nodeList->empty(); }
bool XMLMap::hasWays() const { return wayList->empty(); }
bool XMLMap::hasRelations() const { return relationList->empty(); }
bool XMLMap::empty() const { return !hasNodes() && !hasWays() && !hasRelations(); }

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

vector<int64_t> XMLMap::findAdress(
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

unordered_map<string, int32_t> XMLMap::createNodeTagList() const
{
	unordered_map<string, int32_t> map;
	createTTagList(*nodeList, map);
	return map;
}

unordered_map<string, int32_t> XMLMap::createWayTagList() const
{
	unordered_map<string, int32_t> map;
	createTTagList(*wayList, map);
	return map;
}

unordered_map<string, int32_t> XMLMap::createTagList() const
{
	unordered_map<string, int32_t> map;
	createTTagList(*nodeList, map);
	createTTagList(*wayList, map);
	return map;
}

size_t XMLMap::getNodeIndex(int64_t id) const {
	auto it = nodeMap->find(id);
	return it == nodeMap->end() ? numeric_limits<size_t>::max() : it->second;
}
size_t XMLMap::getWayIndex(int64_t id) const {
	auto it = wayMap->find(id);
	return it == wayMap->end() ? numeric_limits<size_t>::max() : it->second;
}
size_t XMLMap::getRelationIndex(int64_t id) const {
	auto it = relationMap->find(id);
	return it == relationMap->end() ? numeric_limits<size_t>::max() : it->second;
}

bool XMLMap::hasNodeIndex(int64_t id) const { return nodeMap->find(id) != nodeMap->end(); }
bool XMLMap::hasWayIndex(int64_t id) const { return wayMap->find(id) != wayMap->end(); }
bool XMLMap::hasRelationIndex(int64_t id) const { return relationMap->find(id) != relationMap->end(); }

bool XMLMap::addNode(const OSMNode& nd, bool updateBoundaries)
{
	auto it = nodeMap->find(nd.getID());
	if (it != nodeMap->end()) return false;
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

bool XMLMap::addWay(const OSMWay& wd,
	const XMLMap& map, bool addChildren, bool updateBoundaries
) {
	auto it = wayMap->find(wd.getID());
	if (it != wayMap->end()) return false;
	(*wayMap)[wd.getID()] = wayList->size();
	wayList->push_back(wd);

	if (addChildren) {
		for (int64_t id : wd.getNodes()) {
			size_t nodeID = map.getNodeIndex(id);
			if (nodeID == numeric_limits<size_t>::max()) {
				continue;
			}
			addNode(map.getNode(id), updateBoundaries);
		}
	}
	return true;
}

bool XMLMap::addRelation(const OSMRelation& re,
	const XMLMap& map, bool addChildren, bool updateBoundaries
) {
	auto it = relationMap->find(re.getID());
	if (it != relationMap->end()) return false;
	(*relationMap)[re.getID()] = relationList->size();
	relationList->push_back(re);

	if (addChildren) {
		for (RelationMember node : (*re.getNodes()))
			addNode(map.getNode(node.getIndex()), updateBoundaries);
		for (RelationMember way : (*re.getWays()))
			addWay(map.getWay(way.getIndex()), map, true, updateBoundaries);
		for (RelationMember r : (*re.getRelations()))
			addRelation(map.getRelation(r.getIndex()), map, true, updateBoundaries);
	}
	return true;
}

const OSMNode& XMLMap::getNode(int64_t id) const { return (*nodeList)[getNodeIndex(id)]; }
const OSMWay& XMLMap::getWay(int64_t id) const { return (*wayList)[getWayIndex(id)]; }
const OSMRelation& XMLMap::getRelation(int64_t id) const { return (*relationList)[getRelationIndex(id)]; }

XMLMap XMLMap::findSquareNodes(
	float pLowerLat, float pUpperLat,
	float pLowerLon, float pUpperLon
) const {
	return findSquareNodes(Rect::fromBorders(
		pLowerLat, pUpperLat, pLowerLon, pUpperLon));
}

size_t XMLMap::getManagedSize() const {
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


size_t XMLMap::getSize() const {
	return sizeof(*this) + getManagedSize();
}

XMLMap XMLMap::findSquareNodes(const Rect& r) const {
	return findNodes(
		[r](const OSMNode& nd) { return r.contains(Point(nd.getLat(), nd.getLon())); },
		[](const OSMWay&) { return true; },
		[r](const OSMWay&, const OSMNode& nd) { return r.contains(Point(nd.getLat(), nd.getLon())); }
	);
}

XMLMap XMLMap::findTagNodes(const string& tag) const {
	return findNodes(
		[&tag](const OSMNode& nd) { return nd.hasTag(tag); },
		[](const OSMWay&) { return true; },
		[&tag](const OSMWay&, const OSMNode& nd) { return nd.hasTag(tag); }
	);
}

XMLMap XMLMap::findTagWays(const string& tag) const {
	return findNodes(
		[](const OSMNode&) { return true; },
		[&tag](const OSMWay& wd) { return wd.hasTag(tag); },
		[](const OSMWay&, const OSMNode&) { return true; }
	);
}

XMLMap XMLMap::findCircleNode(const Circle& circle) const {
	return findNodes(
		[circle](const OSMNode& nd) { return circle.contains(Point(nd.getLat(), nd.getLon())); },
		[](const OSMWay&) { return true; },
		[circle](const OSMWay&, const OSMNode& nd) { return circle.contains(Point(nd.getLat(), nd.getLon())); }
	);
}

void XMLMap::summary() const {
	printf("XMLMap summary:\n");
	printf("    Lat: %f-%f\n", lowerLat, upperLat);
	printf("    Lon: %f-%f\n", lowerLon, upperLon);
	printf("    Nodes: %d\n", nodeList->size());
	printf("    Ways: %d\n", wayList->size());
	printf("    Relations: %d\n", relationList->size());
	printf("    Total size: %d\n", getSize());
	;
}

int64_t XMLMap::findClosestNode(float lat, float lon) const {
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

shared_ptr<vector<OSMNode>>& XMLMap::getNodes() { return nodeList; }
shared_ptr<vector<OSMWay>>& XMLMap::getWays() { return wayList; }
shared_ptr<vector<OSMRelation>>& XMLMap::getRelations() { return relationList; }

const shared_ptr<vector<OSMNode>>& XMLMap::getNodes() const { return nodeList; }
const shared_ptr<vector<OSMWay>>& XMLMap::getWays() const { return wayList; }
const shared_ptr<vector<OSMRelation>>& XMLMap::getRelations() const { return relationList; }


// TODO
void debugTags(const XMLMap& map) {
	unordered_map<string, int32_t> tagMap = map.createTagList();
	vector<pair<string, int32_t>> tagVec(tagMap.begin(), tagMap.end());
	sort(tagVec.begin(), tagVec.end(), [](auto& a, auto& b) { return a.second > b.second; });

	for (const auto& it : tagVec) {
		printf("Key %s %d\n", it.first.c_str(), it.second);
	}
}