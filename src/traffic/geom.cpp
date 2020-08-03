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

#include <math.h>
#include <algorithm>

#include "geom.h"

using namespace std;
using namespace traffic;

Point::Point() : lat(0.0), lon(0.0) { }
Point::Point(Distance distance)
	: lat(distance.getLatLength()), lon(distance.getLonLength()) { }
Point::Point(prec_t _lat, prec_t _lon)
	: lat(_lat), lon(_lon) { }

prec_t Point::getLatitude() const { return lat; }
prec_t Point::getLongitude() const { return lon; }

Point Point::operator+(const Distance& point) const
{ return Point(lat + point.getLatLength(), lon + point.getLonLength()); }
Point Point::operator-(const Distance& point) const
{ return Point(lat - point.getLatLength(), lon - point.getLonLength()); }

Distance Point::distanceTo(const Point& point) const
{ return Distance(lat - point.getLatitude(), lon - point.getLongitude()); }

glm::vec2 traffic::Point::toVec() const
{
	return glm::vec2(lon, lat);
}

/*
XMLMap XMLMap::findCircleNode(const Circle &circle)
{
	findNodes([&](const OSMNode& nd) {
		return circle.contains(Point(nd.getLat(), nd.getLon()));
		});
}
*/

Rect Rect::fromBorders(prec_t lowerLat, prec_t upperLat, prec_t lowerLon, prec_t upperLon)
{
	prec_t diffLat2 = (upperLat - lowerLat) / 2;
	prec_t diffLon2 = (upperLon - lowerLon) / 2;
	return Rect(
		Point(lowerLat + diffLat2, lowerLon + diffLon2),
		diffLat2, diffLon2
	);
}

Rect traffic::Rect::fromLength(prec_t lowerLat, prec_t lowerLon, prec_t latLength, prec_t lonLength)
{
	return Rect(
		Point(lowerLat + latLength * 0.5, lowerLon + lonLength * 0.5),
		latLength * 0.5, lonLength * 0.5
	);
}

Rect Rect::fromCenter(Point center, prec_t latLength, prec_t lonLength)
{ return Rect(center, latLength, lonLength); }

Rect Rect::fromCenter(prec_t centerLat, prec_t centerLon, prec_t latLength, prec_t lonLength)
{ return Rect(Point(centerLat, centerLon), latLength, lonLength); }

Rect::Rect()
{
	latLength = 0.0;
	lonLength = 0.0;
}

Rect Rect::fromCircle(const Circle& circle)
{
	return Rect(circle.getCenter(),
		circle.getLatRadius(),
		circle.getLonRadius());
}

Rect::Rect(Point center, prec_t latLength, prec_t lonLength)
{
	this->center = center;
	this->latLength = latLength;
	this->lonLength = lonLength;
}

prec_t Rect::getLatLength() const { return latLength; }
prec_t Rect::getLonLength() const { return lonLength; }
Point Rect::latHlonH() const { return center + Distance(latLength, lonLength); }
Point Rect::latHlonL() const { return center + Distance(latLength, -lonLength); }
Point Rect::latLlonH() const { return center + Distance(-latLength, lonLength); }
Point Rect::latLlonL() const { return center + Distance(-latLength, -lonLength); }

void traffic::Rect::summary() const
{
	
}

Rect& Rect::performScaleLat(prec_t scale) {
	latLength *= scale;
	return *this;
}

Rect& Rect::performScaleLon(prec_t scale) {
	lonLength *= scale;
	return *this;
}

Rect& Rect::performScale(prec_t latScale, prec_t lonScale) {
	latLength *= latScale;
	lonLength *= lonScale;
	return *this;
}

Rect& Rect::performScale(prec_t scale) {
	latLength *= scale;
	lonLength *= scale;
	return *this;
}

Rect Rect::scaleLat(prec_t scale) const
{ return Rect(center, latLength * scale, lonLength); }
Rect Rect::scaleLon(prec_t scale) const
{ return Rect(center, latLength, lonLength * scale); }
Rect Rect::scale(prec_t latScale, prec_t lonScale) const
{ return Rect(center, latLength * latScale, lonLength * lonScale); }
Rect Rect::scale(prec_t scale) const
{ return Rect(center, latLength * scale, lonLength * scale); }

bool Rect::containsLon(Point p) const {
	return p.getLongitude() >= lowerLonBorder() &&
		p.getLongitude() <= upperLonBorder();
}

bool Rect::containsLat(Point p) const {
	return p.getLatitude() >= lowerLatBorder() &&
		p.getLatitude() <= upperLatBorder();
}

bool Rect::contains(Point nd) const {
	return containsLon(nd) && containsLat(nd);
}

Distance::Distance()
	: latLength(0.0), lonLength(0.0) { }
Distance::Distance(Point point) 
	: latLength(point.getLatitude()), lonLength(point.getLongitude()) { }
Distance::Distance(Point p1, Point p2) :
	latLength(p1.getLatitude() - p2.getLatitude()),
	lonLength(p1.getLongitude() - p2.getLongitude()) { }
Distance::Distance(prec_t _latLength, prec_t _lonLength)
	: latLength(_latLength), lonLength(_lonLength) { }

prec_t Distance::getLatLength() const { return latLength; }
prec_t Distance::getLonLength() const { return lonLength; }
Distance& Distance::performScaleLat(prec_t scale) {
	latLength *= scale;
	return *this;
}

Distance& Distance::performScaleLon(prec_t scale) {
	lonLength *= scale;
	return *this;
}

Distance& Distance::performScale(prec_t latScale, prec_t lonScale) {
	latLength *= latScale;
	lonLength *= lonScale;
	return *this;
}

Distance& Distance::performScale(prec_t scale) {
	latLength *= scale;
	lonLength *= scale;
	return *this;
}

Distance Distance::scaleLat(prec_t scale) const { return Distance(latLength * scale, lonLength); }
Distance Distance::scaleLon(prec_t scale) const { return Distance(latLength, lonLength * scale); }
Distance Distance::scale(prec_t latScale, prec_t lonScale) const { return Distance(latLength * latScale, lonLength * lonScale); }
Distance Distance::scale(prec_t scale) const { return Distance(latLength * scale, lonLength * scale); }

prec_t Distance::getLength() const {
	return (prec_t)(sqrt(
		(double)latLength * (double)latLength +
		(double)lonLength * (double)lonLength)
	);
}
prec_t Distance::getLengthSquared() const { return latLength * latLength + lonLength * lonLength; }

Distance Distance::operator+(const Distance& dist) const {
	return Distance(latLength + dist.getLatLength(), lonLength + dist.getLonLength());
}

Distance Distance::operator-(const Distance& dist) const {
	return Distance(latLength - dist.getLatLength(), lonLength - dist.getLonLength());
}

Distance Distance::operator*(prec_t _scale) const {
	return scale(_scale);
}

Circle::Circle(const Point& center, prec_t radius)
{
	this->center = center;
	this->radiusLat = radius;
	this->radiusLon = radius;
}

Circle::Circle(const Point& center, prec_t radiusLat, prec_t radiusLon)
{
	this->center = center;
	this->radiusLat = radiusLat;
	this->radiusLon = radiusLon;
}

Circle& Circle::performLatScale(prec_t scale) {
	radiusLat *= scale;
	return *this;
}

Circle& Circle::performLonScale(prec_t scale) {
	radiusLon *= scale;
	return *this;
}

Circle& Circle::performScale(prec_t latScale, prec_t lonScale) {
	radiusLat *= latScale;
	radiusLon *= lonScale;
	return *this;
}

Circle& Circle::performScale(prec_t scale) {
	radiusLat *= scale;
	radiusLon *= scale;
	return *this;
}

Circle Circle::scaleLat(prec_t scale) const { return Circle(center, radiusLat * scale, radiusLon); }
Circle Circle::scaleLon(prec_t scale) const { return Circle(center, radiusLat, radiusLon * scale); }
Circle Circle::scale(prec_t latScale, prec_t lonScale) const { return Circle(center, radiusLat * latScale, radiusLon * lonScale); }
Circle Circle::scale(prec_t scale) const { return Circle(center, radiusLat * scale, radiusLon * scale); }

bool Circle::contains(const Point& p) const {
	Point dist = p - Distance(center);
	return (dist.getLatitude() * dist.getLatitude()) / radiusLat +
		(dist.getLongitude() * dist.getLongitude()) / radiusLon <= 1;
}

Point Circle::getCenter() const { return center; }
prec_t Circle::getLatRadius() const { return radiusLat; }
prec_t Circle::getLonRadius() const { return radiusLon; }

Circle::Circle()
{
	center = Point();
	radiusLat = 1.0;
	radiusLon = 1.0;
}

