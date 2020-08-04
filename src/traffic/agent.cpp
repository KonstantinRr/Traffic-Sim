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

#include "agent.h"

using namespace traffic;
using namespace glm;

// ---- Entity ---- //

bool Entity::isAlive() const { return alive; }

// ---- Agent ---- //

void Agent::setGoal(int64_t newGoal) { goalID = newGoal; }
int64_t Agent::getGoal() const { return goalID; }

void Agent::update() {

}

void Agent::makeGreedyChoice() {
    auto &goal = world->getGraph()->findNodeByID(goalID);
    auto &node = world->getGraph()->findNodeByID(nextVisited);
    
    vec2 goalVec = glm::normalize(node.getPosition() - goal.getPosition());

    int bestFit = -1;
    float bestDotProduct = 0;
    for (size_t i = 0; i < node.connections.size(); i++) {
        // Creates the junction vector for each connection
        int64_t junctionCrossID = node.connections[i].goal;
        auto &junctionCrossNode = world->getGraph()->findNodeByID(junctionCrossID);
        vec2 junctionCross = glm::normalize(
            junctionCrossNode.getPosition() - node.getPosition());

        // checks if the junction vector is better than
        // the current best one. Updates the values if so.
        float dotProduct = glm::dot(junctionCross, goalVec);
        if (dotProduct > bestDotProduct) {
            bestDotProduct = dotProduct;
            bestFit = static_cast<int>(i);
        }
    }

    // There is no way left. The agent is stuck
    if (bestFit == -1) {
        // TODO
    } else {
        // TODO
    }
}

// ---- WorldChunk ---- //

int eraseFast(std::vector<int64_t>& vector, int64_t val)
{
    int found = 0;
    for (size_t i = 0; i < vector.size(); i++) {
        while (vector[i] == val) {
            int64_t lastVal = vector.back();
            vector.pop_back();
            if (i < vector.size()) {
                vector[i] = lastVal;
            }
            found++;
        }
    }
    return found;
}

bool contains(const std::vector<int64_t>& vector, int64_t id)
{
    for (const int64_t check : vector) {
        if (check == id) return true;
    }
    return false;
}

traffic::WorldChunk::WorldChunk()
{
}

traffic::WorldChunk::WorldChunk(const Rect& rect)
    : boundingBox(rect)
{
}

const std::vector<int64_t> traffic::WorldChunk::getNodes() const { return m_nodes; }
const std::vector<int64_t> traffic::WorldChunk::getAgents() const { return m_agents; }
bool traffic::WorldChunk::containtsNode(int64_t id) const { return contains(m_nodes, id); }
bool traffic::WorldChunk::containsAgent(int64_t id) const { return contains(m_agents, id); }

void traffic::WorldChunk::addNode(int64_t node) { m_nodes.push_back(node); }
void traffic::WorldChunk::addAgent(int64_t agent) { m_agents.push_back(agent); }

int traffic::WorldChunk::removeNode(int64_t node) { return eraseFast(m_nodes, node); }
int traffic::WorldChunk::removeAgent(int64_t agent) { return eraseFast(m_agents, agent); }

void traffic::WorldChunk::clearNodes() { m_nodes.clear(); }
void traffic::WorldChunk::clearAgents() { m_agents.clear(); }

void traffic::WorldChunk::clear()
{
    clearNodes();
    clearAgents();
}

Rect traffic::WorldChunk::getBoundingBox() const { return boundingBox; }
void traffic::WorldChunk::setBoundingBox(const Rect& rect) { this->boundingBox = rect; }

// ---- Word ---- //

World::World(const std::shared_ptr<XMLMap>& map, prec_t chunkSize)
{
    this->m_chunkSize = chunkSize;
    this->m_map = map;
    this->m_graph = std::make_shared<Graph>(map);
    recalculateChunks();
}

const std::shared_ptr<XMLMap>& traffic::World::getMap() const { return m_map; }
const std::shared_ptr<Graph>& World::getGraph() const { return m_graph; }
const std::vector<Agent>& World::getAgents() const { return m_agents; }
const std::vector<WorldChunk>& traffic::World::getChunks() const { return m_chunks; }

size_t traffic::World::latCoordToGlobal(prec_t coord) const
{
    return (size_t)((coord + 90.0f) / m_chunkSize);
}

prec_t traffic::World::latGlobalToCoord(size_t global) const
{
    return (prec_t)(global * m_chunkSize) - 90.0f;
}

size_t traffic::World::latLocalToGlobal(size_t local) const
{
    return local + m_latOffset;
}

size_t traffic::World::latGlobalToLocal(size_t global) const
{
    return global - m_latOffset;
}

size_t traffic::World::latCoordToLocal(prec_t coord) const
{
    return latGlobalToLocal(latCoordToGlobal(coord));
}

prec_t traffic::World::latLocalToCoord(size_t local) const
{
    return latGlobalToCoord(latLocalToGlobal(local));
}

// Longitude //

size_t traffic::World::lonCoordToGlobal(prec_t coord) const
{
    return (size_t)((coord + 180.0f) / m_chunkSize);
}

prec_t traffic::World::lonGlobalToCoord(size_t global) const
{
    return (prec_t)(global * m_chunkSize) - 180.0f;
}

size_t traffic::World::lonLocalToGlobal(size_t local) const
{
    return local + m_lonOffset;
}

size_t traffic::World::lonGlobalToLocal(size_t global) const
{
    return global - m_lonOffset;
}

size_t traffic::World::lonCoordToLocal(prec_t coord) const
{
    return lonGlobalToLocal(lonCoordToGlobal(coord));
}

prec_t traffic::World::lonLocalToCoord(size_t local) const
{
    return lonGlobalToCoord(lonLocalToGlobal(local));
}

size_t traffic::World::toStore(size_t localLat, size_t localLon) const
{
    return localLon * m_latChunks + localLat;
}


void traffic::World::recalculateChunks()
{
    const Rect rect = m_map->getRect();
    m_latOffset = latCoordToGlobal(rect.lowerLatBorder());
    m_lonOffset = lonCoordToGlobal(rect.lowerLonBorder());

    m_latChunks = latCoordToGlobal(rect.upperLatBorder()) - m_latOffset + 1;
    m_lonChunks = lonCoordToGlobal(rect.upperLonBorder()) - m_lonOffset + 1;
    m_chunks = std::vector<WorldChunk>(m_latChunks * m_lonChunks);
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

    for (const OSMNode& nd : *(m_map->getNodes()))
    {
        size_t location = toStore(nd.getLat(), nd.getLon());
        m_chunks[location].addNode(nd.getID());
    }
}

size_t traffic::World::toStore(prec_t lat, prec_t lon) const
{
    // Maps each chunk to a unique index.
    size_t localLat = latCoordToLocal(lat);
    size_t localLon = lonCoordToLocal(lon);

    return toStore(localLat, localLon);
}
