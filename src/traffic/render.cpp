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

#include "render.hpp"

using namespace traffic;
using namespace lt;

/// <summary>
/// Renders a list of nodes on an image.
/// </summary>
/// <param name="map">The map that is used as lookup for the nodes</param>
/// <param name="nds">The node IDs that are rendered</param>
/// <param name="param">The render settings</param>
/// <param name="img">The image that is rendered on</param>
/// <param name="color">The color that is used to draw the line</param>
void drawNodeList(
	const OSMSegment& map,
	const std::vector<int64_t>& nds,
	const RenderParams& param,
	ImageRGB8& img,
	Color color
) {
	// does nothing if the list is empty
	if (nds.empty()) return;

	vector<OSMNode>& nodeList = *(map.getNodes());
	int64_t lastNode = nds[0];
	for (size_t i = 1; i < nds.size(); i++) {
		size_t lastNodeID = map.getNodeIndex(lastNode);
		size_t currentNodeID = map.getNodeIndex(nds[i]);
		ImgPoint x1(
			(int64_t)((nodeList[lastNodeID].getLon() - param.lowerLon) * param.ratioLon),
			(int64_t)((nodeList[lastNodeID].getLat() - param.lowerLat) * param.ratioLat)
		);
		ImgPoint x2(
			(int64_t)((nodeList[currentNodeID].getLon() - param.lowerLon) * param.ratioLon),
			(int64_t)((nodeList[currentNodeID].getLat() - param.lowerLat) * param.ratioLat)
		);
		img.drawLine(
			x1, x2,
			color,
			1, 1 // radius, accuracy
		);
		lastNode = nds[i];
	}
}


RenderParams::RenderParams(
	const Rect &r, FitSize fit, size_t width, size_t height)
{
	// longitude x / latitude y
	switch (fit) {
		case FIT_WIDTH:
			ratioLon = static_cast<prec_t>(width) / r.lonDistance();
			ratioLat = ratioLon;
			lowerLat = r.latCenter() - (height / 2) / ratioLat;
			lowerLon = r.lowerLonBorder();
			break;
		case FIT_HEIGHT:
			ratioLat = static_cast<prec_t>(height) / r.latDistance();
			ratioLon = ratioLat;
			lowerLat = r.lowerLatBorder();
			lowerLon = r.lonCenter() - (width / 2) / ratioLon;
			break;
		default:
		case FIT_BOTH:
			ratioLat = static_cast<prec_t>(height) / r.lonDistance();
			ratioLon = static_cast<prec_t>(width) / r.lonDistance();
			lowerLat = r.lowerLatBorder();
			lowerLon = r.lowerLonBorder();
			break;
	}
}
RenderParams::RenderParams(
	const OSMSegment &map, const ImageRGB8 &img, FitSize fit)
	: RenderParams(
		map.getBoundingBox(), fit,
		img.getXExtent(), img.getYExtent()
	) {}


void traffic::drawRoute(
	const OSMSegment &map, const Route &route,
	ImageRGB8 &img, const RenderParams &param)
{
	Color col(0.0, 0.0, 1.0, 1.0);
	drawNodeList(map, route.nodes, param, img, col);
}

void traffic::drawMap(
	const OSMSegment& map, ImageRGB8 &img,
	const RenderParams &param)
{
	// does nothing if the map does not contain any nodes
	if (!map.hasNodes()) return;

	Color col(0.9, 0.9, 0.9, 1.0);
	for (const OSMWay& wd : (*map.getWays())) {
		auto& nds = wd.getNodes();
		drawNodeList(map, nds, param, img, col);
	}
}
