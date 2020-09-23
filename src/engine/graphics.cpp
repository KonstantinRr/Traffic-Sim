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

#include <random>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <chrono>
#include <exception>

#include "graphics.hpp"

using namespace lt;

bool debug = true;

Color Color::fromRGB8(int r, int g, int b) {
	return Color(
		static_cast<double>(r) / 255.0,
		static_cast<double>(g) / 255.0,
		static_cast<double>(b) / 255.0,
		1.0
	);
}

Color Color::fromARGB8(int r, int g, int b, int a) {
	return Color(
		static_cast<double>(r) / 255.0,
		static_cast<double>(g) / 255.0,
		static_cast<double>(b) / 255.0,
		static_cast<double>(a) / 255.0
	);
}

Color::Color() : r(0.0), g(0.0), b(0.0), a(0.0) { }
Color::Color(double pr, double pg, double pb, double pa)
	: r(pr), g(pg), b(pb), a(pa) { }

Color Color::clamp() {
	return Color(
		lt::clamp(r, 0.0, 1.0),
		lt::clamp(g, 0.0, 1.0),
		lt::clamp(b, 0.0, 1.0),
		lt::clamp(a, 0.0, 1.0));
}
Color& Color::iclamp() {
	r = lt::clamp(r, 0.0, 1.0);
	g = lt::clamp(g, 0.0, 1.0);
	b = lt::clamp(b, 0.0, 1.0);
	a = lt::clamp(a, 0.0, 1.0);
	return *this;
}

Color Color::operator+(const Color &p) const { return Color(r + p.r, g + p.g, b + p.b, a + p.a).clamp(); }
Color Color::operator-(const Color &p) const { return Color(r - p.r, g - p.g, b - p.b, a - p.a).clamp(); }
Color Color::operator*(double v) const { return Color(r * v, g * v, b * v, a * v); }
Color Color::operator/(double v) const { return Color(r / v, g / v, b / v, a / v); }

Color& Color::operator+=(const Color &p) {
	r += p.r;
	g += p.g;
	b += p.b;
	a += p.a;
	return iclamp();
}

Color& Color::operator-=(const Color &p) {
	r -= p.r;
	g -= p.g;
	b -= p.b;
	a -= p.a;
	return iclamp();
}
Color& Color::operator*=(double value) {
	r *= value;
	g *= value;
	b *= value;
	a *= value;
	return iclamp();
}
Color& Color::operator/=(double value) {
	r /= value;
	g /= value;
	b /= value;
	a /= value;
	return iclamp();
}

/* Return the color values for the given colors. */
double Color::getR() const { return r; }
double Color::getG() const { return g; }
double Color::getB() const { return b; }
double Color::getA() const { return a; }

ImgPoint::ImgPoint() : x_(0), y_(0) { }
ImgPoint::ImgPoint(int64_t x, int64_t y) : x_(x), y_(y) { }
ImgPoint::ImgPoint(const ImgDistance& dist) {
	x_ = dist.getDX();
	y_ = dist.getDY();
}

ImgPoint ImgPoint::operator+(ImgDistance d) const { return ImgPoint(x_ + d.getDX(), y_ + d.getDY()); }
ImgPoint ImgPoint::operator-(ImgDistance d) const { return ImgPoint(x_ - d.getDX(), y_ - d.getDY()); }

ImgPoint& ImgPoint::operator+=(ImgDistance dist) {
	x_ += dist.getDX();
	y_ += dist.getDY();
	return *this;
}
ImgPoint& ImgPoint::operator-=(ImgDistance dist) {
	x_ -= dist.getDX();
	y_ -= dist.getDY();
	return *this;
}
ImgCircle::ImgCircle() : radiusx_(0), radiusy_(0) { }
ImgCircle::ImgCircle(int64_t radiusx, int64_t radiusy, int64_t x, int64_t y) {
	center_ = ImgPoint(x, y);
	radiusx_ = radiusx;
	radiusy_ = radiusy;
}

ImgCircle::ImgCircle(int64_t radiusx, int64_t radiusy, ImgPoint center) {
	radiusx_ = radiusx;
	radiusy_ = radiusy;
	center_ = center;
}

ImgPoint ImgCircle::getCenter() const { return center_; }
int64_t ImgCircle::getRadiusX() const { return radiusx_; }
int64_t ImgCircle::getRadiusY() const { return radiusy_; }

bool ImgCircle::contains(ImgPoint point) const {
	ImgPoint dist = point - ImgDistance(center_);
	prec_t doubleX = static_cast<prec_t>(dist.getX());
	prec_t doubleY = static_cast<prec_t>(dist.getY());
	prec_t doubleRadiusX = static_cast<prec_t>(radiusx_);
	prec_t doubleRadiusY = static_cast<prec_t>(radiusy_);
	return
		(doubleX * doubleX) / (doubleRadiusX * doubleRadiusX) +
		(doubleY * doubleY) / (doubleRadiusY * doubleRadiusY) <= 1;
}


ImgRect ImgRect::fromBorders(int64_t lowerLat, int64_t upperLat,
	int64_t lowerLon, int64_t upperLon) {
	int64_t diffLat = upperLat - lowerLat;
	int64_t diffLon = upperLon - lowerLon;
	return ImgRect(
		ImgPoint(lowerLat + diffLat / 2, lowerLon + diffLon / 2),
		diffLat / 2, diffLon / 2
	);
}

ImgRect ImgRect::fromCenter(ImgPoint center, int64_t latLength, int64_t lonLength)
{ return ImgRect(center, latLength, lonLength); }

ImgRect ImgRect::fromCenter(int64_t centerLat, int64_t centerLon, int64_t latLength, int64_t lonLength)
{ return ImgRect(ImgPoint(centerLat, centerLon), latLength, lonLength); }

ImgRect::ImgRect()
	: center_(0, 0), xextent_(0), yextent_(0) {}

ImgRect ImgRect::fromCircle(const ImgCircle& circle)
{
	return ImgRect(circle.getCenter(),
		circle.getRadiusX(),
		circle.getRadiusY());
}

ImgRect::ImgRect(ImgPoint center, int64_t xextent, int64_t yextent)
	: center_(center), xextent_(xextent), yextent_(yextent) { }

ImgPoint ImgRect::xHyH() const { return center_ + ImgDistance(xextent_, yextent_); }
ImgPoint ImgRect::xHyL() const { return center_ + ImgDistance(xextent_, -yextent_); }
ImgPoint ImgRect::xLyH() const { return center_ + ImgDistance(-xextent_, yextent_); }
ImgPoint ImgRect::xLyL() const { return center_ + ImgDistance(-xextent_, -yextent_); }

int64_t ImgRect::upperXBorder() const { return center_.getX() + xextent_; }
int64_t ImgRect::lowerXBorder() const { return center_.getX() - xextent_; }
int64_t ImgRect::upperYBorder() const { return center_.getY() + yextent_; }
int64_t ImgRect::lowerYBorder() const { return center_.getY() - yextent_; }



int64_t ImgRect::getXExtent() const { return 2 * xextent_; }
int64_t ImgRect::getYExtent() const { return 2 * yextent_; }
ImgPoint ImgRect::getCenter() const { return center_; }

bool ImgRect::contains(ImgPoint point) const { return containsX(point) && containsY(point); }
bool ImgRect::containsX(ImgPoint point) const { return lowerXBorder() <= point.getX() && upperXBorder() >= point.getX(); }
bool ImgRect::containsY(ImgPoint point) const { return lowerYBorder() <= point.getY() && upperYBorder() >= point.getY(); }

ImgDistance::ImgDistance()
{
	dx = 0;
	dy = 0;
}

ImgDistance::ImgDistance(int64_t dx_, int64_t dy_)
	: dx(dx_), dy(dy_) { }
ImgDistance::ImgDistance(ImgPoint p1, ImgPoint p2)
	: dx(p1.getX() - p2.getX()), dy(p1.getY() - p2.getY()) { }
ImgDistance::ImgDistance(const ImgPoint& point)
	: dx(point.getX()), dy(point.getY()) { }

ImgDistance ImgDistance::scaleX(int64_t scale) const {
	return ImgDistance(dx * scale, dy); }
ImgDistance ImgDistance::scaleY(int64_t scale) const {
	return ImgDistance(dx, dy * scale); }
ImgDistance ImgDistance::scale(int64_t scaleX, int64_t scaleY) const {
	return ImgDistance(dx * scaleX, dy * scaleY); }
ImgDistance ImgDistance::scale(int64_t scale) const {
	return ImgDistance(dx * scale, dy * scale);
}

ImgDistance& ImgDistance::performScaleX(int64_t scale) {
	dx *= scale;
	return *this;
}

ImgDistance& ImgDistance::performScaleY(int64_t scale) {
	dy *= scale;
	return *this;
}

ImgDistance& ImgDistance::performScale(int64_t scaleX, int64_t scaleY) {
	dx *= scaleX;
	dy *= scaleY;
	return *this;
}

ImgDistance& ImgDistance::performScale(int64_t scale) {
	dx *= scale;
	dy *= scale;
	return *this;
}

ImgDistance ImgDistance::operator+(ImgDistance dist) const {
	return ImgDistance(dx + dist.getDX(), dy + dist.getDY());
}

ImgDistance ImgDistance::operator-(ImgDistance dist) const {
	return ImgDistance(dx - dist.getDX(), dy - dist.getDY());
}

ImgDistance& ImgDistance::operator+=(ImgDistance dist) {
	dx += dist.getDX();
	dy += dist.getDY();
	return *this;
}

ImgDistance& ImgDistance::operator-=(ImgDistance dist) {
	dx -= dist.getDX();
	dy -= dist.getDY();
	return *this;
}

int64_t ImgDistance::getLengthSquared() const {
	return dx * dx + dy * dy;
}

prec_t ImgDistance::getLength() const {
	return sqrt(dx * dx + dy * dy);
}

/////////////////////////
//// ---- Image ---- ////
template<template <class> class Pixel, class Value>
Image<Pixel, Value>::Image() : data(0), width(0), height(0) { }

template<template <class> class Pixel, class Value>
bool Image<Pixel, Value>::checkBoundaries(const ImgRect& rect) const
{
	return rect.lowerXBorder() >= 0 &&
		static_cast<size_t>(rect.upperXBorder()) <= width &&
		rect.lowerYBorder() >= 0 &&
		static_cast<size_t>(rect.upperYBorder()) <= height;
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::throwBoundary(const ImgRect& rect) const
{
	if (!checkBoundaries(rect)) {
		//printf("%d %d %d %d\n", rect.lowerXBorder(), rect.upperXBorder(),
		//	rect.lowerYBorder(), rect.upperYBorder());
		//throw std::range_error("Boundaries of image violated");
	}
}

template<template <class> class Pixel, class Value>
bool Image<Pixel, Value>::checkBoundaries(size_t w, size_t h) const {
	return w >= 0 && w < width && h >= 0 && h < height;
}

template<template <class> class Pixel, class Value>
Image<Pixel, Value> Image<Pixel, Value>::ant(size_t aa)
{
	// TODO
	if (width % aa != 0 || height % aa != 0) {
		throw std::runtime_error("Width and height must be divisible by aa");
	}
	size_t newWidth = width / aa;
	size_t newHeight = height / aa;
	size_t aaScale = aa * aa;
	Image<Pixel, Value> newImg(newWidth, newHeight);

	for (size_t x = 0, newx = 0; x < width; x += aa, newx++) {
		for (size_t y = 0, newy = 0; y < height; y += aa, newy++) {
			// TODO wrong implementation
			Color color;
			for (size_t ax = 0; ax < aa; ax++) {
				for (size_t ay = 0; ay < aa; ay++) {
					//color += get(x + ax, y + ay).toColor();
				}
			}
			color /= static_cast<prec_t>(aaScale);
			newImg.set(color, newx, newy);
		}
	}


	return newImg;
}


// Width Height
template<template <class> class Pixel, class Value>
size_t Image<Pixel, Value>::getXExtent() const { return width; }
template<template <class> class Pixel, class Value>
size_t Image<Pixel, Value>::getYExtent() const { return height; }

template<template <class> class Pixel, class Value>
const Pixel<Value>& Image<Pixel, Value>::operator[](size_t index) const {
	return data[index];
}

template<template <class> class Pixel, class Value>
Pixel<Value>& Image<Pixel, Value>::operator[](size_t index) {
	return data[index];
}

template<template <class> class Pixel, class Value>
size_t Image<Pixel, Value>::getPostion(size_t x, size_t y) const {
	return y * width + x;
}

template<template <class> class Pixel, class Value>
Pixel<Value>& Image<Pixel, Value>::get(size_t x, size_t y) {
	return operator[](getPostion(x, y));
}
template<template <class> class Pixel, class Value>
const Pixel<Value>& Image<Pixel, Value>::get(size_t x, size_t y) const {
	return operator[](getPostion(x, y));
}


template<template <class> class Pixel, class Value>
unsigned char* Image<Pixel, Value>::raw() { return (unsigned char*)(data.data()); }

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::drawRect(ImgRect x1, Color color) {
	throwBoundary(x1);
	x1.every([&](int64_t x, int64_t y) {
		size_t sx = static_cast<size_t>(x);
		size_t sy = static_cast<size_t>(y);
		if (checkBoundaries(sx, sy)) {
			set(color, sx, sy);
		}
	});
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::drawCircle(ImgCircle circle, Color color) {
	ImgRect rect = ImgRect::fromCircle(circle);
	throwBoundary(rect);
	rect.every([&](int64_t x, int64_t y) {
		size_t sx = static_cast<size_t>(x);
		size_t sy = static_cast<size_t>(y);
		if (circle.contains(ImgPoint(x, y)) && checkBoundaries(sx, sy)) {
			set(color, sx, sy);
		}
	});
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::drawLine(
	const ImgPoint& x1,
	const ImgPoint& x2,
	Color color,
	int32_t radius,
	prec_t accuracy
) {
	// Calculates the distence between the points
	ImgDistance dist(x2, x1);
	prec_t length = dist.getLength();

	prec_t acc = accuracy * length;
	prec_t dx = static_cast<prec_t>(dist.getDX()) / acc;
	prec_t dy = static_cast<prec_t>(dist.getDY()) / acc;
	size_t amount = static_cast<size_t>(acc);

	// moves the position across the line
	prec_t cx = static_cast<prec_t>(x1.getX());
	prec_t cy = static_cast<prec_t>(x1.getY());
	for (size_t i = 0; i < amount; i++) {
		drawCircle(ImgCircle(radius, radius,
			ImgPoint(
				static_cast<int32_t>(cx),
				static_cast<int32_t>(cy))),
			color);
		cx += dx;
		cy += dy;
	}
}



template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::set(const Color &color) {
	foreachIndex([this, color](size_t index) { this->set(color, index); });
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::set(
	const Color &value, size_t index) {
	data[index] = value;
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::set(
	const Color &value, size_t w, size_t h) {
	data[getPostion(w, h)] = value;
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::set(const Pixel<Value> &value)
{ foreachIndex([this, value](size_t index) { this->set(value, index); }); }

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::set(const Pixel<Value> &value, size_t index) {
	data[index] = value;
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::set(const Pixel<Value> &value, size_t w, size_t h) {
	data[getPostion(w, h)] = value;
}

template<template <class> class Pixel, class Value>
Image<Pixel, Value> Image<Pixel, Value>::exportImage(const ImgRect &rect) const {
	Image<Pixel, Value> img(
		static_cast<size_t>(rect.getXExtent()),
		static_cast<size_t>(rect.getYExtent()));
	size_t wLength = static_cast<size_t>(rect.getXExtent());
	size_t hLength = static_cast<size_t>(rect.getYExtent());
	size_t lowerXBorder = static_cast<size_t>(rect.lowerXBorder());
	size_t lowerYBorder = static_cast<size_t>(rect.lowerYBorder());

	for (size_t w = 0; w < wLength; w++) {
		for (size_t h = 0; h < hLength; h++) {
			img.set(this->get(
				lowerXBorder + w,
				lowerYBorder + h
			), w, h);
		}
	}
	return img;
}

template<template <class> class Pixel, class Value>
Image<Pixel, Value> Image<Pixel, Value>::copy() {
	std::vector<Pixel<Value>> convertData(width * height);
	for (size_t i = 0; i < width * height; i++) {
		convertData[i] = data[i];
	}
	return Image<Pixel, Value>(
		std::move(convertData), width, height);
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::flipHorizontalInplace() {
	for (size_t y = 0; y < height; y++) {
		size_t xBegin = 0, xEnd = width - 1;
		for ( ; xBegin < xEnd; xBegin++, xEnd-- ) {
			Pixel<Value> beginVal = get(xBegin, y);
			Pixel<Value> endVal = get(xEnd, y);
			set(endVal, xBegin, y);
			set(beginVal, xEnd, y);
		}
	}
}

template<template <class> class Pixel, class Value>
void Image<Pixel, Value>::flipVerticalInplace() {
	for (size_t x = 0; x < width; x++) {
		size_t yBegin = 0, yEnd = height - 1;
		for ( ; yBegin < yEnd; yBegin++, yEnd-- ) {
			Pixel<Value> beginVal = get(x, yBegin);
			Pixel<Value> endVal = get(x, yEnd);
			set(endVal, x, yBegin);
			set(beginVal, x, yEnd);
		}
	}
}

template<template <class> class Pixel, class Value>
Pixel<Value>* Image<Pixel, Value>::getData() { return data.data(); }

template<template <class> class Pixel, class Value>
const Pixel<Value>* Image<Pixel, Value>::getData() const { return data.data(); }

namespace lt {
// supported color types for bitdepth 8
template class Image<PixelRGB, uint8_t>;
template class Image<PixelBGR, uint8_t>;
template class Image<PixelARGB, uint8_t>;
template class Image<PixelRGBA, uint8_t>;
// supported color types for bitdepth 16
template class Image<PixelRGB, uint16_t>;
template class Image<PixelBGR, uint16_t>;
template class Image<PixelARGB, uint16_t>;
template class Image<PixelRGBA, uint16_t>;
// supported color types for bitdepth 32
template class Image<PixelRGB, uint32_t>;
template class Image<PixelBGR, uint32_t>;
template class Image<PixelARGB, uint32_t>;
template class Image<PixelRGBA, uint32_t>;
// supported color types for bitdepth 64
template class Image<PixelRGB, uint64_t>;
template class Image<PixelBGR, uint64_t>;
template class Image<PixelARGB, uint64_t>;
template class Image<PixelRGBA, uint64_t>;
}