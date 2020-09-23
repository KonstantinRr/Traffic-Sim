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

#include "../engine/graphics.hpp"
#include "osm.h"
#include "osm_graph.h"


namespace traffic
{
    /// <summary> Defines the style how the latitude and longitude values are
	/// mapped to an image that does not necessarily has the same aspect ratio.</summary>
    enum FitSize {
		FIT_BOTH, // Fits the width and the height. May lead to distortion
		FIT_WIDTH, // Fits the width and adapts the height.
		FIT_HEIGHT // Fits the height and adapts the width.
	};

	struct RenderParams {
		prec_t ratioLat, ratioLon, lowerLat, lowerLon;

		/// <summary>Creates the render params using a bounding box, render style and
		/// the image dimensions. </summary>
		/// <param name="r">The bounding box that should later be rendered.</param>
		/// <param name="fit">The render style</param>
		/// <param name="width">The image dimension</param>
		/// <param name="height">The image dimension</param>
		RenderParams(const Rect &r, FitSize fit, size_t width, size_t height);
		RenderParams(const OSMSegment &map, const lt::ImageRGB8 &image, FitSize fit);
	};

	/// <summary>
	/// Draws a route to an image. The render settings are specified by the RenderParams.
	/// </summary>
	/// <param name="map">The map that is used as lookup for the route IDs</param>
	/// <param name="route">The route that is rendered on the page</param>
	/// <param name="img">The image that is rendered on</param>
	/// <param name="param">Additional render settings that specify the image boundaries</param>
	void drawRoute(const OSMSegment& map, const Route& route, lt::ImageRGB8 &img, const RenderParams &param);

	/// <summary>
	/// Renders a complete map to an image. The render settings are specified by the RenderParams.
	/// </summary>
	/// <param name="map">The map that is rendered to the image</param>
	/// <param name="img">The image the map is rendered on</param>
	/// <param name="param">Additional render settings that specify the image boundaries</param>
	void drawMap(const OSMSegment& map, lt::ImageRGB8 &img, const RenderParams &param);
} // trafficc

#endif
