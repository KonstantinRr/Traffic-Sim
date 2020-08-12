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

#include "osm_graph.h"

#include <limits>
#include <queue>

using namespace traffic;
using namespace glm;
using namespace std;

// ---- BufferedNode ---- //

/// <summary>
/// class BufferedNode
/// BufferedNodes are used in path finding algorithms to store additional information
/// about GraphNodes. This includes a visited flag and a previous pointer that marks
/// the location from which the node was discovered. It also holds a distance specifying
/// the total distance from the source.
/// </summary>
class BufferedNode
{
public:
	BufferedNode() = default;
	BufferedNode(GraphNode* node, BufferedNode* previous, prec_t distance, bool visited);

	// ---- Member definitions ---- //
	GraphNode* node;
	BufferedNode* previous;
	prec_t distance;
	bool visited;
};

BufferedNode::BufferedNode(
	GraphNode *pNode, BufferedNode *pPrevious, prec_t pDistance, bool pVisited)
{
	this->node = pNode;
	this->previous = pPrevious;
	this->distance = pDistance;
	this->visited = pVisited;
}

// ---- GraphNode ---- //

GraphNode::GraphNode(const OSMNode &node)
{
	this->lat = node.getLat();
	this->lon = node.getLon();
	this->nodeID = node.getID();
}

bool traffic::GraphNode::hasManagedSize() const { return true; }
size_t traffic::GraphNode::getManagedSize() const
{
	return getSizeOfObjects(connections);
}

size_t traffic::GraphNode::getSize() const
{
	return sizeof(*this) + getManagedSize();
}

vec2 GraphNode::getPosition() const { return vec2(lon, lat); }
prec_t GraphNode::getLatitude() const { return lat; }
prec_t GraphNode::getLongitude() const { return lon; }

// ---- GraphEdge ---- //

GraphEdge::GraphEdge(int64_t pGoalID, prec_t pWeight) {
	this->goal = pGoalID;
	this->weight = pWeight;
}

size_t traffic::GraphEdge::getSize() const { return sizeof(*this); }

// ---- Route ---- //

bool Route::exists() const {
	return !nodes.empty();
}

void Route::addNode(int64_t nodeID) {
	nodes.push_back(nodeID);
}

// ---- Graph ---- //

Graph::Graph(const shared_ptr<OSMSegment>& xmlmap)
{
	this->xmlmap = xmlmap;
	// Iterates throught the whole list of ways, and nodes for each way. The algorithm checks
	// constantly if the node already exists in the map. It connects the nodes by creating
	// a new edge.

	for (const OSMWay& way : (*xmlmap->getWays()))
	{
		int64_t lastID = -1;

		for (const int64_t& currentID : way.getNodes())
		{
			// Checks whether the ID was found before
			auto indexIt = graphMap.find(currentID);
			if (indexIt == graphMap.end())
			{
				size_t currentIndex = graphBuffer.size();
				// (1) Inserts the value in the buffer
				// (2) Creates a new map entry
				graphBuffer.push_back(GraphNode(xmlmap->getNode(currentID)));
				graphMap[currentID] = currentIndex;

				if (lastID != -1)
				{
					size_t lastIndex = graphMap[lastID];
					prec_t distance = (prec_t)glm::distance(
						xmlmap->getNode(lastID).asVector(),
						xmlmap->getNode(currentID).asVector());
					graphBuffer[currentIndex].connections.push_back(GraphEdge(lastID, distance));
					graphBuffer[lastIndex].connections.push_back(GraphEdge(currentID, distance));
				}
			}
			else
			{
				// Connect the points together if there is
				// a valid last point that can be connected.
				if (lastID != -1)
				{
					size_t currentIndex = graphMap[currentID];
					size_t lastIndex = graphMap[lastID];
					prec_t distance = (prec_t)glm::distance(
						xmlmap->getNode(lastID).asVector(),
						xmlmap->getNode(currentID).asVector());
					graphBuffer[currentIndex].connections.push_back(GraphEdge(lastID, distance));
					graphBuffer[lastIndex].connections.push_back(GraphEdge(currentID, distance));
				}
			}
			lastID = currentID;
		}
	}
}

Route Graph::findRoute(int64_t start, int64_t goal)
{
	// Initializes the buffered data using an empty list
	size_t nodeCount = graphBuffer.size();
	vector<BufferedNode> nodes(nodeCount);
	for (size_t i = 0; i < nodeCount; i++) {
		nodes[i].distance = std::numeric_limits<double>::max();
		nodes[i].visited = false;
		nodes[i].previous = nullptr;
		nodes[i].node = &(graphBuffer[i]);
	}

	// defines the min priority queue
	auto cmp = [](const BufferedNode* left, const BufferedNode* right)
		{ return left->distance > right->distance; };
	priority_queue<BufferedNode*,
		vector<BufferedNode*>, decltype(cmp)> queue(cmp);

	size_t startIndex = graphMap[start];
	nodes[startIndex].distance = 0;
	queue.push(&(nodes[startIndex]));

	while (true)
	{
		// All possible connections where searched and the goal was not found.
		// This means that there is not a possible way to reach the destination node.
		if (queue.empty())
			return Route();

		// Takes the first element
		BufferedNode* currentNode = queue.top();
		queue.pop();


		// Checks the goal condition. Starts the backpropagation
		// algorithm if the goal was found to output the shortest
		// route.
		if (currentNode->node->nodeID == goal)
		{
			Route route;
			do {
				route.addNode(currentNode->node->nodeID);
				currentNode = currentNode->previous;
			} while (currentNode->node->nodeID != start);
			return route;
		}

		auto& connections = currentNode->node->connections;
		for (size_t i = 0; i < connections.size(); i++)
		{
			size_t nodeIndex = graphMap[connections[i].goal];

			// Checks if the node was already visited
			BufferedNode* nextNode = &(nodes[nodeIndex]);
			if (nextNode->visited) continue;

			// Updates the distance
			prec_t newDistance = currentNode->distance
				+ connections[i].weight;
			if (newDistance < nextNode->distance)
			{
				nextNode->distance = newDistance;
				nextNode->previous = currentNode;
			}

			// Adds the node to the list of nodes that need to
			// be visited. The node will be visited in one of the
			// next iterations
			queue.push(nextNode);
		}

		currentNode->visited = true;
	}
}

GraphNode& Graph::findNodeByIndex(size_t index) { return graphBuffer[index]; }
GraphNode& Graph::findNodeByID(int64_t id) {
	int64_t index = findNodeIndex(id);
	if (index == -1) throw "Could not find node ID";
	return graphBuffer[static_cast<size_t>(index)];
}
const GraphNode& Graph::findNodeByIndex(size_t index) const { return graphBuffer[index]; }
const GraphNode& Graph::findNodeByID(int64_t id) const {
	int64_t index = findNodeIndex(id);
	if (index == -1) throw "Could not find node ID";
	return graphBuffer[static_cast<size_t>(index)];
}

int64_t Graph::findNodeIndex(int64_t id) const {
	auto it = graphMap.find(id);
	return graphMap.end() == it ? -1 : it->second;
}

graphmap_t& Graph::getMap() { return graphMap; }
std::vector<GraphNode>& Graph::getBuffer() { return graphBuffer; }
std::shared_ptr<OSMSegment> Graph::getXMLMap() { return xmlmap; }

const graphmap_t& Graph::getMap() const { return graphMap; }
const std::vector<GraphNode>& Graph::getBuffer() const { return graphBuffer; }
std::shared_ptr<const OSMSegment> Graph::getXMLMap() const { return xmlmap; }

size_t Graph::countNodes() const {
	return graphBuffer.size();
}

size_t Graph::countEdges() const {
	// Counting edges
	size_t sum = 0;
	for (auto& node : graphBuffer)
		sum += node.connections.size();
	return sum;
}

void Graph::clear() {
	graphBuffer.clear();
	graphMap.clear();
}

bool Graph::checkConsistency() const {
	printf("Checking graph consistency: %d Nodes\n", graphBuffer.size());

	// Counting edges
	size_t sum = 0;
	for (auto &node : graphBuffer)
		sum += node.connections.size();
	printf("Edges: %d\n", sum);

	
	// checking buffer consistency
	bool check = true;
	for (size_t i = 0; i < graphBuffer.size(); i++) {
		auto checkIndex = graphMap.find(graphBuffer[i].nodeID);
		if (checkIndex == graphMap.end()) {
			printf("Could not find nodeID in map. INDEX: %d ID: %d\n",
				i, graphBuffer[i].nodeID);
			check = false;
			continue;
		}
		if (checkIndex->second != i) {
			printf("Map index does not match buffer index. Buffer: %d Map: %d\n",
				i, checkIndex->second);
			check = false;
			continue;
		}
		if (xmlmap->hasNodeIndex(checkIndex->second)) {
			printf("OSMNode does not exist in OSMSegment: %d\n", checkIndex->second);
			check = false;
			continue;
		}

		// Check the connections
		for (size_t k = 0; k < graphBuffer[i].connections.size(); k++) {
			auto connCheck = graphMap.find(graphBuffer[i].connections[k].goal);
			if (connCheck == graphMap.end()) {
				printf("Connection NodeID is not part of NodeMap. ID: %d\n",
					graphBuffer[i].connections[k].goal);
				check = false;
				continue;
			}
		}
	}

	// checking map consistency
	for (auto& it: graphMap) {
		// Do stuff
		if (it.second >= graphBuffer.size()) {
			printf("GraphMap value out of range %d\n", it.second);
			check = false;
			continue;
		}
	}

	printf("Graph consistency check computed %d\n", check);
	return true;
}

bool traffic::Graph::hasManagedSize() const { return true; }
size_t traffic::Graph::getManagedSize() const
{
	return getSizeOfObjects(graphBuffer);
}

size_t traffic::Graph::getSize() const
{
	return size_t();
}
