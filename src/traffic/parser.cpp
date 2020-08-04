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

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <algorithm>
#include <exception>
#include <atomic>
#include <thread>
#include <mutex>

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include <cptl.hpp>

#include "parser.hpp"

using namespace rapidxml;
using namespace std;
using namespace chrono;

using namespace traffic;

using map_t = robin_hood::unordered_node_map<int64_t, size_t>;

/// <summary>Converts a string to an template argument by using the
/// converter function</summary>
/// <typeparam name="C">Conversion functor type</typeparam>
/// <param name="val">The string that is converted</param>
/// <param name="conv">The conversion funtor that is used</param>
/// <returns>The value t</returns>
template<typename C>
auto conversion(const string& val, const C& conv) {
	try {
		return conv(val);
	}
	catch (const invalid_argument &) {
		cout << "Could not convert argument:" << val << endl;
	}
	catch (const out_of_range& ) {
		cout << "Argument out of range:" << val;
	}
	throw runtime_error("Could not convert argument");
}

prec_t parseDouble(const string &str)
{ return static_cast<prec_t>(conversion(str, [](const string& str){ return stod(str); })); }
int32_t parseInt32(const string& str)
{ return static_cast<int32_t>(conversion(str, [](const string &str){ return stoi(str); })); }
int64_t parseInt64(const string& str)
{ return static_cast<int64_t>(conversion(str, [](const string& str){ return stoll(str); })); }

template<typename T>
T parse(const string& str) {
	int64_t val = parseInt64(str);
	if (val > (numeric_limits<T>::max)() ||
		val < (numeric_limits<T>::min)()) {
		cout << "Argument out of range:" << val << endl;
		throw runtime_error("Argument out of range");
	}
	return static_cast<T>(val);
}

int readFile(vector<char> &data, const string &file) {
	FILE *f = fopen(file.c_str(), "rb");
	if (!f) return -1;
	
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	data.resize(static_cast<size_t>(fsize) + 1);
	fread(data.data(), 1, fsize, f);
	fclose(f);

	data[fsize] = 0; // null byte terminator
	return 0;
}

template<typename T, typename E>
bool tryMerge(std::vector<T>& vec, AtomicLock& lock, E&& function)
{
	if (vec.size() >= 1024 * 16) {
		if (lock.try_lock()) {
			function();
			lock.unlock();
			return true;
		}
	}
	return false;
}

struct ParseInfo
{
	// READ ACCESS ONLY //
	xml_document<char> doc;
	xml_node<char>* osm_node = nullptr;
	xml_node<char>* meta_node = nullptr;

	// LOCK ACCESS //
	AtomicLock lockNodes;
	AtomicLock lockWays;
	AtomicLock lockRelations;

	// ACCESS after lock aquire //
	vector<OSMNode> nodeList;
	vector<OSMWay> wayList;
	vector<OSMRelation> relationList;

	map_t nodeMap;
	map_t wayMap;
	map_t relationMap;
};

struct LocalParseInfo
{
	// Thread information //
	int start, stride;
	bool merge = true;
	size_t bufferSize = 16 * 1024;
	size_t mergeSize = 8 * 1024;
};

class ParseTask
{
public:
	ParseTask(ParseInfo *info, LocalParseInfo local);

	void forceMergeNodes();
	void forceMergeWays();
	void forceMergeRelations();

	void updateNodes();
	void updateWays();
	void updateRelations();

	bool operator()(int id);
	bool parseNode(xml_node<char>* singleNode);
	bool parseWay(xml_node<char>* singleNode);
	bool parseRelation(xml_node<char>* singleNode);

	bool parseTag(xml_node<char>* node, vector<pair<string, string>> &tagList);
protected:
	// Global parse data //
	ParseInfo* info;
	LocalParseInfo local;

	// Local storage for merging //
	vector<OSMNode> localNodeList;
	vector<OSMWay> localWayList;
	vector<OSMRelation> localRelationList;
};

ParseTask::ParseTask(ParseInfo* info, LocalParseInfo local)
{
	this->info = info;
	this->local = local;
}

void ParseTask::forceMergeNodes()
{
	if (!localNodeList.empty()) {
		scoped_lock lock(info->lockNodes);
		updateNodes();
	}
}

void ParseTask::forceMergeWays()
{
	if (!localWayList.empty()) {
		scoped_lock lock(info->lockWays);
		updateWays();
	}
}

void ParseTask::forceMergeRelations()
{
	if (!localRelationList.empty()) {
		scoped_lock lock(info->lockRelations);
		updateRelations();
	}
}

void ParseTask::updateNodes()
{
	info->nodeList.reserve(info->nodeList.size() + localNodeList.size());
	for (size_t i = 0; i < localNodeList.size(); i++) {
		info->nodeMap[localNodeList[i].getID()] = info->nodeList.size();
		info->nodeList.push_back(std::move(localNodeList[i]));
	}
	localNodeList.clear();
}

void ParseTask::updateWays()
{
	info->wayList.reserve(info->wayList.size() + localWayList.size());
	for (size_t i = 0; i < localWayList.size(); i++) {
		info->wayMap[localWayList[i].getID()] = info->wayList.size();
		info->wayList.push_back(std::move(localWayList[i]));
	}
	localWayList.clear();
}

void ParseTask::updateRelations()
{
	info->relationList.reserve(info->relationList.size() + localRelationList.size());
	for (size_t i = 0; i < localRelationList.size(); i++) {
		info->relationMap[localRelationList[i].getID()] = info->relationList.size();
		info->relationList.push_back(std::move(localRelationList[i]));
	}
	localRelationList.clear();
}

bool ParseTask::operator()(int id)
{
	// Skips the first offset nodes
	xml_node<char>* singleNode = info->osm_node->first_node();
	for (int i = 0; singleNode && i < local.start; i++) {
		singleNode = singleNode->next_sibling();
	}

	localNodeList.reserve(local.bufferSize);
	localWayList.reserve(local.bufferSize);
	localRelationList.reserve(local.bufferSize);

	/// Iterates over every node in this document
	for (int i = local.stride; singleNode; i++,
		singleNode = singleNode->next_sibling())
	{
		if (i != local.stride) continue;
		i = 0;

		string nodeString = singleNode->name();
		if (nodeString == "node") {
			parseNode(singleNode);
			if (local.merge) {
				tryMerge(localNodeList, info->lockNodes,
					[this]() { this->updateNodes(); });
			}
		}
		else if (nodeString == "way") {
			parseWay(singleNode);
			if (local.merge) {
				tryMerge(localWayList, info->lockWays,
					[this]() { this->updateWays(); });
			}
		}
		else if (nodeString == "relation") {
			parseRelation(singleNode);
			if (local.merge) {
				tryMerge(localRelationList, info->lockRelations,
					[this]() { this->updateRelations(); });
			}
		}
		else {
			printf("Unknown XML node: %s\n", nodeString.c_str());
		}
	}
	forceMergeNodes();
	forceMergeWays();
	forceMergeRelations();
	return true;
}

bool ParseTask::parseNode(xml_node<char>* singleNode)
{
	// Tries parsing the basic node attributes.
	// The parser must find all of the following attributes to continue parsing.
	xml_attribute<char>* idAtt = singleNode->first_attribute("id");
	xml_attribute<char>* latAtt = singleNode->first_attribute("lat");
	xml_attribute<char>* lonAtt = singleNode->first_attribute("lon");
	xml_attribute<char>* verAtt = singleNode->first_attribute("version");

	if (idAtt == nullptr) printf("ID attribute is nullptr (skipping node)\n");
	if (verAtt == nullptr) printf("VERSION attribute is nullptr (skipping node)\n");
	if (latAtt == nullptr) printf("LAT attribute is nullptr (skipping node)\n");
	if (lonAtt == nullptr) printf("LON attribute is nullptr (skipping node)\n");
	// Checks if each attributee was found.
	if (!idAtt || !verAtt || !latAtt || !lonAtt) return false;

	// Tries parsing the arguments to the correct internal representation.
	// The node is skipped if any errors occur during parsing.
	int64_t id;
	int32_t ver;
	prec_t lat, lon;
	try {
		id = parse<int64_t>(idAtt->value());
		ver = parse<int32_t>(verAtt->value());
		lat = parseDouble(latAtt->value());
		lon = parseDouble(lonAtt->value());
	}
	catch (runtime_error&) {
		printf("Could not convert node parameter to integer argument\n");
		return false;
	}

	// Tries parsing the the list of tags attached to this node.
	// Every tag is build in the format <tag k="..." v="...">.
	// The attribute is skipped if the parser cannot find both attributes.
	shared_ptr<vector<pair<string, string>>> tags
		= make_shared<vector<pair<string, string>>>();

	for (xml_node<char>* tagNode = singleNode->first_node();
		tagNode; tagNode = tagNode->next_sibling())
	{

		string tagNodeName = tagNode->name();
		if (tagNodeName == "tag") {
			parseTag(tagNode, *tags);
		}
		else {
			printf("Unknown tag in node %s, skipping tag entry\n", tagNodeName.c_str());
		}
	}
	// Shrinks the vector to save memory.
	vector<pair<string, string>>(*tags).swap(*tags);

	// Successfully parsed the whole node. The node will be added to the node list.
	// A reference to the index will be saved inside the dictionary at a later point.
	OSMNode nd(id, ver, tags, lat, lon);
	localNodeList.push_back(nd);

	return true;
}

bool ParseTask::parseWay(xml_node<char>* singleNode)
{
	// Tries parsing the basic way attributes.
	// The parser must find all of the following attributes to continue parsing.
	xml_attribute<char>* idAtt = singleNode->first_attribute("id");
	xml_attribute<char>* verAtt = singleNode->first_attribute("version");

	if (idAtt == nullptr) printf("ID attribute is nullptr (skipping node)\n");
	if (verAtt == nullptr) printf("VERSION attribute is nullptr (skipping node)\n");
	// Checks if each attributee was found.
	if (!idAtt || !verAtt) return false;


	// Tries casting the way attributes to numeric values.
	// The parser must be able to convert all values to continue.
	int64_t id;
	int32_t ver;
	try {
		id = parse<int64_t>(idAtt->value());
		ver = parse<int32_t>(verAtt->value());
	}
	catch (runtime_error&) {
		printf("Could not convert way parameter to integer argument\n");
		return false;
	}

	// Parses all child nodes that are attached to this nodes. Child nodes may
	// either be tags of the format <tag k="..." v="..."> or node references.
	shared_ptr<vector<int64_t>> wayInfo =
		make_shared<vector<int64_t>>();
	shared_ptr<vector<pair<string, string>>> tags =
		make_shared<vector<pair<string, string>>>();

	for (xml_node<char>* wayNode = singleNode->first_node();
		wayNode; wayNode = wayNode->next_sibling())
	{
		string wayNodeName = wayNode->name();

		// Tries parsing a node reference. Nodes are defined by the tag 'nd'.
		// Every node tag has a reference integer giving the node's ID.
		if (wayNodeName == "nd") {
			xml_attribute<char>* refAtt = wayNode->first_attribute("ref");

			if (refAtt == nullptr) {
				printf("Ref attribute of way is not defined, skipping tag\n");
				continue;
			}

			int64_t ref;
			try {
				ref = parse<int64_t>(refAtt->value());
			}
			catch (runtime_error&) {
				printf("Could not cast ref attribute, skipping tag\n");
				continue;
			}
			wayInfo->push_back(ref);
		}

		// Tries parsing a tag. A tag needs to have a key and
		// value defined by 'k' and 'v'.
		else if (wayNodeName == "tag") {
			parseTag(wayNode, *tags);
		}
		// Could not parse the way child node.
		else {
			printf("Unknown way child node: %s\n", wayNodeName.c_str());
		}
	}
	// shrinks the tags to save memory
	vector<pair<string, string>>(*tags).swap(*tags);
	vector<int64_t>(*wayInfo).swap(*wayInfo);

	localWayList.push_back(OSMWay(id, ver, move(wayInfo), tags));
	return true;
}

bool ParseTask::parseRelation(xml_node<char>* singleNode)
{
	// Tries parsing the basic attributes.
			// The parser must find all attributes to continue.
	xml_attribute<char>* idAtt = singleNode->first_attribute("id");
	xml_attribute<char>* verAtt = singleNode->first_attribute("version");

	if (idAtt == nullptr) printf("ID attribute is nullptr (skipping relation)\n");
	if (verAtt == nullptr) printf("VERSION attribute is nullptr (skipping relation)\n");
	
	if (!idAtt || !verAtt) return false;

	// Tries casting the way attributes to numeric values.
	// The parser must be able to convert all values to continue.
	int64_t id;
	int32_t ver;
	try {
		id = parse<int64_t>(idAtt->value());
		ver = parse<int32_t>(verAtt->value());
	}
	catch (runtime_error&) {
		printf("Could not convert relation parameter to integer argument\n");
		return false;
	}


	shared_ptr<vector<RelationMember>> nodeRel = make_shared<vector<RelationMember>>();
	shared_ptr<vector<RelationMember>> wayRel = make_shared<vector<RelationMember>>();
	shared_ptr<vector<RelationMember>> relationRel = make_shared<vector<RelationMember>>();
	shared_ptr<vector<pair<string, string>>> tags = make_shared<vector<pair<string, string>>>();

	for (xml_node<char>* childNode = singleNode->first_node();
		childNode; childNode = childNode->next_sibling())
	{
		string childNodeName = childNode->name();
		/// Tries parsing a member node. Every member node is
		/// either a reference to a way or to a node. They are
		/// required to have a 'type', 'ref' and 'role' tag.
		/// The parser must find all these tags to continue.
		if (childNodeName == "member") {
			xml_attribute<char>* typeAtt = childNode->first_attribute("type");
			xml_attribute<char>* indexAtt = childNode->first_attribute("ref");
			xml_attribute<char>* roleAtt = childNode->first_attribute("role");

			if (typeAtt == nullptr) {
				printf("Member type is nullptr, skipping entry in relation\n");
				continue;
			}
			if (indexAtt == nullptr) {
				printf("Index attribute is nullptr, skipping entry in relation\n");
				continue;
			}
			if (roleAtt == nullptr) {
				printf("Role attribute is nullptr, skipping entry in relation\n");
				continue;
			}

			int64_t ref;
			try {
				ref = parse<int64_t>(indexAtt->value());
			}
			catch (runtime_error&) {
				printf("Could not parse ref attribute to integer argument\n");
				continue;
			}


			string typeAttName = typeAtt->value();
			// Checks the type of the entry
			if (typeAttName == "node") {
				nodeRel->push_back(RelationMember(ref, roleAtt->value()));
			}
			else if (typeAttName == "way") {
				wayRel->push_back(RelationMember(ref, roleAtt->value()));
			}
			else if (typeAttName == "relation") {
				relationRel->push_back(RelationMember(ref, roleAtt->value()));
			}
			else {
				printf("Unknown type attribute in relation member '%s'\n", typeAttName.c_str());
			}
		}
		else if (childNodeName == "tag") {
			parseTag(childNode, *tags);
		}
		else {
			printf("Unknown relation tag %s\n", childNodeName.c_str());
		}
	}

	vector<RelationMember>(*nodeRel).swap(*nodeRel);
	vector<RelationMember>(*wayRel).swap(*wayRel);
	vector<RelationMember>(*relationRel).swap(*relationRel);
	vector<pair<string, string>>(*tags).swap(*tags);

	localRelationList.push_back(OSMRelation(id, ver,
		tags, nodeRel, wayRel, relationRel));
	return true;
}

bool ParseTask::parseTag(xml_node<char>* node, vector<pair<string, string>>& tagList)
{
	xml_attribute<char>* kAtt = node->first_attribute("k");
	xml_attribute<char>* vAtt = node->first_attribute("v");

	if (kAtt->value() == nullptr) {
		printf("Tag key attribute is nullptr, skipping node entry\n");
		return false;
	}
	if (vAtt->value() == nullptr) {
		printf("Tag value attribute is nullptr, skipping node entry\n");
		return false;
	}

	tagList.push_back(pair<string, string>(
		kAtt->value(), vAtt->value()));
	return true;
}

XMLMap traffic::parseXMLMap(const std::string& file)
{
	ctpl::thread_pool pool(1);
	return parseXMLMap(file, pool);
}

XMLMap traffic::parseXMLMap(const string& file, ctpl::thread_pool &pool)
{
	printf("Parsing XML file %s\n", file.c_str());
	int threads = 8;
	auto begin = high_resolution_clock::now();

	ParseInfo info;
	// Read the xml file into a vector of chars.
	// Adds a 0 byte to the vector to mark the end.
	vector<char> buffer;
	if (readFile(buffer, file) != 0) {
		printf("Could not open file %s\n", file.c_str());
		throw runtime_error("Could not open file");
	}

	// Gives some diagnostics
	auto endSystemRead = high_resolution_clock::now();
	printf("Read file into memory (Size: %d). Took %dms total %dms\n",
		buffer.size(),
		duration_cast<milliseconds>(endSystemRead - begin).count(),
		duration_cast<milliseconds>(endSystemRead - begin).count());

	try {
		info.doc.parse<0>(&buffer[0]);
	} catch (const parse_error&) {
		printf("Could not parse xml file '%s'\n", file.c_str());
		throw runtime_error("Could not parse xml file");
	}

	auto endParse = chrono::high_resolution_clock::now();
	printf("Parsed XML file, Took %dms, Total %dms\n",
		duration_cast<milliseconds>(endParse - endSystemRead).count(),
		duration_cast<milliseconds>(endParse - begin).count());

	/// The OSM node is the root of the document.
	info.osm_node = info.doc.first_node("osm");
	if (info.osm_node == nullptr) {
		printf("Could not find root node 'osm'\n");
		throw runtime_error("Could not find root node 'osm'\n");
	}

	/// The meta data node describes some meta data.
	info.meta_node = info.osm_node->first_node("meta");
	if (info.meta_node == nullptr) {
		printf("Could not find root node 'meta'\n");
		throw runtime_error("Could not find root node 'meta'\n");
	}

	ParseInfo *infoPtr = &info;
	vector<std::future<bool>> futures(threads);
	for (int i = 0; i < threads; i++) {
		LocalParseInfo local;
		local.bufferSize = 1024 * 16;
		local.mergeSize = 1024 * 8;
		local.merge = false;
		local.start = i;
		local.stride = threads;
		futures[i] = pool.push(ParseTask(infoPtr, local));
	}

	for (int i = 0; i < threads; i++) {
		try {
			futures[i].get();
		}
		catch (const std::exception &e) {
			printf("Got exception from thread %d\n", i);
		}
	}
	//threaded_parse(info, 0, 1);
	

	if (info.nodeList.capacity() > info.nodeList.size())
		vector<OSMNode>(info.nodeList).swap(info.nodeList);
	if (info.wayList.capacity() > info.wayList.size())
		vector<OSMWay>(info.wayList).swap(info.wayList);
	if (info.relationList.capacity() > info.relationList.size())
		vector<OSMRelation>(info.relationList).swap(info.relationList);

	// Prints some diagnostics about the program
	auto endRead = chrono::high_resolution_clock::now();
	printf("Parsed %d ways and %d nodes. Took %dms, Total %dms\n",
		info.wayList.size(), info.nodeList.size(),
		duration_cast<milliseconds>(endRead - endParse).count(),
		duration_cast<milliseconds>(endRead - begin).count());

	return XMLMap(
		make_shared<vector<OSMNode>>(move(info.nodeList)),
		make_shared<vector<OSMWay>>(move(info.wayList)),
		make_shared<vector<OSMRelation>>(move(info.relationList)),
		make_shared<map_t>(move(info.nodeMap)),
		make_shared<map_t>(move(info.wayMap)),
		make_shared<map_t>(move(info.relationMap))
	);
}
