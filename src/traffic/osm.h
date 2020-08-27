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

#include "engine.h"

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

	private:
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
		int64_t getID() const noexcept;

		/// <summary> Returns the version of this OSM object</summary>
		/// <returns>The version of this object</returns>
		int32_t getVer() const noexcept;

		// ---- Tag functions ---- //

		/// <summary> Returns true whether the tag list contains a tag with the
		/// given name.</summary>
		/// <param name="key">The key which is searched in the map</param>
		/// <returns>True if the map contains a key with this tag, false otherwise</returns>
		bool hasTag(const std::string& key) const noexcept;

		/// <summary>Returns whether the tag list contains the given key-value pair. 
		/// </summary>
		/// <param name="key">Key of the key-value pair</param>
		/// <param name="value">Value of the key-value pair</param>
		/// <returns>True if the map contains the key-value pair, false otherwise</returns>
		bool hasTagValue(const std::string& key, const std::string& value) const noexcept;

		/// <summary>Returns the value of index by the given key. Raises an exception
		/// if the map does not contain the specific key</summary>
		/// <param name="key">The key used to index this value</param>
		/// <returns>The value index by this key</returns>
		std::string getValue(const std::string& key) const;

		/// <summary>Returns the key-value map in vector format</summary>
		/// <returns>A vector map that contains all key-value pairs</returns>
		std::shared_ptr<vector_map> getData() const noexcept;

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
		int32_t subIndex;

	public:
		/// <summary> Creates an empty way that does not contain any nodes</summary>
		/// <returns></returns>
		OSMWay();
		
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

		void clear() noexcept;
		void addNode(int64_t node) noexcept;

		int32_t getSubIndex() const;
		void setSubIndex(int32_t subIndex);

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
		int32_t subIndex;

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

		int32_t getSubIndex() const;
		void setSubIndex(int32_t subIndex);

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

	struct OSMFinder {
	public:
		std::function<bool(const OSMNode&)> acceptNode;
		std::function<bool(const OSMWay&)> acceptWay;
		std::function<bool(const OSMRelation&)> acceptRelation;

		std::function<bool(const OSMWay&, const OSMNode&)> acceptWayNodes;

		std::function<bool(const OSMRelation &, const OSMNode&)> acceptRelationNodes;
		std::function<bool(const OSMRelation &, const OSMWay&)> acceptRelationWays;
		std::function<bool(const OSMRelation &, const OSMRelation&)> acceptRelationRelations;

	public:
		OSMFinder();

		OSMFinder& setNodeAccept(std::function<bool(const OSMNode&)> accept);
		OSMFinder& setWayAccept(std::function<bool(const OSMWay&)> accept);
		OSMFinder& setRelationAccept(std::function<bool(const OSMRelation&)> accept);

		OSMFinder& setWayNodeAccept(std::function<bool(const OSMWay&, const OSMNode&)> accept);

		OSMFinder& setRelationNodeAccept(std::function<bool(const OSMRelation &, const OSMNode&)> accept);
		OSMFinder& setRelationWayAccept(std::function<bool(const OSMRelation &, const OSMWay&)> accept);
		OSMFinder& setRelationRelationAccept(std::function<bool(const OSMRelation &, const OSMRelation&)> accept);
	};

	/// This class represents a MapStructure. It combines all
	/// values stored in the OpenStreetMap XML format.
	/// nodeList		All nodes stored in the OSMSegment section
	/// wayList			All ways stored in the OSMSegment section
	/// relationList	All relations stored in the OSMSegment section
	/// float lowerLat	Min latitude value that is stored in this map
	/// float upperLat	Max latitude value that is stored in this map
	/// float lowerLon	Min longitude vlaue that is stored in this map
	/// float upperLon	Max longitude value that is stored in this map
	class OSMSegment
	{
	protected:
		/// <summary> Defines the bounding boxes of this map segment </summary>
		float lowerLat, upperLat, lowerLon, upperLon;

		using listnode_ptr_t = std::shared_ptr<std::vector<OSMNode>>;
		using listway_ptr_t = std::shared_ptr<std::vector<OSMWay>>;
		using listrelation_ptr_t = std::shared_ptr<std::vector<OSMRelation>>;

		listnode_ptr_t nodeList; // containts all nodes
		listway_ptr_t wayList; // contains all ways
		listrelation_ptr_t relationList; // contains all relations

		// (1) maps the node ids to indices in the node list
		// (2) maps the way ids to indices in the way list
		// (3) maps the relation ids to indices in the relation list
		//using map_t = std::unordered_map<int64_t, size_t>;
		//using map_t = robin_hood::unordered_node_map<int64_t, size_t>;
		std::shared_ptr<map_t> nodeMap;
		std::shared_ptr<mapid_t<std::vector<size_t>>> wayMap;
		std::shared_ptr<mapid_t<std::vector<size_t>>> relationMap;

	public:
		//// ---- Constructors ---- ////
		/// Creates a map that does not hold any data 
		explicit OSMSegment();
		explicit OSMSegment(const Rect &rect);
		explicit OSMSegment(const listnode_ptr_t& nodes,
			const listway_ptr_t& ways, const listrelation_ptr_t& relations);
		/// Creates a map that holds the passed data
		explicit OSMSegment(
			const listnode_ptr_t& nodes, const listway_ptr_t& ways,
			const listrelation_ptr_t& relations,
			const std::shared_ptr<map_t>& nodeMap,
			const std::shared_ptr<mapid_t<std::vector<size_t>>>& wayMap,
			const std::shared_ptr<mapid_t<std::vector<size_t>>>& relationMap);
		
		// ---- JSON interface ---- //
		explicit OSMSegment(const json& json);
		void toJson(json& json) const;

		OSMSegment(const OSMSegment&) = delete;
		OSMSegment(OSMSegment&&) = default;

		OSMSegment& operator=(const OSMSegment&) = delete;
		OSMSegment& operator=(OSMSegment&&) = default;

		~OSMSegment();
		
		// ---- evaluation functions ---- //

		void reindexMap(bool merge=false);
		void recalculateBoundaries();

		// ---- Size functions (inherited ---- //

		size_t getManagedSize() const;
		size_t getSize() const;


		/// (1) Returns whether this map has any nodes
		/// (2) Returns whether this map has any ways
		/// (3) Returns whether this map has any relations
		/// (4) Returns whether this map holds any objects
		bool hasNodes() const noexcept;
		bool hasWays() const noexcept;
		bool hasRelations() const noexcept;
		bool empty() const noexcept;

		/// (1) Maps a node index to the position in the list
		/// (2) Maps a way index to the position in the list
		/// (3) Maps a relation index to the position in the list
		size_t getNodeIndex(int64_t id) const;
		size_t getWayIndex(int64_t id) const; // Returns only the first entry
		size_t getRelationIndex(int64_t id) const; // Returns only the first entry

		const std::vector<size_t>& getWayIndices(int64_t id) const;
		const std::vector<size_t>& getRelationIndices(int64_t id) const;

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
		bool addNode(const OSMNode& nd);
		bool addWay(const OSMWay& wd);
		bool addRelation(const OSMRelation& re);

		bool addWayRecursive(const OSMWay &way, const OSMSegment& lookup);
		bool addRelationRecursive(const OSMRelation &re, const OSMSegment& lookup);

		/// Finds all nodes that satisfy the given functions
		/// FuncNodes&& this function takes a const OSMNode& and returns a boolean
		///		that marks whether this node is accepted
		/// FuncWays&& this function takes a const OSMWay& and returns a boolean
		///		that marks whether this way is accepted
		OSMSegment findNodes(const OSMFinder &finder) const;

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
		OSMSegment findSquareNodes(float lowerLat, float upperLat, float lowerLon, float upperLon) const;
		OSMSegment findSquareNodes(const Rect& rect) const;
		/// (1) Finds all nodes that have the given tag
		/// (2) Finds all ways that have the given tag
		OSMSegment findTagNodes(const std::string& tag) const;
		OSMSegment findTagWays(const std::string& tag) const;

		OSMSegment findCircleNode(const Circle& circle) const;

		/// (1) Returns the (const) node list
		/// (2) Returns the (const) way list
		/// (3) Returns the (const) relation list
		const std::shared_ptr<std::vector<OSMNode>>& getNodes() const noexcept;
		const std::shared_ptr<std::vector<OSMWay>>& getWays() const noexcept;
		const std::shared_ptr<std::vector<OSMRelation>>& getRelations() const noexcept;

		size_t getNodeCount() const noexcept;
		size_t getWayCount() const noexcept;
		size_t getRelationCount() const noexcept;

		void summary() const;

		/// (1) Returns the node map
		/// (2) Returns the way map
		/// (3) Returns the relation map
		const std::shared_ptr<map_t>& getNodeMap() const noexcept;
		const std::shared_ptr<mapid_t<std::vector<size_t>>>& getWayMap() const noexcept;
		const std::shared_ptr<mapid_t<std::vector<size_t>>>& getRelationMap() const noexcept;
		Rect getBoundingBox() const noexcept;
		void setBoundingBox(const Rect &r) noexcept;
	};

	struct OSMMapBuffer {
		uint32_t chunk;
	};

	class OSMMap
	{
	public:
		explicit OSMMap(const std::shared_ptr<OSMSegment>& map, prec_t chunkSize = 0.005);
		
		void insertSegment(const OSMSegment &segment);
		void recalculateChunks();

		const OSMSegment& getSegmentByNode(int64_t id) const;
		const OSMSegment& getSegment(prec_t lat, prec_t lon) const;
		const OSMNode& getNode(int64_t nodeID) const;
		const OSMWay& getWay(int64_t wayID) const;
		const OSMRelation& getRelation(int64_t relationID) const;

		size_t getSegmentIndexByNode(int64_t nodeID) const;
		const std::vector<size_t>& getSegmentIndexByWay(int64_t wayID) const;
		const std::vector<size_t>& getSegmentIndexByRelation(int64_t nodeID) const;
		size_t getSegmentIndex(prec_t lat, prec_t lon) const;

		/// (1) Adds a new node to this map
		/// (2) Adds a new way to this map
		/// (3) Adds a new relation to this map
		bool addNode(const OSMNode& nd);
		bool addWayRecursive(const OSMWay& way, const OSMSegment& lookup);
		bool addRelationRecursive(const OSMRelation& re, const OSMSegment& lookup);

		// ---- Coordinate transformation ---- //
		size_t latCoordToGlobal(prec_t coord) const;
		prec_t latGlobalToCoord(size_t global) const;
		size_t latLocalToGlobal(size_t local) const;
		size_t latGlobalToLocal(size_t global) const;
		size_t latCoordToLocal(prec_t coord) const;
		prec_t latLocalToCoord(size_t local) const;


		size_t lonCoordToGlobal(prec_t coord) const;
		prec_t lonGlobalToCoord(size_t global) const;
		size_t lonLocalToGlobal(size_t local) const;
		size_t lonGlobalToLocal(size_t global) const;
		size_t lonCoordToLocal(prec_t coord) const;
		prec_t lonLocalToCoord(size_t local) const;

		size_t toStore(prec_t lat, prec_t lon) const;
		size_t toStore(size_t localLat, size_t localLon) const;

		const std::vector<OSMSegment>& getChunks() const;
		size_t keyCheck(size_t index) const;

	protected:
		// ---- Member definitions ---- //
		mapid_t<size_t> m_nodemap;
		mapid_t<std::vector<size_t>> m_waymap;
		mapid_t<std::vector<size_t>> m_relationmap;
		std::vector<size_t> noValues;

		Rect boundingBox;
		prec_t m_chunkSize;
		std::vector<OSMSegment> m_chunks;
		size_t m_latChunks, m_lonChunks;
		size_t m_latOffset, m_lonOffset;
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
		std::vector<OSMSegment> childMaps;
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

	void debugTags(const OSMSegment& map);
} // namespace traffic

#endif