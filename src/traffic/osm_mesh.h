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

#ifndef OSM_MESH_H
#define OSM_MESH_H

#include "engine.h"

#include <glm/glm.hpp>

#include "osm.h"
#include "agent.h"

namespace traffic
{
    glm::vec2 sphereToPlane(glm::vec2 latLon, glm::vec2 center);
    std::vector<glm::vec2> generateMesh(const XMLMap& map);
    std::vector<glm::vec2> generateChunkMesh(const World& world);
    void unify(std::vector<glm::vec2> &points);

    const char * getLineVertex();
    const char * getLineFragment();

    const char * getChunkVertex();
    const char * getChunkFragment();
}

#endif
