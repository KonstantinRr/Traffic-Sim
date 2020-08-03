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
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include "parser.hpp"

using namespace rapidxml;
using namespace std;
using namespace chrono;

using namespace traffic;

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
	if (!f) {
		return -1;
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	data.resize(fsize + 1);
	fread(data.data(), 1, fsize, f);
	fclose(f);

	data[fsize] = 0;
	return 0;
}


XMLMap traffic::parseXMLMap(const string& file)
{
	printf("Parsing XML file %s\n", file.c_str());
	auto begin = high_resolution_clock::now();

	xml_document<char> doc;
	xml_node<char>* osm_node;
	xml_node<char>* meta_node;

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
		doc.parse<0>(&buffer[0]);
	} catch (const parse_error&) {
		printf("Could not parse xml file '%s'\n", file.c_str());
		throw runtime_error("Could not parse xml file");
	}

	auto endParse = chrono::high_resolution_clock::now();
	printf("Parsed XML file, Took %dms, Total %dms\n",
		duration_cast<milliseconds>(endParse - endSystemRead).count(),
		duration_cast<milliseconds>(endParse - begin).count());

	/// The OSM node is the root of the document.
	osm_node = doc.first_node("osm");
	if (osm_node == nullptr) {
		printf("Could not find root node 'osm'\n");
		throw runtime_error("Could not find root node 'osm'\n");
	}

	/// The meta data node describes some meta data.
	meta_node = osm_node->first_node("meta");
	if (meta_node == nullptr) {
		printf("Could not find root node 'meta'\n");
		throw runtime_error("Could not find root node 'meta'\n");
	}

	// The variables that will be filled during parsing
	vector<OSMNode> nodeList;
	vector<OSMWay> wayList;
	vector<OSMRelation> relationList;

	using map_t = robin_hood::unordered_node_map<int64_t, size_t>;
	map_t nodeMap;
	map_t wayMap;
	map_t relationMap; 

	/// Iterates over every node in this document
	for (xml_node<char>* singleNode = osm_node->first_node();
		singleNode; singleNode = singleNode->next_sibling())
	{
		string nodeString = singleNode->name();

		/// Tries parsing a node. Every node must have the
		/// following attributes. The parser will skip this node
		/// if any of these items are missing.
		/// <int64> id, <int32> version, float> lat, lon
		///
		/// The items will be casted from strings. The parser will
		/// skip this node if any casting error happens. Casting errors
		/// include out_of_range and invalid_number exceptions.
		/// Every node has a set of attributes that are parsed.
		if (nodeString == "node") {
			// Tries parsing the basic node attributes.
			// The parser must find all attributes to continue.
			xml_attribute<char>* idAtt = singleNode->first_attribute("id");
			xml_attribute<char>* latAtt = singleNode->first_attribute("lat");
			xml_attribute<char>* lonAtt = singleNode->first_attribute("lon");
			xml_attribute<char>* verAtt = singleNode->first_attribute("version");
			
			if (idAtt == nullptr) {
				printf("ID attribute is nullptr (skipping node)\n");
				continue;
			}
			if (verAtt == nullptr) {
				printf("VERSION attribute is nullptr (skipping node)\n");
				continue;
			}
			if (latAtt == nullptr) {
				printf("LAT attribute is nullptr (skipping node)\n");
				continue;
			}
			if (lonAtt == nullptr) {
				printf("LON attribute is nullptr (skipping node)\n");
				continue;
			}

			// Tries casting the arguments. The node is skipped
			// if any errors occur during casting.
			int64_t id;
			int32_t ver;
			prec_t lat, lon;
			try {
				id = parse<int64_t>(idAtt->value());
				ver = parse<int32_t>(verAtt->value());
				lat = parseDouble(latAtt->value());
				lon = parseDouble(lonAtt->value());
			}
			catch (runtime_error &) {
				printf("Could not convert node parameter to integer argument\n");
				continue;
			}

			// Tries parsing the tag arguments of this node.
			// Every tag consists of the format <tag k="" v="">.
			// The attribute is skipped if the parser cannot find both
			// attributes (k, v).
			shared_ptr<vector<pair<string, string>>> tags = make_shared<vector<pair<string, string>>>();
			for (xml_node<char>* tagNode = singleNode->first_node();
				tagNode; tagNode = tagNode->next_sibling()) {
			
				string tagNodeName = tagNode->name();
				
				if (tagNodeName == "tag") {
					xml_attribute<char>* kAtt = tagNode->first_attribute("k");
					xml_attribute<char>* vAtt = tagNode->first_attribute("v");

					if (kAtt->value() == nullptr) {
						printf("Tag key attribute is nullptr, skipping node entry\n");
						continue;
					}
					if (vAtt->value() == nullptr) {
						printf("Tag value attribute is nullptr, skipping node entry\n");
						continue;
					}

					tags->push_back(pair<string, string>(kAtt->value(), vAtt->value()));
				}
				else {
					printf("Unknown tag in node %s, skipping tag entry\n", tagNodeName.c_str());
				}
			}
			// shrinks the vector to save memory.
			vector<pair<string, string>>(*tags).swap(*tags);

			// Successfully parsed the whole node. The node
			// will be added to the node list. A reference to
			// the index will be saved inside the dictionary.
			OSMNode nd(id, ver, tags, lat, lon);
			nodeList.push_back(nd);
			nodeMap[id] = nodeList.size() - 1;
		}

		/// Tries parsing a way using the current node.
		/// Every way must include the following attributes.
		/// <int64> id, <int32> version
		else if (nodeString == "way") {
			// Tries parsing the basic node attributes.
			// The parser must find all attributes to continue.
			xml_attribute<char>* idAtt = singleNode->first_attribute("id");
			xml_attribute<char>* verAtt = singleNode->first_attribute("version");

			if (idAtt == nullptr) {
				printf("ID attribute is nullptr (skipping node)\n");
				continue;
			}
			if (verAtt == nullptr) {
				printf("VERSION attribute is nullptr (skipping node)\n");
				continue;
			}


			// Tries casting the way attributes to numeric values.
			// The parser must be able to convert all values to continue.
			int64_t id;
			int32_t ver;
			try {
				id = parse<int64_t>(idAtt->value());
				ver = parse<int32_t>(verAtt->value());
			}
			catch (runtime_error &) {
				printf("Could not convert way parameter to integer argument\n");
				continue;
			}

			// Parses all child nodes in the tag. There are
			// two possible tags in a way.
			shared_ptr<vector<int64_t>> wayInfo =
				make_shared<vector<int64_t>>();
			shared_ptr<vector<pair<string, string>>> tags =
				make_shared<vector<pair<string, string>>>();
			for (xml_node<char>* wayNode = singleNode->first_node();
				wayNode; wayNode = wayNode->next_sibling())
			{	
				string wayNodeName = wayNode->name();

				// Tries parsing a node. Nodes are defined by the tag 'nd'.
				// Every node tag has a reference node pointing to a node.
				if (wayNodeName == "nd") {
					xml_attribute<char>* refAtt = wayNode->first_attribute("ref");

					if (refAtt == nullptr) {
						printf("Ref attribute of way is not defined, skipping tag\n");
						continue;
					}

					int64_t ref;
					try {
						ref = parse<int64_t>(refAtt->value());
					} catch (runtime_error &) {
						printf("Could not cast ref attribute, skipping tag\n");
						continue;
					}

					auto it = nodeMap.find(ref);
					if (it == nodeMap.end()) {
						printf("Could not find node: %d\n", ref);
						continue;
					}
					wayInfo->push_back(ref);
				}

				// Tries parsing a tag. A tag needs to have a key and
				// value defined by 'k' and 'v'.
				else if (wayNodeName == "tag") {
					xml_attribute<char>* kAtt = wayNode->first_attribute("k");
					xml_attribute<char>* vAtt = wayNode->first_attribute("v");

					if (kAtt->value() == nullptr) {
						printf("Tag key attribute is nullptr, skipping way entry\n");
						continue;
					}
					if (vAtt->value() == nullptr) {
						printf("Tag value attribute is nullptr, skipping way entry\n");
						continue;
					}

					tags->push_back(pair<string, string>(
						kAtt->value(), vAtt->value()));
				}
				// Could not parse the way child node.
				else {
					printf("Unknown way child node: %s\n", wayNodeName.c_str());
				}
			}
			// shrinks the tags to save memory
			vector<pair<string, string>>(*tags).swap(*tags);
			vector<int64_t>(*wayInfo).swap(*wayInfo);
			
			wayList.push_back(OSMWay(id, ver, move(wayInfo), tags));
			wayMap[id] = wayList.size() - 1;
		}

		/// Tries parsing a relation using the current node.
		/// Every way must include the following attributes.
		/// <int64> id, <int32> version
		else if (nodeString == "relation") {
			// Tries parsing the basic attributes.
			// The parser must find all attributes to continue.
			xml_attribute<char>* idAtt = singleNode->first_attribute("id");
			xml_attribute<char>* verAtt = singleNode->first_attribute("version");

			if (idAtt == nullptr) {
				printf("ID attribute is nullptr (skipping relation)\n");
				continue;
			}
			if (verAtt == nullptr) {
				printf("VERSION attribute is nullptr (skipping relation)\n");
				continue;
			}

			// Tries casting the way attributes to numeric values.
			// The parser must be able to convert all values to continue.
			int64_t id;
			int32_t ver;
			try {
				id = parse<int64_t>(idAtt->value());
				ver = parse<int32_t>(verAtt->value());
			}
			catch (runtime_error &) {
				printf("Could not convert relation parameter to integer argument\n");
				continue;
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
					catch (runtime_error &) {
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
					xml_attribute<char>* kAtt = childNode->first_attribute("k");
					xml_attribute<char>* vAtt = childNode->first_attribute("v");

					if (kAtt->value() == nullptr) {
						printf("Tag key attribute is nullptr, skipping way entry\n");
						continue;
					}
					if (vAtt->value() == nullptr) {
						printf("Tag value attribute is nullptr, skipping way entry\n");
						continue;
					}

					tags->push_back(pair<string, string>(kAtt->value(), vAtt->value()));
				}
				else {
					printf("Unknown relation tag %s\n", childNodeName.c_str());
				}
			}

			vector<RelationMember>(*nodeRel).swap(*nodeRel);
			vector<RelationMember>(*wayRel).swap(*wayRel);
			vector<RelationMember>(*relationRel).swap(*relationRel);
			vector<pair<string, string>>(*tags).swap(*tags);

			relationList.push_back(OSMRelation(id, ver,
				tags, nodeRel, wayRel, relationRel));
			relationMap[id] = relationList.size() - 1;
		}
		/// Could not identify the node.
		else {
			printf("Unknown XML node: %s\n", nodeString.c_str());
		}
	}


	vector<OSMNode>(nodeList).swap(nodeList);
	vector<OSMWay>(wayList).swap(wayList);
	vector<OSMRelation>(relationList).swap(relationList);

	// Prints some diagnostics about the program
	auto endRead = chrono::high_resolution_clock::now();
	printf("Parsed %d ways and %d nodes. Took %dms, Total %dms\n",
		wayList.size(), nodeList.size(),
		duration_cast<milliseconds>(endRead - endParse).count(),
		duration_cast<milliseconds>(endRead - begin).count());

	return XMLMap(
		make_shared<vector<OSMNode>>(move(nodeList)),
		make_shared<vector<OSMWay>>(move(wayList)),
		make_shared<vector<OSMRelation>>(move(relationList)),
		make_shared<map_t>(move(nodeMap)),
		make_shared<map_t>(move(wayMap)),
		make_shared<map_t>(move(relationMap))
	);
}
