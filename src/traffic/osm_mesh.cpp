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

#include "osm_mesh.h"
#include "geom.h"

#define USE_OPENGL

using namespace glm;
using namespace std;
using namespace traffic;

constexpr float Pi = 3.14159f;

vec2 traffic::sphereToPlane(vec2 latLon, vec2 center) {
	return vec2(
		(float)(latLon.x * cos((double)center.y * Pi / 180.0)),
		latLon.y
	);
}

glm::vec2 traffic::sphereToPlane(glm::vec2 latLon)
{
	return vec2(
		(float)(latLon.x * cos((double)latLon.y * Pi / 180.0)),
		latLon.y
	);
}


std::vector<glm::vec2> traffic::generateMesh(const XMLMap& map) {
	auto& nodeList = *(map.getNodes());
	std::vector<glm::vec2> points;

	Point centerP = map.getRect().getCenter();
	vec2 center(centerP.getLongitude(), centerP.getLatitude());

	for (const OSMWay& wd : (*map.getWays())) {
		auto& nds = wd.getNodes();
		if (nds.empty()) continue;

		int64_t lastNode = nds[0];
		for (size_t i = 1; i < nds.size(); i++) {
			size_t lastNodeID = map.getNodeIndex(lastNode);
			size_t currentNodeID = map.getNodeIndex(nds[i]);
			glm::vec2 pos1(
				static_cast<float>(nodeList[lastNodeID].getLon()),
				static_cast<float>(nodeList[lastNodeID].getLat()));
			glm::vec2 pos2(
				static_cast<float>(nodeList[currentNodeID].getLon()),
				static_cast<float>(nodeList[currentNodeID].getLat()));

			points.push_back(sphereToPlane(pos1, center));
			points.push_back(sphereToPlane(pos2, center));
			lastNode = nds[i];
		}
	}

	return points;
}

std::vector<glm::vec2> traffic::generateChunkMesh(const World& world)
{
	std::vector<glm::vec2> positions;
	glm::vec2 center = world.getMap()->getRect().getCenter().toVec();
	for (const WorldChunk& chunk : world.getChunks())
	{
		const Rect box = chunk.getBoundingBox();
		cout << box.summary() << endl;

		positions.push_back(sphereToPlane(box.latLlonL().toVec(), center));
		positions.push_back(sphereToPlane(box.latLlonH().toVec(), center));

		positions.push_back(sphereToPlane(box.latLlonL().toVec(), center));
		positions.push_back(sphereToPlane(box.latHlonL().toVec(), center));

		positions.push_back(sphereToPlane(box.latHlonH().toVec(), center));
		positions.push_back(sphereToPlane(box.latHlonL().toVec(), center));

		positions.push_back(sphereToPlane(box.latHlonH().toVec(), center));
		positions.push_back(sphereToPlane(box.latLlonH().toVec(), center));
	}
	return positions;
}

void traffic::unify(std::vector<glm::vec2>& points)
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
		points[i] += glm::vec2(-xMin, -yMin);
		points[i] /= scale;
	}
}

// ---- Shaders ---- //
const char * lineVert = R"(
#version 330
uniform mat4 mvp;

in vec2 vVertex;
in vec3 color;

out vec3 mixedColor;

void main(void)
{
	gl_Position = mvp * vec4(vVertex, 0.0, 1.0);
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
