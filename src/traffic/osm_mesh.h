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
#include <vector>

namespace traffic
{
    // ---- Forward declarations ---- //
    class OSMSegment;
    class Word;

    // ---- Plane to Sphere ---- //

    /// <summary>Transforms a plane value to a sphere latitude value</summary>
    /// <param name="planeLat">The plane value</param>
    /// <param name="center">The center of the viewport that was used</param>
    /// <returns>The original latitude in sphere coordinates</returns>
    double planeToLatitude(double planeLat, glm::dvec2 center);
    /// <summary>Transforms a plane value to a sphere longitude value</summary>
    /// <param name="planeLon">The plane value</param>
    /// <param name="center">The center of the viewport that was used</param>
    /// <returns>The original longitude in sphere coordinates</returns>
    double planeToLongitude(double planeLon, glm::dvec2 center);

    glm::dvec2 planeToSphere(glm::dvec2 latLon, glm::dvec2 center);
    glm::dvec2 planeToSphere(glm::dvec2 latLon);
    
    // ---- Sphere to Plane ---- //

    /// <summary>Transforms a latitude value to a plane value</summary>
    /// <param name="lat">The latitude in sphere coordinates</param>
    /// <param name="center">The center of the viewport</param>
    /// <returns>A plane value that takes the distortion into account</returns>
    double latitudeToPlane(double lat, glm::dvec2 center);
    /// <summary>Transforms a longitude value to a plane value</summary>
    /// <param name="lon">The longitude in sphere coordinates</param>
    /// <param name="center">The center of the viewport</param>
    /// <returns>A plane value that takes the distortion into account</returns>
    double longitudeToPlane(double lon, glm::dvec2 center);

    glm::dvec2 sphereToPlane(glm::dvec2 latLon, glm::dvec2 center);
    glm::dvec2 sphereToPlane(glm::dvec2 latLon);

    // ---- Mesh Generation ---- //
    std::vector<glm::vec2> generateMesh(const OSMSegment& map);
    std::vector<glm::vec2> generateChunkMesh(const World& world);
    void unify(std::vector<glm::vec2> &points);

    // ---- Shaders ---- //

    const char * getLineVertex();
    const char * getLineFragment();

    const char * getChunkVertex();
    const char * getChunkFragment();
}

#endif
