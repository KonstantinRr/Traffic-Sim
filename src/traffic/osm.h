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

#pragma once

#ifndef OSM_H
#define OSM_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include "geom.h"
#include "robin_hood.h"
#include "json.hpp"

using nlohmann::json;

namespace traffic
{
	// forward declarations of all classes that are defined
	// in this header file.
	class OSMMapObject;	// The base class of all objects defined in the OSM format
	class OSMRelation;		// OpenStreetmap relation definition
	class OSMNode;			// OpenStreetMap node definition
	class OSMWay;			// OpenStreetMap way definition

	/// <summary>
	/// class OSMMapObject
	/// This class is the parent class of every object that is defined in the OSM data
	/// format. This includes the OSMNode, OSMWay and OSMRelation classes. Each of these classes
	/// share some common behavior that is summarized in this class system.
	/// It contains an ID, version identifier and tag system that is shared acroos all
	/// OSM objects. It offers some functionality to parse these tags and ids.
	/// See the class body for more information about the class' member variables.
	/// </summary>
	class OSMMapObject
	{
	protected:
		using vector_map = std::vector<
			std::pair<std::string, std::string>>;

		/// <summary> Unique identifier of the object inside the closed world.
		/// Identifiers may be negative in newer versions of the OSM format.
		/// </summary>
		int64_t id;

		/// <summary> The version of this object. Each object can be uniquely identified
		/// by an identifier (ID) and version tag. </summary>
		int32_t version;

		/// <summary> List of tags that every entity own. These attributes do not need
		/// to follow certain criteria and can store basically every std::string value.
		/// </summary>
		std::shared_ptr<vector_map> tags;

	public:
		// Constructor definitions //

		/// <summary>Creates an unitialized object</summary>
		/// <returns></returns>
		OSMMapObject();

		/// <summary> Creates an object with the given id and version. The tag list
		/// will be initialized without any key-value pairs.
		/// </summary>
		/// <param name="id">The OSM identifier of this object</param>
		/// <param name="version">The version of this object</param>
		/// <returns></returns>
		explicit OSMMapObject(int64_t id, int32_t version);

		/// <summary> Creates an object with the given id, version and taglist
		/// </summary>
		/// <param name="id">The OSM identifier of this object</param>
		/// <param name="version">The version of this object</param>
		/// <param name="tags">All tags that were specified in the OSM format</param>
		/// <returns></returns>
		explicit OSMMapObject(int64_t id, int32_t version,
			std::shared_ptr<vector_map> tags);

		/// <summary> Parses an OSM object from a JSON enoded object. All child classes
		/// are required to call this function when they are parsing from a JSON object.
		/// </summary>
		/// <param name="json">The JSON encoded object</param>
		/// <returns></returns>
		explicit OSMMapObject(const json& json);

		/// <summary> Destroys this object and frees all gather resources</summary>
		/// <returns></returns>
		virtual ~OSMMapObject() = default;

		// ---- Getters ---- //

		/// <summary> Returns the ID that is stored in this OSM object</summary>
		/// <returns>The identifier of this object (ID)</returns>
		int64_t getID() const;

		/// <summary> Returns the version of this OSM object</summary>
		/// <returns>The version of this object</returns>
		int32_t getVer() const;

		// ---- Tag functions ---- //

		/// <summary> Returns true whether the tag list contains a tag with the
		/// given name.</summary>
		/// <param name="key">The key which is searched in the map</param>
		/// <returns>True if the map contains a key with this tag, false otherwise</returns>
		bool hasTag(const std::string& key) const;

		/// <summary>Returns whether the tag list contains the given key-value pair. 
		/// </summary>
		/// <param name="key">Key of the key-value pair</param>
		/// <param name="value">Value of the key-value pair</param>
		/// <returns>True if the map contains the key-value pair, false otherwise</returns>
		bool hasTagValue(const std::string& key, const std::string& value) const;

		/// <summary>Returns the value of index by the given key. Raises an exception
		/// if the map does not contain the specific key</summary>
		/// <param name="key">The key used to index this value</param>
		/// <returns>The value index by this key</returns>
		std::string getValue(const std::string& key) const;

		/// <summary>Returns the key-value map in vector format</summary>
		/// <returns>A vector map that contains all key-value pairs</returns>
		std::shared_ptr<vector_map> getData() const;

		// ---- Size operators ---- //

		/// <summary>
		/// Returns the byte size that is managed by this object.
		/// This does not include the size of this object itself.
		/// </summary>
		/// <returns>The amount of bytes managed by this object</returns>
		virtual size_t getManagedSize() const;

		/// <summary>
		/// Returns the size of this object. This includes the managed size
		/// as well as the size of the object itself.
		/// </summary>
		/// <returns>The total size that is taken by this object</returns>
		virtual size_t getSize() const;

		// Encoding / Decoding //

		/// <summary>
		/// Exports this node to a JSON encoded file. This follows the given format
		/// specification.
		/// </summary>
		/// <param name="json">A JSON encoded object</param>
		void toJson(json& json) const;
	};


	/// <summary>
	/// class OSMNode
	/// This class represents a node entity in the world as specified by the
	/// OSM format. Nodes can be used to give any object a specific location
	/// in latitude and longitude  coordinates. They subclass the OSMMapObject
	/// class using a id, version and tag list.
	/// </summary>
	class OSMNode : public OSMMapObject
	{
	protected:
		/// <summary>The latitude coordinates of this node</summary>
		prec_t lat;

		/// <summary>The longitude coordinates of this node</summary>
		prec_t lon;

	public:
		/// <summary>Creates a node at the position (0.0, 0.0)</summary>
		/// <returns></returns>
		OSMNode();

		/// <summary>
		/// Creates a node using an id, version, latitude and longitude
		/// </summary>
		/// <param name="id">The node's unique ID</param>
		/// <param name="ver">The node's version tag</param>
		/// <param name="lat">The node's latitude</param>
		/// <param name="lon">The node's longitude</param>
		/// <returns></returns>
		explicit OSMNode(int64_t id, int32_t ver, float lat, float lon);

		/// <summary>
		/// Creates a node using an id, version, latitude and longitude with
		/// an additional tag list that consists of string pairs.
		/// </summary>
		/// <param name="id">The node's unique ID</param>
		/// <param name="ver">The node's version tag</param>
		/// <param name="tags">The node's tags</param>
		/// <param name="lat">The node's latitude</param>
		/// <param name="lon">The node's longitude</param>
		/// <returns></returns>
		explicit OSMNode(int64_t id, int32_t ver,
			std::shared_ptr<std::vector<std::pair<std::string, std::string>>> tags,
			float lat, float lon);

		/// <summary>
		/// Parses a OSMNode using a JSON encoded file. This json data needs
		/// to follow the format specifications.
		/// </summary>
		/// <param name="json">The JSON encoded object</param>
		/// <returns></returns>
		explicit OSMNode(const json& json);

		/// <summary>
		/// Destroys this node and all dynamically allocated objects.
		/// </summary>
		/// <returns></returns>
		virtual ~OSMNode() = default;

		virtual size_t getManagedSize() const;
		virtual size_t getSize() const;

		glm::vec2 asVector() const;

		/// <summary> Returns the latitude of this node </summary>
		/// <returns>The node's latitude</returns>
		prec_t getLat() const;

		/// <summary> Returns the longitude of this node </summary>
		/// <returns>The node's longitude</returns>
		prec_t getLon() const;


		/// <summary>
		/// Exports this node to a JSON file. This follows the given format
		/// specification.
		/// </summary>
		/// <param name="json"></param>
		void toJson(json& json) const;
	};

	/// <summary>
	/// class OSMWay
	/// Ways describe a pattern of nodes in the real world.
	/// </summary>
	class OSMWay : public OSMMapObject
	{
	protected:
		std::shared_ptr<std::vector<int64_t>> nodes;

	public:
		/// <summary> Creates an empty way that does not contain any nodes</summary>
		/// <returns></returns>
		OSMWay() = default;
		
		/// <summary>Creates a way using an id, version and nodes</summary>
		/// <param name="id">The way's ID</param>
		/// <param name="ver">The way's version</param>
		/// <param name="nodes">The way's nodes</param>
		/// <returns></returns>
		explicit OSMWay(int64_t id, int32_t ver, std::shared_ptr<std::vector<int64_t>>&& nodes);

		/// <summary>Creates a way using an id, version, nodes and tags</summary>
		/// <param name="id">The way's ID</param>
		/// <param name="ver">The way's version</param>
		/// <param name="nodes">The way's nodes</param>
		/// <param name="tags">The way's tags</param>
		/// <returns></returns>
		explicit OSMWay(int64_t id, int32_t ver,
			std::shared_ptr<std::vector<int64_t>>&& nodes,
			std::shared_ptr<std::vector<std::pair<std::string, std::string>>> tags);

		/// <summary> Parses a OSMWay using a json settings.
		/// This json data needs to follow the format specifications</summary>
		/// <param name="json">The JSON encoded object</param>
		/// <returns></returns>
		explicit OSMWay(const json& json);

		/// <summary>
		/// Destroys this way and all dynamically allocated objects.
		/// </summary>
		/// <returns></returns>
		virtual ~OSMWay() = default;

		virtual size_t getManagedSize() const;
		virtual size_t getSize() const;

		/// <summary>Accesses the nodes that are stored in this way</summary>
		/// <returns>A vector of nodes</returns>
		std::vector<int64_t>& getNodes();

		/// <summary>Accesses the nodes that are stored in this way</summary>
		/// <returns>A vector of nodes</returns>
		const std::vector<int64_t>& getNodes() const;
		
		/// <summary>Exports this node to a json file. This follows
		/// the given format specification</summary>
		/// <param name="json">The JSON encoded object</param>
		void toJson(json& json) const;
	};


	class RelationMember
	{
	protected:
		int64_t index;
		std::string type; // TODO change to role

	public:
		/// (1) Creates an empty relation member
		/// (2) Creates a relation member using an index and type
		/// (3) Parse a RelationMember object using a json object.
		explicit RelationMember();
		explicit RelationMember(int64_t index, const std::string& type);
		explicit RelationMember(const json& json);

		size_t getManagedSize() const;
		size_t getSize() const;


		/// Returns the index/type of this RelationMember
		int64_t getIndex() const;
		std::string& getType();
		const std::string& getType() const;

		/// Exports this RelationMember to a json file. This follows
		/// the given format specifications.
		void toJson(json& json) const;
	};

	/// Relations describe the correlation between
	/// ways and nodes. Every relation can have an
	/// arbitrary amount of member nodes and ways.
	/// They include tags in the same way as any
	/// other OSMMapObject does.
	class OSMRelation : public OSMMapObject
	{
	protected:
		std::shared_ptr<std::vector<RelationMember>> nodes;
		std::shared_ptr<std::vector<RelationMember>> ways;
		std::shared_ptr<std::vector<RelationMember>> relations;

	public:
		explicit OSMRelation();
		explicit OSMRelation(
			int64_t id, int32_t ver,
			std::shared_ptr<std::vector<RelationMember>> nodes,
			std::shared_ptr<std::vector<RelationMember>> ways,
			std::shared_ptr<std::vector<RelationMember>> relations);
		explicit OSMRelation(
			int64_t id, int32_t ver,
			std::shared_ptr<std::vector<std::pair<std::string, std::string>>> tags,
			std::shared_ptr<std::vector<RelationMember>> nodes,
			std::shared_ptr<std::vector<RelationMember>> ways,
			std::shared_ptr<std::vector<RelationMember>> relations);
		explicit OSMRelation(const json& json);

		virtual ~OSMRelation() = default;

		virtual size_t getManagedSize() const;
		virtual size_t getSize() const;

		void toJson(json& json) const;

		std::shared_ptr<std::vector<RelationMember>> getNodes() const;
		std::shared_ptr<std::vector<RelationMember>> getWays() const;
		std::shared_ptr<std::vector<RelationMember>> getRelations() const;
	};


	class NodeRef {
	protected:
		float value_;
		size_t index_;

	public:
		explicit NodeRef() = default;
		explicit NodeRef(float value, size_t index);

		inline void setValue(float value) { value_ = value; }
		inline void setIndex(size_t index) { index_ = index; }

		inline float getValue() const { return value_; }
		inline size_t getIndex() const { return index_; }
	};

	/// This class represents a MapStructure. It combines all
	/// values stored in the OpenStreetMap XML format.
	/// nodeList		All nodes stored in the XMLMap section
	/// wayList			All ways stored in the XMLMap section
	/// relationList	All relations stored in the XMLMap section
	/// float lowerLat	Min latitude value that is stored in this map
	/// float upperLat	Max latitude value that is stored in this map
	/// float lowerLon	Min longitude vlaue that is stored in this map
	/// float upperLon	Max longitude value that is stored in this map
	class XMLMap {
	protected:
		float lowerLat, upperLat, lowerLon, upperLon;

		std::shared_ptr<std::vector<OSMNode>> nodeList; // containts all nodes
		std::shared_ptr<std::vector<OSMWay>> wayList; // contains all ways
		std::shared_ptr<std::vector<OSMRelation>> relationList; // contains all relations

		// (1) maps the node ids to indices in the node list
		// (2) maps the way ids to indices in the way list
		// (3) maps the relation ids to indices in the relation list
		//using map_t = std::unordered_map<int64_t, size_t>;
		using map_t = robin_hood::unordered_node_map<int64_t, size_t>;
		std::shared_ptr<map_t> nodeMap;
		std::shared_ptr<map_t> wayMap;
		std::shared_ptr<map_t> relationMap;

	public:
		/// Creates a map that does not hold any data 
		explicit XMLMap();
		/// Creates a map that holds the passed data
		explicit XMLMap(
			const std::shared_ptr<std::vector<OSMNode>>& nodes,
			const std::shared_ptr<std::vector<OSMWay>>& ways,
			const std::shared_ptr<std::vector<OSMRelation>>& relations,
			const std::shared_ptr<map_t>& nodeMap,
			const std::shared_ptr<map_t>& wayMap,
			const std::shared_ptr<map_t>& relationMap);
		explicit XMLMap(const json& json);

		size_t getManagedSize() const;
		size_t getSize() const;

		void toJson(json& json) const;

		XMLMap(const XMLMap&) = delete;
		XMLMap(XMLMap&&) = default;

		XMLMap& operator=(const XMLMap&) = delete;
		XMLMap& operator=(XMLMap&&) = default;

		void recalculateBoundaries();
		void createSegmentMap() const;

		/// (1) Returns whether this map has any nodes
		/// (2) Returns whether this map has any ways
		/// (3) Returns whether this map has any relations
		/// (4) Returns whether this map holds any objects
		bool hasNodes() const;
		bool hasWays() const;
		bool hasRelations() const;
		bool empty() const;

		/// (1) Maps a node index to the position in the list
		/// (2) Maps a way index to the position in the list
		/// (3) Maps a relation index to the position in the list
		size_t getNodeIndex(int64_t id) const;
		size_t getWayIndex(int64_t id) const;
		size_t getRelationIndex(int64_t id) const;

		bool hasNodeIndex(int64_t id) const;
		bool hasWayIndex(int64_t id) const;
		bool hasRelationIndex(int64_t id) const;

		const OSMNode& getNode(int64_t id) const;
		const OSMWay& getWay(int64_t id) const;
		const OSMRelation& getRelation(int64_t id) const;

		int64_t findClosestNode(float lat, float lon) const;

		/// (1) Adds a new node to this map
		/// (2) Adds a new way to this map
		/// (3) Adds a new relation to this map
		bool addNode(const OSMNode& nd, bool updateBoundaries = true);
		bool addWay(const OSMWay& wd, const XMLMap& map, bool addChildren = true, bool updateBoundaries = true);
		bool addRelation(const OSMRelation& re, const XMLMap& map, bool addChildren = true, bool updateBoundaries = true);

		/// Finds all nodes that satisfy the given functions
		/// FuncNodes&& this function takes a const OSMNode& and returns a boolean
		///		that marks whether this node is accepted
		/// FuncWays&& this function takes a const OSMWay& and returns a boolean
		///		that marks whether this way is accepted
		template<typename FuncNodes, typename FuncWays, typename FuncWayNodes>//, typename AcceptRelation>
		XMLMap findNodes(
			FuncNodes&& funcNodes,
			FuncWays&& funcWays,
			FuncWayNodes&& funcWayNodes
			//AcceptRelation&& funcRel
		) const {
			XMLMap map;
			for (const OSMNode nd : (*nodeList)) {
				if (funcNodes(nd)) map.addNode(nd);
			}
			for (const OSMWay wd : (*wayList)) {
				if (funcWays(wd)) {
					auto st = std::make_shared<std::vector<int64_t>>();
					for (int64_t id : wd.getNodes()) {
						const OSMNode& nd = getNode(id);
						if (funcWayNodes(wd, nd)) st->push_back(id);
					}
					if (!st->empty()) {
						OSMWay w(wd.getID(), wd.getVer(), move(st), wd.getData());
						map.addWay(w, *this);
					}
				}
			}
			return map;
		}


		std::vector<int64_t> findAdress(
			const std::string& city, const std::string& postcode,
			const std::string& street, const std::string& housenumber) const;

		/// (1) Creates a tag list that contains all tags of nodes
		/// (2) Creates a way list that contains all tags of ways
		/// (3) Creates a tag list of all entities
		std::unordered_map<std::string, int32_t> createNodeTagList() const;
		std::unordered_map<std::string, int32_t> createWayTagList() const;
		std::unordered_map<std::string, int32_t> createTagList() const;

		/// (1) Finds all nodes that are located in a given rectangle
		/// (2) Finds all nodes that are located in a given rectangle
		XMLMap findSquareNodes(float lowerLat, float upperLat, float lowerLon, float upperLon) const;
		XMLMap findSquareNodes(const Rect& rect) const;
		/// (1) Finds all nodes that have the given tag
		/// (2) Finds all ways that have the given tag
		XMLMap findTagNodes(const std::string& tag) const;
		XMLMap findTagWays(const std::string& tag) const;

		XMLMap findCircleNode(const Circle& circle) const;

		/// (1) Returns the (const) node list
		/// (2) Returns the (const) way list
		/// (3) Returns the (const) relation list
		std::shared_ptr<std::vector<OSMNode>>& getNodes();
		std::shared_ptr<std::vector<OSMWay>>& getWays();
		std::shared_ptr<std::vector<OSMRelation>>& getRelations();

		const std::shared_ptr<std::vector<OSMNode>>& getNodes() const;
		const std::shared_ptr<std::vector<OSMWay>>& getWays() const;
		const std::shared_ptr<std::vector<OSMRelation>>& getRelations() const;

		void summary() const;

		/// (1) Returns the node map
		/// (2) Returns the way map
		/// (3) Returns the relation map
		inline std::shared_ptr<map_t> getNodeMap() { return nodeMap; }
		inline std::shared_ptr<map_t> getWayMap() { return wayMap; }
		inline std::shared_ptr<map_t> getRelationMap() { return relationMap; }
		inline Rect getRect() const { return Rect::fromBorders(lowerLat, upperLat, lowerLon, upperLon); }
	};
} // namespace traffic

// Experimental !!
namespace traffic
{
	struct StatusRequest {

	};

	struct AgentTransfer {

	};

	struct BorderChange {

	};

	struct DataTransfer {

	};

	class WorkerInterface {
		/// Checks the status of the Worker. The function will return
		/// true if this worker is active and successfully connected to
		/// the network. Returns false otherwise.
		virtual bool requestStatus(const StatusRequest& req) = 0;

		/// Requests to transfer an agent from the current worker to
		/// this worker. Returns true if the transfer was successfully.
		/// Returns false if the transfer failed.
		virtual bool requestAgentTransfer(const AgentTransfer& req) = 0;

		/// Requests to change the position of a border node. This cannot
		/// be done by the worker on its own because the coordination of
		/// both workers is needed to change it. Returns true if the change
		/// was successfully. Returns false otherwise.
		virtual bool requestBorderChange(const BorderChange& req) = 0;

		/// Requests to transfer data to this worker. This function may
		/// be used to transfer arbitrary data between the workers.
		virtual bool requestDataTransfer(const DataTransfer& rqeq) = 0;
	};

	class GlobalXMLMap {
	protected:
		std::vector<XMLMap> childMaps;
	};

	// Converts nodes to json files
	inline void to_json(json& j, const OSMNode& node) { node.toJson(j); }
	inline void from_json(const json& j, OSMNode& node) { node = OSMNode(j); }
	// Converts ways to json files
	inline void to_json(json& j, const OSMWay& map) { map.toJson(j); }
	inline void from_json(const json& j, OSMWay& map) { map = OSMWay(j); }
	// Converts relation members to json files
	inline void to_json(json& j, const RelationMember& opt) { opt.toJson(j); }
	inline void from_json(const json& j, RelationMember& opt) { opt = RelationMember(j); }
	// Converts relations to json files
	inline void to_json(json& j, const OSMRelation& opt) { opt.toJson(j); }
	inline void from_json(const json& j, OSMRelation& opt) { opt = OSMRelation(j); }
	// Converts XMLMapObjects to json files
	inline void to_json(json& j, const OSMMapObject& map) { map.toJson(j); }
	inline void from_json(const json& j, OSMMapObject& map) { map = OSMMapObject(j); }

	void debugTags(const XMLMap& map);
} // namespace traffic

#endif