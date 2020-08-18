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

#define RAPIDXML_DYNAMIC_POOL_SIZE 4 * 64 * 1024 * 1024

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include <fmt/format.h>
#include <cptl.hpp>

#include "parser.hpp"

using namespace rapidxml;
using namespace std;
using namespace chrono;

using namespace traffic;

/// <summary>Converts a string to an template argument by using the
/// converter function</summary>
/// <typeparam name="C">Conversion functor type</typeparam>
/// <param name="val">The string that is converted</param>
/// <param name="conv">The conversion funtor that is used</param>
/// <returns>The value t</returns>
template<typename C>
auto conversion(const string& val, const C& conv) {
	try { return conv(val); }
	catch (const invalid_argument &) { cout << "Could not convert argument:" << val << endl; }
	catch (const out_of_range& ) { cout << "Argument out of range:" << val; }
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

struct ParseInfo
{
	// READ ACCESS ONLY //
	xml_document<char> doc;
	xml_node<char>* osm_node = nullptr;
	xml_node<char>* meta_node = nullptr;

	// ACCESS after lock aquire //
	vector<OSMNode> nodeList;
	vector<OSMWay> wayList;
	vector<OSMRelation> relationList;
	vector<atomic<bool>> values;

	unordered_map<string, uint16_t> tagMap;
};

struct LocalParseInfo
{
	// Thread information //
	int start, stride;
};

class ParseTask
{
public:
	ParseTask(ParseInfo *info, LocalParseInfo local);

	bool operator()(int id);
	bool parseNode(xml_node<char>* singleNode, int id);
	bool parseWay(xml_node<char>* singleNode, int id);
	bool parseRelation(xml_node<char>* singleNode, int id);

	bool parseTag(xml_node<char>* node, shared_ptr<vector<pair<string, string>>> &tagList);

protected:
	// Global parse data //
	ParseInfo* info;
	LocalParseInfo local;
};

ParseTask::ParseTask(ParseInfo* info, LocalParseInfo local)
{
	this->info = info;
	this->local = local;
}

bool ParseTask::operator()(int id)
{
	// Skips the first offset nodes
	int nodeCount = 0, wayCount = 0, relationCount = 0;
	xml_node<char>* singleNode = info->osm_node->first_node();
	for (int i = 0; singleNode && i < local.start; i++) {
		if (strncmp(singleNode->name(), "node", 4) == 0) nodeCount++;
		if (strncmp(singleNode->name(), "way", 8) == 0) wayCount++;
		if (strncmp(singleNode->name(), "relationCount", 3) == 0) relationCount++;
		singleNode = singleNode->next_sibling();
	}

	/// Iterates over every node in this document
	for (int i = 0; singleNode; 
		singleNode = singleNode->next_sibling())
	{
		char *nodeString = singleNode->name();
		if (strncmp(nodeString, "node", 4) == 0) {
			if (i == 0) parseNode(singleNode, nodeCount);
			nodeCount++;
		}
		else if (strncmp(nodeString, "way", 3) == 0) {
			if (i == 0) parseWay(singleNode, wayCount);
			wayCount++;
		}
		else if (strncmp(nodeString, "relation", 8) == 0) {
			if (i == 0) parseRelation(singleNode, relationCount);
			relationCount++;
		}
		else {
			printf("Unknown XML node: %.*s\n",
				(int)singleNode->name_size(), nodeString);
		}
		i = (i + 1) % local.stride;
	}
	return true;
}

bool ParseTask::parseNode(xml_node<char>* singleNode, int pos)
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
		id = parse<int64_t>(string(idAtt->value(), idAtt->value_size()));
		ver = parse<int32_t>(string(verAtt->value(), verAtt->value_size()));
		lat = parseDouble(string(latAtt->value(), latAtt->value_size()));
		lon = parseDouble(string(lonAtt->value(), lonAtt->value_size()));
	}
	catch (runtime_error&) {
		printf("Could not convert node parameter to integer argument\n");
		return false;
	}

	// Tries parsing the the list of tags attached to this node.
	// Every tag is build in the format <tag k="..." v="...">.
	// The attribute is skipped if the parser cannot find both attributes.
	shared_ptr<vector<pair<string, string>>> tags;

	for (xml_node<char>* tagNode = singleNode->first_node();
		tagNode; tagNode = tagNode->next_sibling())
	{
		char *tagNodeName = tagNode->name();
		if (strncmp(tagNodeName, "tag", 3) == 0) {
			parseTag(tagNode, tags);
		}
		else {
			printf("Unknown tag in node %.*s, skipping tag entry\n",
				(int)tagNode->name_size(), tagNodeName);
		}
	}
	// Shrinks the vector to save memory.
	if (tags)
		vector<pair<string, string>>(*tags).swap(*tags);

	// Successfully parsed the whole node. The node will be added to the node list.
	// A reference to the index will be saved inside the dictionary at a later point.
	info->nodeList[pos] = OSMNode(id, ver, tags, lat, lon);;
	return true;
}

bool ParseTask::parseWay(xml_node<char>* singleNode, int pos)
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
		id = parse<int64_t>(string(idAtt->value(), idAtt->value_size()));
		ver = parse<int32_t>(string(verAtt->value(), verAtt->value_size()));
	}
	catch (runtime_error&) {
		printf("Could not convert way parameter to integer argument\n");
		return false;
	}

	// Parses all child nodes that are attached to this nodes. Child nodes may
	// either be tags of the format <tag k="..." v="..."> or node references.
	shared_ptr<vector<int64_t>> wayInfo =
		make_shared<vector<int64_t>>();
	shared_ptr<vector<pair<string, string>>> tags;

	for (xml_node<char>* wayNode = singleNode->first_node();
		wayNode; wayNode = wayNode->next_sibling())
	{
		char *wayNodeName = wayNode->name();

		// Tries parsing a node reference. Nodes are defined by the tag 'nd'.
		// Every node tag has a reference integer giving the node's ID.
		if (strncmp(wayNodeName, "nd", 2) == 0) {
			xml_attribute<char>* refAtt = wayNode->first_attribute("ref");

			if (refAtt == nullptr) {
				printf("Ref attribute of way is not defined, skipping tag\n");
				continue;
			}

			int64_t ref;
			try { ref = parse<int64_t>(string(refAtt->value(), refAtt->value_size())); }
			catch (runtime_error&) {
				printf("Could not cast ref attribute, skipping tag\n");
				continue;
			}
			wayInfo->push_back(ref);
		}

		// Tries parsing a tag. A tag needs to have a key and
		// value defined by 'k' and 'v'.
		else if (strncmp(wayNodeName, "tag", 3) == 0) {
			parseTag(wayNode, tags);
		}
		// Could not parse the way child node.
		else {
			printf("Unknown way child node: %.*s\n",
				(int)wayNode->name_size(), wayNodeName);
		}
	}
	// shrinks the tags to save memory
	if (tags)
		vector<pair<string, string>>(*tags).swap(*tags);

	vector<int64_t>(*wayInfo).swap(*wayInfo);
	info->wayList[pos] = OSMWay(id, ver, move(wayInfo), tags);
	return true;
}

bool ParseTask::parseRelation(xml_node<char>* singleNode, int pos)
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
		id = parse<int64_t>(string(idAtt->value(), idAtt->value_size()));
		ver = parse<int32_t>(string(verAtt->value(), verAtt->value_size()));
	}
	catch (runtime_error&) {
		printf("Could not convert relation parameter to integer argument\n");
		return false;
	}


	shared_ptr<vector<RelationMember>> nodeRel = make_shared<vector<RelationMember>>();
	shared_ptr<vector<RelationMember>> wayRel = make_shared<vector<RelationMember>>();
	shared_ptr<vector<RelationMember>> relationRel = make_shared<vector<RelationMember>>();
	shared_ptr<vector<pair<string, string>>> tags;

	for (xml_node<char>* childNode = singleNode->first_node();
		childNode; childNode = childNode->next_sibling())
	{
		char *childNodeName = childNode->name();
		/// Tries parsing a member node. Every member node is
		/// either a reference to a way or to a node. They are
		/// required to have a 'type', 'ref' and 'role' tag.
		/// The parser must find all these tags to continue.
		if (strncmp(childNodeName, "member", 6) == 0) {
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
				ref = parse<int64_t>(string(indexAtt->value(), indexAtt->value_size()));
			}
			catch (runtime_error&) {
				printf("Could not parse ref attribute to integer argument\n");
				continue;
			}


			char *typeAttName = typeAtt->value();
			// Checks the type of the entry
			RelationMember member(ref, string(roleAtt->value(), roleAtt->value_size()));
			if (strncmp(typeAttName, "node", 4) == 0) {
				nodeRel->push_back(member);
			}
			else if (strncmp(typeAttName, "way", 3) == 0) {
				wayRel->push_back(member);
			}
			else if (strncmp(typeAttName, "relation", 8) == 0) {
				relationRel->push_back(member);
			}
			else {
				printf("Unknown type attribute in relation member '%.*s'\n",
					(int)typeAtt->name_size(), typeAttName);
			}
		}
		else if (strncmp(childNodeName, "tag", 3) == 0) {
			parseTag(childNode, tags);
		}
		else {
			printf("Unknown relation tag %.*s\n",
				(int)childNode->name_size(), childNodeName);
		}
	}

	vector<RelationMember>(*nodeRel).swap(*nodeRel);
	vector<RelationMember>(*wayRel).swap(*wayRel);
	vector<RelationMember>(*relationRel).swap(*relationRel);

	if (tags)
		vector<pair<string, string>>(*tags).swap(*tags);

	info->relationList[pos] = OSMRelation(id, ver,
		tags, nodeRel, wayRel, relationRel);
	return true;
}

bool ParseTask::parseTag(xml_node<char>* node,
	std::shared_ptr<vector<pair<string, string>>>& tagList)
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

	if (!tagList) {
		tagList = make_shared<vector<pair<string, string>>>();
	}

	tagList->push_back(pair<string, string>(
		string(kAtt->value(), kAtt->value_size()),
		string(vAtt->value(), vAtt->value_size()))
	);
	return true;
}

OSMSegment traffic::parseXMLMap(const ParseArguments &args)
{
	if (args.timings)
		args.timings->begin = high_resolution_clock::now();

	ParseInfo info; // Stores the global parse variables
	vector<char> buffer; // Reads the XML file into a vector of chars.
	if (readFile(buffer, args.file) != 0) 
		throw runtime_error("Could not read file into memory!");

	if (args.timings)
		args.timings->endRead = high_resolution_clock::now();

	try {
		info.doc.parse<parse_fastest>(buffer.data());
	} catch (const parse_error&) {
		throw runtime_error("Could not parse XML file!");
	}

	if (args.timings)
		args.timings->endXMLParse = chrono::high_resolution_clock::now();

	// Parses some special nodes
	// 1. The OSM node is the root of the document.
	// 2. The meta data node describes some meta data.
	info.osm_node = info.doc.first_node("osm");
	if (info.osm_node == nullptr)
		throw runtime_error("Could not find root node 'osm'\n");
	info.meta_node = info.osm_node->first_node("meta");
	if (info.meta_node == nullptr)
		throw runtime_error("Could not find root node 'meta'\n");


	/// Iterates over every node in this document
	size_t sizeNodes = 0, sizeRelations = 0, sizeWays = 0, size = 0;
	for (xml_node<char>* singleNode = info.osm_node->first_node(); singleNode;
		singleNode = singleNode->next_sibling()) {
		if (strncmp(singleNode->name(), "node", 4) == 0) sizeNodes++;
		else if (strncmp(singleNode->name(), "way", 3) == 0) sizeWays++;
		else if (strncmp(singleNode->name(), "relation", 8) == 0) sizeRelations++;
		size++;
	}

	info.nodeList.resize(sizeNodes);
	info.wayList.resize(sizeWays);
	info.relationList.resize(sizeRelations);

	ctpl::thread_pool* usedPool;
	if (args.pool) {
		usedPool = args.pool;
	} else {
		usedPool = new ctpl::thread_pool(args.threads);
	}

	ParseInfo *infoPtr = &info;
	{
		vector<future<bool>> futures(args.threads);
		for (int i = 0; i < args.threads; i++) {
			LocalParseInfo local;
			local.start = i;
			local.stride = args.threads;
			futures[i] = usedPool->push(ParseTask(infoPtr, local));
		}

		for (int i = 0; i < args.threads; i++) {
			try { futures[i].get(); }
			catch (const std::exception &e) {
				printf("Got exception from thread %d\n", i);
			}
		}
	}

	if (!args.pool) {
		delete usedPool;
	}

	// Prints some diagnostics about the program
	if (args.timings)
		args.timings->endDataParse = chrono::high_resolution_clock::now();

	return OSMSegment(
		make_shared<vector<OSMNode>>(move(info.nodeList)),
		make_shared<vector<OSMWay>>(move(info.wayList)),
		make_shared<vector<OSMRelation>>(move(info.relationList))
	);
}

void traffic::ParseTimings::summary()
{
	string f1 = fmt::format("Read file into memory. Took {}ms total {}ms",
		duration_cast<milliseconds>(endRead - begin).count(),
		duration_cast<milliseconds>(endRead - begin).count());
	string f2 = fmt::format("Parsed XML file, Took {}ms, Total {}ms",
		duration_cast<milliseconds>(endXMLParse - endRead).count(),
		duration_cast<milliseconds>(endXMLParse - begin).count());
	string f3 = fmt::format("Parsed ways and nodes. Took {}ms, Total {}ms",
		duration_cast<milliseconds>(endDataParse - endXMLParse).count(),
		duration_cast<milliseconds>(endDataParse - begin).count());

	cout << f1 << endl << f2 << endl << f3 << endl;
}
