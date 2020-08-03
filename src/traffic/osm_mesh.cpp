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

#include <vector>
#include <limits>

#include "osm_mesh.h"
#include "geom.h"
#include <glm/glm.hpp>

using namespace glm;
using namespace traffic;

constexpr float Pi = 3.14159f;

vec2 traffic::sphereToPlane(vec2 latLon, vec2 center) {
	return vec2(
		latLon.x * cos(center.y * Pi / 180.0),
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
				nodeList[lastNodeID].getLon(),
				nodeList[lastNodeID].getLat());
			glm::vec2 pos2(
				nodeList[currentNodeID].getLon(),
				nodeList[currentNodeID].getLat());

			points.push_back(sphereToPlane(pos1, center));
			points.push_back(sphereToPlane(pos2, center));
			lastNode = nds[i];
		}
	}

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

	return points;
}
