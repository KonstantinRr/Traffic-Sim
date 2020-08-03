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

std::shared_ptr<Graph>& World::getGraph() { return graph; }
std::vector<Agent>& World::getAgents() { return agents; }