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

#ifndef CORE_HPP
#define CORE_HPP

#include "engine.h"

#include <string>
#include <glm/glm.hpp>

namespace traffic
{
	class Point;	// basic point defined by latitude and longitude
	class Distance; // distance between two points defined by d-latitude and d-longitude
	class Circle;	// circle definition using a center point and a radius
	class Rect;		// rectangle defined using a latitude and longitude

	/// Represents a point using the latitude and longitude
	/// format. Points can be created using coordinates or
	/// by using a distance.
	class Point
	{
	protected:
		prec_t lat, lon;

	public:
		/// (1) Creates a point at the coordinates (0.0, 0.0)
		/// (2) Creates a point using the given distance
		/// (3) Creates a point using the given latitude and longitude
		explicit Point();
		explicit Point(Distance distance);
		explicit Point(prec_t lat, prec_t lon);

		/// (1) Returns the latitude of this point
		/// (2) Returns the longitude of thuis point
		prec_t getLatitude() const;
		prec_t getLongitude() const;
		/// (1) Adds a distance from this point and returns the new point
		/// (2) Subtracts a distance from this point and returns the new point
		Point operator+(const Distance& point) const;
		Point operator-(const Distance& point) const;

		Distance distanceTo(const Point& point) const;
		glm::vec2 toVec() const;
	};

	/// The distance class describes a distance using
	/// the latitude and longitude coordinates.
	class Distance
	{
	protected:
		prec_t latLength, lonLength;

	public:
		/// (1) Creates a distance that spans (0.0, 0.0)
		/// (2) Creates a distance using the point parameters
		/// (3) Creates a distance between to points
		/// (4) Creates a distance using the latitude and longitude difference
		explicit Distance();
		explicit Distance(Point point);
		explicit Distance(Point p1, Point p2);
		explicit Distance(prec_t latLength, prec_t lonLength);

		/// (1) Returns the difference in latitude
		/// (2) Returns the difference in longitude
		prec_t getLatLength() const;
		prec_t getLonLength() const;
		Distance& performScaleLat(prec_t scale);
		Distance& performScaleLon(prec_t scale);
		Distance& performScale(prec_t latScale, prec_t lonScale);
		Distance& performScale(prec_t scale);

		Distance scaleLat(prec_t scale) const;
		Distance scaleLon(prec_t scale) const;
		Distance scale(prec_t latScale, prec_t lonScale) const;
		Distance scale(prec_t scale) const;

		prec_t getLength() const;
		prec_t getLengthSquared() const;
		Distance operator+(const Distance& dist) const;
		Distance operator-(const Distance& dist) const;
		Distance operator*(prec_t scale) const;
	};

	class Rect
	{
	protected:
		Point center;
		prec_t latLength, lonLength;

	public:
		/// Creates that most tightly encloses the given values
		/// prec_t lowerLat: Lower latitude boundary
		/// prec_t upperLat: Upper latitude boundary
		/// prec_t lowerLon: Lower longitude boundary
		/// prec_t upperLon: Upper longitude boundary
		static Rect fromBorders(prec_t lowerLat, prec_t upperLat, prec_t lowerLon, prec_t upperLon);
		static Rect fromLength(prec_t lowerLat, prec_t lowerLon, prec_t latLength, prec_t lonLength);
		
		/// Creates a rect from a center point and the distances from
		/// the center to the sides.
		/// Point center: Center point of this rect
		/// prec_t latLength: Distance from the center to the latitude border
		/// prec_t lonLength: Distance from the center to the longitude border
		static Rect fromCenter(Point center, prec_t latLength, prec_t lonLength);
		/// Creates a rect from a center point and the distances from
		/// the center to the sides. This function has the same logic as
		/// fromCenter(Point, prec_t, prec_t)
		static Rect fromCenter(prec_t centerLat, prec_t centerLon, prec_t latLength, prec_t lonLength);
		/// Creates a rect that most tightly encloses a circle.
		static Rect fromCircle(const Circle& circle);


		/// (1) Creates a rect that is equal to fromBorders(0, 0, 0, 0)
		explicit Rect();
		explicit Rect(Point center, prec_t latLength, prec_t lonLength);
		prec_t getLatLength() const;
		prec_t getLonLength() const;
		Point latHlonH() const;
		Point latHlonL() const;
		Point latLlonH() const;
		Point latLlonL() const;

		inline Point latHCenter() const { return center + Distance(latLength, 0); }
		inline Point latLCenter() const { return center + Distance(-latLength, 0); }
		inline Point lonHCenter() const { return center + Distance(0, lonLength); }
		inline Point lonLCenter() const { return center + Distance(0, -lonLength); }

		inline prec_t lowerLatBorder() const { return center.getLatitude() - latLength; }
		inline prec_t upperLatBorder() const { return center.getLatitude() + latLength; }
		inline prec_t lowerLonBorder() const { return center.getLongitude() - lonLength; }
		inline prec_t upperLonBorder() const { return center.getLongitude() + lonLength; }
		inline prec_t latCenter() const { return center.getLatitude(); }
		inline prec_t lonCenter() const { return center.getLongitude(); }
		inline prec_t latDistance() const { return 2 * latLength; }
		inline prec_t lonDistance() const { return 2 * lonLength;}

		std::string summary() const;

		Rect& performScaleLat(prec_t scale);
		Rect& performScaleLon(prec_t scale);
		Rect& performScale(prec_t latScale, prec_t lonScale);
		Rect& performScale(prec_t scale);

		Rect scaleLat(prec_t scale) const;
		Rect scaleLon(prec_t scale) const;
		Rect scale(prec_t latScale, prec_t lonScale) const;
		Rect scale(prec_t scale) const;

		bool containsLon(Point p) const;
		bool containsLat(Point p) const;
		bool contains(Point p) const;
		inline Point getCenter() const { return center; }
	};

	class Circle
	{
	protected:
		Point center;
		prec_t radiusLat, radiusLon;

	public:
		explicit Circle();
		explicit Circle(const Point& center, prec_t radius);
		explicit Circle(const Point& center, prec_t radiusLat, prec_t radiusLon);

		Circle& performLatScale(prec_t scale);
		Circle& performLonScale(prec_t scale);
		Circle& performScale(prec_t latScale, prec_t lonScale);
		Circle& performScale(prec_t scale);

		Circle scaleLat(prec_t scale) const;
		Circle scaleLon(prec_t scale) const;
		Circle scale(prec_t latScale, prec_t lonScale) const;
		Circle scale(prec_t scale) const;

		bool contains(const Point& p) const;
		Point getCenter() const;
		prec_t getLatRadius() const;
		prec_t getLonRadius() const;
	};
} // namespace traffic

#endif
