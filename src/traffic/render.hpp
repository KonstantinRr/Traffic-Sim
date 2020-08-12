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

#ifndef TRAFFIC_RENDER_HPP
#define TRAFFIC_RENDER_HPP

#include "engine.h"

#include "graphics.hpp"
#include "osm.h"
#include "osm_graph.h"


namespace traffic {
    //lt::resource::MeshBuilder2D convertToMesh(const OSMSegment &map);
    enum FitSize {
		SCALE, FIT_WIDTH, FIT_HEIGHT
	};

	struct RenderParams {
		prec_t ratioLat, ratioLon, lowerLat, lowerLon;

		RenderParams(const Rect &r, FitSize fit, size_t width, size_t height);
		RenderParams(const OSMSegment &map, const lt::ImageRGB8 &image, FitSize fit);
	};

	void generateMap(const OSMSegment &map, const Rect &rect);
	void renderMap(const OSMSegment &map, prec_t containerSize);
	void drawRoute(const OSMSegment& map, const Route& route, lt::ImageRGB8 &img, const RenderParams &param);
	void drawMap(const OSMSegment& map, lt::ImageRGB8 &img, const RenderParams &param);
}

#endif
