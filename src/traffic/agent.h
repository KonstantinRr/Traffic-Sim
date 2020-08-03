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

#ifndef AGENT_H
#define AGENT_H

#include <memory>

#include "osm_graph.h"

namespace traffic
{
    class Agent; // A single agent that takes part in the world
    class World; // The world the agent takes part of

    class Entity
    {
    public:
        virtual ~Entity() = default;
        virtual void update() = 0;

        bool isAlive() const;

    protected:
        bool alive;
    };

    /// <summary>
    /// Agents are entities that act in the world to achieve a certain goal. Each agent
    /// has its own set of believes desires and goals that it tries to achieve. Agents
    /// are generally selfish meaning they always want the best outcome for themselves.
    /// </summary>
    class Agent
    {
    public:
        // ---- Constructors ---- //
        Agent(const Agent&) = delete;
        Agent(Agent&&) = delete;

        Agent& operator=(const Agent&) = delete;
        Agent& operator=(Agent&&) = delete;

        virtual ~Agent() = default;

        // ---- Functions ---- //

        void setGoal(int64_t newGoal);
        int64_t getGoal() const;

        virtual void update();
        void makeGreedyChoice();

    protected:
        // ---- Member definitions ---- //
        std::shared_ptr<World> world;
        int64_t goalID;

        int64_t lastVisited;
        int64_t nextVisited;
    };

    class World
    {
    public:
        // ---- Contstructors ---- //
        World(const std::shared_ptr<Graph> &graph);

        // ---- Functions ---- //

        std::shared_ptr<Graph>& getGraph();
        std::vector<Agent>& getAgents();

    protected:
        // ---- Member definitions ---- //
        std::shared_ptr<Graph> graph;
        std::vector<Agent> agents;

    }; 
} // namespace traffic

#endif