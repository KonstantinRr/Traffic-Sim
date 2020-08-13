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

#ifndef PARSER_HPP
#define PARSER_HPP

#include "engine.h"

#include <string>
#include <chrono>

#include "osm.h"

class thread_pool;
using taglist_t = std::vector<std::pair<std::string, std::string>>;

namespace traffic
{
	/// <summary>
	/// Stores the parser timings in a combined location
	/// </summary>
	struct ParseTimings
	{
		std::chrono::steady_clock::time_point
			begin, endRead, endXMLParse, endDataParse, end;

		/// <summary>Prints a detailed summary on the timings</summary>
		void summary();
	};

	struct ParseArguments
	{
		int threads = 8;
		std::string file = "map.xmlmap";
		ctpl::thread_pool *pool = nullptr;
		ParseTimings *timings = nullptr;
	};

	struct OSMNodeTemp
	{
		int64_t id; int32_t ver;
		std::shared_ptr<taglist_t> tags;
		prec_t lat, lon;
		
		OSMNodeTemp(int64_t id, int32_t ver,
			const std::shared_ptr<taglist_t> &tags,
			prec_t lat, prec_t lon) : id(id), ver(ver),
			tags(tags), lat(lat), lon(lon) { }
	};

	struct OSMWayTemp
	{
		int64_t id; int32_t ver;
		std::shared_ptr<taglist_t> tags;
		std::shared_ptr<std::vector<int64_t>> nodes;

		OSMWayTemp(int64_t id, int32_t ver,
			const std::shared_ptr<taglist_t>& tags,
			const std::shared_ptr<std::vector<int64_t>>& nodes) :
			id(id), ver(ver), tags(tags), nodes(nodes) { }
	};

	struct OSMRelationTemp
	{
		int64_t id; int32_t ver;
		std::shared_ptr<taglist_t> tags;
		std::shared_ptr<std::vector<RelationMember>> nodes;
		std::shared_ptr<std::vector<RelationMember>> ways;
		std::shared_ptr<std::vector<RelationMember>> relations;

		OSMRelationTemp(
			int64_t id, int32_t ver,
			const std::shared_ptr<taglist_t> &tags,
			const std::shared_ptr<std::vector<RelationMember>> &nodes,
			const std::shared_ptr<std::vector<RelationMember>> &ways,
			const std::shared_ptr<std::vector<RelationMember>> &relations) :
			id(id), ver(ver), tags(tags), nodes(nodes),
			ways(ways), relations(relations) { }
	};

	std::vector<unsigned char> writeXOSMMap(const OSMSegment &map, const std::string &file);
	OSMSegment parseXMLMap(const ParseArguments &args);
} // namespace traffic

#endif
