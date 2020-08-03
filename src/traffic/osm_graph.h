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

#ifndef GRAHP_HPP
#define GRAHP_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

#include "engine.h"
#include "osm.h"

namespace traffic
{
    /* Graph Representation */
	struct GraphEdge; // A single edge that is part of a larger graph
	struct GraphNode; // A node that is part of a larger graph
	struct Route; // Defines a route between two graph nodes
	class Graph; // Combines a list of GraphNodes in a network by GraphEdges
	

	/// <summary>
	/// Graph edges are used to connect GraphNodes with each other. The implemented
	/// graph is directed which means that connections are uni-directional. Each
	/// edge contains a weight value specifying the cost of taking this path. The
	/// cost may be calculated by different method but may never be negative. This
	/// may lead to unexpected consequences.
	/// </summary>
	struct GraphEdge
	{ 
		/// <summary>Creates an edge of a graph</summary>
		/// <param name="goalID">The destination where this edge leads</param>
		/// <param name="weight">The weight that is associated with this edge</param>
		/// <returns></returns>
		GraphEdge(int64_t goalID, prec_t weight);

		// ---- Member definitions ---- //
		int64_t goal;
		prec_t weight;
	};

	/// <summary>
	/// class GraphNode
	/// Graph nodes are used in a graph representation to perform route optimization.
	/// Each node has a certain set of connections that lead to other nodes creating
	/// a connected graph.
	/// </summary>
	struct GraphNode
	{
		/// <summary> Creates a GraphNode from a regular OSM node</summary>
		/// <param name="node">A regular OSM node</param>
		/// <returns></returns>
		GraphNode(const OSMNode &node);

		glm::vec2 getPosition() const;
		prec_t getLatitude() const;
		prec_t getLongitude() const;

		// ---- Member definitions ---- //
		prec_t lat, lon;
		int64_t nodeID;
		std::vector<GraphEdge> connections;
	};

	/// <summary>
	/// A route defines a way to navigate inside a Graph. It stores a sequential
	/// array of node IDs defining a way to navigate. Routes are produced by path
	/// finding algorithms.
	/// </summary>
	struct Route
	{
		Route() = default;

		bool exists() const;
		void addNode(int64_t nodeID);

		// ---- Member definitions ---- //
		std::vector<int64_t> nodes;
	};

	/// <summary>
	/// This class defines a directed graph that is essential a network of GraphNodes
	/// which is connected by GraphEdges. A graph may be created from an OSM map by
	/// converting the node, way, relation structure to a directed graph. The map
	/// owns an additional 
	/// </summary>
	class Graph
	{
	public:
		/// <summary>Creates a Graph from an OSM map object. The node, way, relation
		/// structure is converted to a directional graph. Each node that is enclosed
		/// by at least one way is appended as GraphNode to this graph</summary>
		/// <param name="xmlmap">The OSM map that is converted</param>
		/// <returns></returns>
		Graph(std::shared_ptr<XMLMap> &xmlmap);

		/// <summary>Applies the AStar (A*) path finding algorithm on the graph</summary>
		/// <param name="start">The starting node ID</param>
		/// <param name="goal">The destination node ID</param>
		/// <returns>The shortest route between start and goal</returns>
		Route findRoute(int64_t start, int64_t goal);

		/// <summary>Finds a node by its index in the sequential node array</summary>
		/// <param name="index">The node's index</param>
		/// <returns>The node at the given index</returns>
		GraphNode& findNodeByIndex(size_t index);
		const GraphNode& findNodeByIndex(size_t index) const;

		/// <summary>Finds a node by its ID. The ID is converted to an actual
		/// array position which is used to return the node.</summary>
		/// <param name="id">The node's ID</param>
		/// <returns>The node that is stored by the given ID</returns>
		GraphNode& findNodeByID(int64_t id);
		const GraphNode& findNodeByID(int64_t id) const;

		/// <summary>Converts a GraphNode ID to the index in the node array</summary>
		/// <param name="id">The GraphNode ID that is searched</param>
		/// <returns>The corresponding index or -1 if that node does not exist</returns>
		int64_t findNodeIndex(int64_t id) const;

		// ---- Getter functions ---- //

		std::map<int64_t, size_t>& getMap();
		std::vector<GraphNode>& getBuffer();
		std::shared_ptr<XMLMap> getXMLMap();

		const std::map<int64_t, size_t>& getMap() const;
		const std::vector<GraphNode>& getBuffer() const;
		std::shared_ptr<const XMLMap> getXMLMap() const;

		size_t countNodes() const;
		size_t countEdges() const;
		void clear();

		bool checkConsistency() const;

	protected:
		std::vector<GraphNode> graphBuffer;
		std::map<int64_t, size_t> graphMap;
		std::shared_ptr<XMLMap> xmlmap;
	};
}

#endif
