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

#include <vector>
#include <limits>

#include <glm/glm.hpp>

#include "osm.h"
#include "agent.h"
#include "osm_mesh.h"
#include "geom.h"

#define USE_OPENGL

using namespace glm;
using namespace std;
using namespace traffic;

constexpr double Pi = 3.141592653589793238462643383279502;

// ---- Plane to Sphere ---- //

double traffic::planeToLatitude(double planeLat, dvec2 center)
{
	return planeLat / cos(center.y * Pi / 180.0);
}

double traffic::planeToLongitude(double planeLon, dvec2 center)
{
	return planeLon;
}

dvec2 traffic::planeToSphere(dvec2 latLon, dvec2 center)
{
	return dvec2(
		planeToLatitude(latLon.x, center),
		planeToLongitude(latLon.y, center)
	);
}

dvec2 traffic::planeToSphere(dvec2 latLon)
{
	return dvec2(
		planeToLatitude(latLon.x, latLon),
		planeToLongitude(latLon.y, latLon)
	);
}

// ---- Sphere to Plane ---- //

double traffic::latitudeToPlane(double lat, dvec2 center)
{
	return lat * cos(center.y * Pi / 180.0);
}

double traffic::longitudeToPlane(double lon, dvec2 center)
{
	return lon;
}

dvec2 traffic::sphereToPlane(dvec2 latLon, dvec2 center) {
	return dvec2(
		latitudeToPlane(latLon.x, center),
		longitudeToPlane(latLon.y, center)
	);
}

dvec2 traffic::sphereToPlane(dvec2 latLon) {
	return dvec2(
		latitudeToPlane(latLon.x, latLon),
		longitudeToPlane(latLon.y, latLon)
	);
}

double traffic::distance(dvec2 p1, dvec2 p2, double radius)
{
        double dLat = (p2.x - p1.x) * Pi / 180.0;
        double dLon = (p2.y - p1.y) * Pi / 180.0;
  
        // convert to radians 
        double lat1 = (p1.x) * Pi / 180.0;
        double lat2 = (p2.x) * Pi / 180.0;
  
        // apply formulae 
        double a = pow(sin(dLat / 2), 2) +  
                   pow(sin(dLon / 2), 2) *  
                   cos(lat1) * cos(lat2); 
        double c = 2.0 * asin(sqrt(a)); 
        return radius * c; 
}

// ---- Mesh Generation ---- //

void applyNodes(const std::vector<int64_t> nds, const OSMSegment& map,
	std::vector<glm::vec2> &points)
{
	if (nds.empty()) return;
	Point centerP = map.getBoundingBox().getCenter();
	vec2 center(centerP.getLongitude(), centerP.getLatitude());
	auto& nodeList = *(map.getNodes());

	int64_t lastNode = nds[0];
	for (size_t i = 1; i < nds.size(); i++)
	{
		size_t lastNodeID = map.getNodeIndex(lastNode);
		size_t currentNodeID = map.getNodeIndex(nds[i]);
		vec2 pos1(
			static_cast<float>(nodeList[lastNodeID].getLon()),
			static_cast<float>(nodeList[lastNodeID].getLat()));
		vec2 pos2(
			static_cast<float>(nodeList[currentNodeID].getLon()),
			static_cast<float>(nodeList[currentNodeID].getLat()));

		points.push_back(sphereToPlane(pos1, center));
		points.push_back(sphereToPlane(pos2, center));
		lastNode = nds[i];
	}
}

std::vector<vec2> traffic::generateMesh(const OSMSegment& map)
{
	std::vector<vec2> points;
	for (const OSMWay& wd : (*map.getWays())) {
		applyNodes(wd.getNodes(), map, points);
	}

	return points;
}

std::vector<vec2> traffic::generateChunkMesh(const World& world)
{
	std::vector<vec2> positions;
	vec2 center = world.getMap()->getBoundingBox().getCenter().toVec();
	/* // TODO
	for (const WorldChunk& chunk : world.getChunks())
	{
		const Rect box = chunk.getBoundingBox();

		positions.push_back(sphereToPlane(box.latLlonL().toVec(), center));
		positions.push_back(sphereToPlane(box.latLlonH().toVec(), center));

		positions.push_back(sphereToPlane(box.latLlonL().toVec(), center));
		positions.push_back(sphereToPlane(box.latHlonL().toVec(), center));

		positions.push_back(sphereToPlane(box.latHlonH().toVec(), center));
		positions.push_back(sphereToPlane(box.latHlonL().toVec(), center));

		positions.push_back(sphereToPlane(box.latHlonH().toVec(), center));
		positions.push_back(sphereToPlane(box.latLlonH().toVec(), center));
	}
	*/
	return positions;
}

std::vector<glm::vec2> traffic::generateRouteMesh(const Route route, const OSMSegment& map)
{
	std::vector<glm::vec2> points(route.nodes.size());
	applyNodes(route.nodes, map, points);
	return points;
}

void traffic::unify(std::vector<vec2>& points)
{
	float xMax = std::numeric_limits<float>::min();
	float xMin = std::numeric_limits<float>::max();
	float yMax = std::numeric_limits<float>::min();
	float yMin = std::numeric_limits<float>::max();
	for (const auto& p : points) {
		if (p.x > xMax) xMax = p.x;
		if (p.x < xMin) xMin = p.x;
		if (p.y > yMax) yMax = p.y;
		if (p.y < yMin) yMin = p.y;
	}

	float scale = std::max((xMax - xMin), (yMax - yMin));
	for (size_t i = 0; i < points.size(); i++) {
		points[i] += vec2(-xMin, -yMin);
		points[i] /= scale;
	}
}

// ---- Shaders ---- //
const char * lineVert = R"(
#version 330
#define MAT3 0

#if MAT3
uniform mat3 mvp;
#else
uniform mat4 mvp;
#endif

in vec2 vVertex;
in vec3 color;

out vec3 mixedColor;

void main(void)
{
#if MAT3
	gl_Position = vec4(mvp * vec3(vVertex, 0.0), 1.0);
#else
	gl_Position = mvp * vec4(vVertex, 0.0, 1.0);
#endif

	mixedColor = color;
})";

// Fragment shader
const char * lineFragment = R"(
#version 330
in vec3 mixedColor;

out vec4 color;

void main() {
    color = vec4(mixedColor, 1.0);
})";

const char* traffic::getLineVertex()
{
#if defined(USE_OPENGL)
	return lineVert;
#else
	return nullptr;
#endif
}
const char* traffic::getLineFragment()
{
#if defined(USE_OPENGL)
	return lineFragment;
#else
	return nullptr;
#endif
}

const char* chunkVert = R"(
#version 330
uniform mat4 mvp;

in vec2 vVertex;

void main(void)
{
	gl_Position = mvp * vec4(vVertex, 0.0, 1.0);
})";

const char* chunkFragment = R"(
#version 330
uniform vec4 color;

out vec4 outColor;

void main() {
    outColor = color;
})";

const char* traffic::getChunkVertex()
{
#if defined(USE_OPENGL)
	return chunkVert;
#else
	return nullptr;
#endif
}

const char* traffic::getChunkFragment()
{
#if defined(USE_OPENGL)
	return chunkFragment;
#else
	return nullptr;
#endif
}
