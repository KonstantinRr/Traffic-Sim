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

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <vector>
#include <array>
#include <limits>

namespace lt {
	using prec_t = float;

	/// The general color type that encodes colors using
	/// floating value types.
	class Color;

	/// Defines the available color and pixel types.
	/// The library currently supports RGB, BGR, ARGB
	/// as well as RGBA pixel types.
	/// Each pixel type takes another template parameter
	/// that 
	template<typename Value> class PixelRGB;
	template<typename Value> class PixelBGR;
	template<typename Value> class PixelARGB;
	template<typename Value> class PixelRGBA;

	//
	template<template <class> class Pixel, class Value> class Image;
	template<template <class> class Pixel, class Value> class RasterMap;
	template<template <class> class Pixel, class Value> class RasterStorage;

	/// These classes are used to render simple objects in
	/// images. ImgDistance represents a distance on an image
	/// similar to a vector. ImgPoint represents a single point
	/// inside an image and ImgCircle represents a circular
	/// region inside the image.
	class ImgDistance;
	class ImgPoint;
	class ImgCircle;
	//class ImgRect; // TODO

	/// Defines the supported color types. The library has support
	/// for unsigned integers up to 64 bits per channel. It also
	/// offers support for single and double precision floating
	/// values. Color values in floating point representation
	/// are stored in the range [0, 1.0] inclusively. 

	// supported color types for bitdepth 8
	using PixelRGB8 = PixelRGB<uint8_t>;
	using PixelBGR8 = PixelBGR<uint8_t>;
	using PixelARGB8 = PixelARGB<uint8_t>;
	using PixelRGBA8 = PixelRGBA<uint8_t>;
	// supported color types for bitdepth 16
	using PixelRGB16 = PixelRGB<uint16_t>;
	using PixelBGR16 = PixelBGR<uint16_t>;
	using PixelARGB16 = PixelARGB<uint16_t>;
	using PixelRGBA16 = PixelRGBA<uint16_t>;
	// supported color types for bitdepth 32
	using PixelRGB32 = PixelRGB<uint32_t>;
	using PixelBGR32 = PixelBGR<uint32_t>;
	using PixelARGB32 = PixelARGB<uint32_t>;
	using PixelRGBA32 = PixelRGBA<uint32_t>;
	// supported color types for bitdepth 64
	using PixelRGB64 = PixelRGB<uint64_t>;
	using PixelBGR64 = PixelBGR<uint64_t>;
	using PixelARGB64 = PixelARGB<uint64_t>;
	using PixelRGBA64 = PixelRGBA<uint64_t>;


	// supported color types for bitdepth 8
	using ImageRGB8 = Image<PixelRGB, uint8_t>;
	using ImageBGR8 = Image<PixelBGR, uint8_t>;
	using ImageARGB8 = Image<PixelARGB, uint8_t>;
	using ImageRGBA8 = Image<PixelRGBA, uint8_t>;
	using RasterMapRGB8 = RasterMap<PixelRGB, uint8_t>;
	using RasterMapBGR8 = RasterMap<PixelBGR, uint8_t>;
	using RasterMapARGB8 = RasterMap<PixelARGB, uint8_t>;
	using RasterMapRGBA8 = RasterMap<PixelRGBA, uint8_t>;
	using RasterStorageRGB8 = RasterStorage<PixelRGB, uint8_t>;
	// supported color types for bitdepth 16
	using ImageRGB16 = Image<PixelRGB, uint16_t>;
	using ImageBGR16 = Image<PixelBGR, uint16_t>;
	using ImageARGB16 = Image<PixelARGB, uint16_t>;
	using ImageRGBA16 = Image<PixelRGBA, uint16_t>;
	using RasterMapRGB16 = RasterMap<PixelRGB, uint16_t>;
	using RasterMapBGR16 = RasterMap<PixelBGR, uint16_t>;
	using RasterMapARGB16 = RasterMap<PixelARGB, uint16_t>;
	using RasterMapRGBA16 = RasterMap<PixelRGBA, uint16_t>;
	// supported color types for bitdepth 32
	using ImageRGB32 = Image<PixelRGB, uint32_t>;
	using ImageBGR32 = Image<PixelBGR, uint32_t>;
	using ImageARGB32 = Image<PixelARGB, uint32_t>;
	using ImageRGBA32 = Image<PixelRGBA, uint32_t>;
	using RasterMapRGB32 = RasterMap<PixelRGB, uint32_t>;
	using RasterMapBGR32 = RasterMap<PixelBGR, uint32_t>;
	using RasterMapARGB32 = RasterMap<PixelARGB, uint32_t>;
	using RasterMapRGBA32 = RasterMap<PixelRGBA, uint32_t>;
	// supported color types for bitdepth 64
	using ImageRGB64 = Image<PixelRGB, uint64_t>;
	using ImageBGR64 = Image<PixelBGR, uint64_t>;
	using ImageARGB64 = Image<PixelARGB, uint64_t>;
	using ImageRGBA64 = Image<PixelRGBA, uint64_t>;
	using RasterMapRGB64 = RasterMap<PixelRGB, uint64_t>;
	using RasterMapBGR64 = RasterMap<PixelBGR, uint64_t>;
	using RasterMapARGB64 = RasterMap<PixelARGB, uint64_t>;
	using RasterMapRGBA64 = RasterMap<PixelRGBA, uint64_t>;

	template<class Value>
	struct ColorLimits {
		static Value max() { return Value(); }
		static Value min() { return Value(); }
	};

	// floating point color limits //
	template<> struct ColorLimits<float> {
		static float max() { return 1.0f; }
		static float min() { return 0.0f; }
	};

	template<> struct ColorLimits<double> {
		static double max() { return 1.0; }
		static double min() { return 0.0; }
	};
	// Integer color limits //
	template<> struct ColorLimits<uint8_t> {
		static uint8_t max() { return std::numeric_limits<uint8_t>::max(); }
		static uint8_t min() { return std::numeric_limits<uint8_t>::min(); }
	};
	template<> struct ColorLimits<uint16_t> {
		static uint16_t max() { return std::numeric_limits<uint16_t>::max(); }
		static uint16_t min() { return std::numeric_limits<uint16_t>::min(); }
	};
	template<> struct ColorLimits<uint32_t> {
		static uint32_t max() { return std::numeric_limits<uint32_t>::max(); }
		static uint32_t min() { return std::numeric_limits<uint32_t>::min(); }
	};
	template<> struct ColorLimits<uint64_t> {
		static uint64_t max() { return std::numeric_limits<uint64_t>::max(); }
		static uint64_t min() { return std::numeric_limits<uint64_t>::max(); }
	};

	template<typename T>
	T clamp(T v, T lower, T upper) {
		if (v < lower) return lower;
		if (v > upper) return upper;
		return v;
	}

	/// Represents a color with the given byte depth.
	/// Possible values for the depth channel are:
	/// Color8:		uint8_t: [0:2^8-1]
	/// Color16:	uint16_t: [0:2^16-1]
	/// Color32:	uint32_t: [0:2^32-1]
	/// Color64:	uint64_t: [0:2^64-1]
	/// Colorf32:	sfloat_t: +-1.18x10^-38 to +-3.4x10^38
	/// Colorf64:	prec_t: +-2.23x10^-308 to +-1.80x10^308
	/// Colors are represented in RGB format. Other color
	/// formats are currently not supported but will probably
	/// be supported in the near future.
	/// Colors can be 
	class Color {
	protected:
		double r, g, b, a;
	public:
		// Creates a color with the values 0 in each channel
		explicit Color();
		explicit Color(double r, double g, double b, double a);

		template<template <class> class Pixel, class Value>
		explicit Color(const Pixel<Value> &v) {
			r = static_cast<double>(v.getR()) / std::numeric_limits<Value>::max();
			g = static_cast<double>(v.getG()) / std::numeric_limits<Value>::max();
			b = static_cast<double>(v.getG()) / std::numeric_limits<Value>::max();
			a = static_cast<double>(v.getG()) / std::numeric_limits<Value>::max();
		}

		static Color fromRGB8(int r, int g, int b);
		static Color fromARGB8(int r, int g, int b, int a);

		Color clamp();
		Color& iclamp();

		Color operator+(const Color &other) const;
		Color operator-(const Color &other) const;
		Color operator*(double value) const;
		Color operator/(double value) const;

		Color& operator+=(const Color &other);
		Color& operator-=(const Color &other);
		Color& operator*=(double value);
		Color& operator/=(double value);

		/* Return the color values for the given colors. */
		double getR() const;
		double getG() const;
		double getB() const;
		double getA() const;
	};


	/// Image type that stores data in the format RGB.
	/// The Byte_ template argument gives the internal
	/// representation of the color values.
	/// Note: This format cannot store transparency,
	/// use RGBA or ARGB instead 
	template<typename Value>
	class PixelRGB {
	protected:
		std::array<Value, 3> data;

	public:
		static const size_t Length = 3 * sizeof(Value);
		static const size_t RED = 0, GREEN = 1, BLUE = 2;

	public:
		PixelRGB() : data{0} { }
		explicit PixelRGB(Value r, Value g, Value b) {
			data[RED] = r;
			data[GREEN] = g;
			data[BLUE] = b;
		}
	
		const Value getR() const { return data[RED]; }
		const Value getG() const { return data[GREEN]; }
		const Value getB() const { return data[BLUE]; }
		const Value getA() const { return 0xff; }

		/// Writes a color of the same Byte_ type to this format.
		PixelRGB<Value>& writeColor(const Color &color) {
			data[RED] = static_cast<Value>(color.getR() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[GREEN] = static_cast<Value>(color.getG() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[BLUE] = static_cast<Value>(color.getB() *
				static_cast<double>(ColorLimits<Value>::max()));
			return *this;
		}

		PixelRGB<Value>& writePixel(const PixelRGB &pixel) {
			data[RED] = pixel.getR();
			data[GREEN] = pixel.getG();
			data[BLUE] = pixel.getB();
			return *this;
		}

		PixelRGB<Value>& operator=(const Color &color) { return writeColor(color); }
		PixelRGB<Value>& operator=(const PixelRGB<Value> &pixel) { return writePixel(pixel); }

		explicit PixelRGB(const Color &color) { writeColor(color); }
		PixelRGB(const PixelRGB<Value> &pixel) { writePixel(pixel); }

		Value* getData() { return data.data(); }
		const Value* getData() const { return data.data(); }
	};

	/// Image type that stores data in the format BGR.
	/// The Byte_ template argument gives the internal
	/// representation of the color values.
	/// Note: This format cannot store transparency,
	/// use RGBA or ARGB instead
	template<typename Value>
	class PixelBGR {
	protected:
		std::array<Value, 3> data;

	public:
		static const size_t Length = 3 * sizeof(Value);
		static const size_t RED = 2, GREEN = 1, BLUE = 0;

	public:
		PixelBGR() : data{0} { }
		explicit PixelBGR(Value b, Value g, Value r) {
			data[BLUE] = b;
			data[GREEN] = g;
			data[RED] = r;
		}

		const Value getR() const { return data[RED]; }
		const Value getG() const { return data[GREEN]; }
		const Value getB() const { return data[BLUE]; }
		const Value getA() const { return 0xff; }


		/// Writes a color of the same Byte_ type to this format.
		PixelBGR<Value>& writeColor(const Color &color) {
			data[BLUE] = static_cast<Value>(color.getB() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[GREEN] = static_cast<Value>(color.getG() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[RED] = static_cast<Value>(color.getR() *
				static_cast<double>(ColorLimits<Value>::max()));
			return *this;
		}

		PixelBGR<Value>& writePixel(const PixelBGR<Value> &pixel) {
			data[BLUE] = pixel.getB();
			data[GREEN] = pixel.getG();
			data[RED] = pixel.getR();
			return *this;
		}

		PixelBGR<Value>& operator=(const Color &color) { return writeColor(color); }
		PixelBGR<Value>& operator=(const PixelBGR<Value> &pixel) { return writePixel(pixel); }

		explicit PixelBGR(const Color &color) { writeColor(color); }
		PixelBGR(const PixelBGR<Value> & pixel) { writePixel(pixel); }

		Value* getData() { return data.data(); }
		const Value* getData() const { return data.data(); }
	};


	/// Image type that stores data in the format ARGB.
	/// The Byte_ template argument gives the internal
	/// representation of the color values.
	template<typename Value>
	class PixelARGB {
	protected:
		std::array<Value, 4> data;

	public:
		static const size_t Length = 4 * sizeof(Value);
		static const size_t RED = 1, GREEN = 2, BLUE = 3, ALPHA = 0;

	public:
		const Value getR() const { return data[RED]; }
		const Value getG() const { return data[GREEN]; }
		const Value getB() const { return data[BLUE]; }
		const Value getA() const { return data[ALPHA]; }

		PixelARGB() : data{0} { }
		explicit PixelARGB(Value a, Value r, Value g, Value b) {
			data[ALPHA] = a;
			data[RED] = r;
			data[GREEN] = g;
			data[BLUE] = b;
		}

		/// Writes a color of the same Byte_ type to this format.
		PixelARGB<Value>& writeColor(const Color &color) {
			data[ALPHA] = static_cast<Value>(color.getA() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[RED] = static_cast<Value>(color.getR() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[GREEN] = static_cast<Value>(color.getG() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[BLUE] = static_cast<Value>(color.getB() *
				static_cast<double>(ColorLimits<Value>::max()));
			return *this;
		}

		PixelARGB<Value>& writePixel(const PixelARGB<Value> &pixel) {
			data[ALPHA] = pixel.getA();
			data[RED] = pixel.getR();
			data[GREEN] = pixel.getG();
			data[BLUE] = pixel.getB();
			return *this;
		}

		PixelARGB<Value>& operator=(const Color &color) { return writeColor(color); }
		PixelARGB<Value>& operator=(const PixelARGB<Value> &pixel) { return writePixel(pixel); }

		explicit PixelARGB(const Color &color) { writeColor(color); }
		PixelARGB(const PixelARGB<Value> &pixel) { writePixel(pixel); }

		Value* getData() { return data.data(); }
		const Value* getData() const { return data.data(); }
	};

	/// Image type that stores data in the format RGBA.
	/// The Byte_ template argument gives the internal
	/// representation of the color values.
	template<typename Value>
	class PixelRGBA {
	protected:
		std::array<Value, 4> data;

	public:
		static const size_t Length = 4 * sizeof(Value);
		static const size_t RED = 0, GREEN = 1, BLUE = 2, ALPHA = 3;

	public:
		const Value getR() const { return data[RED]; }
		const Value getG() const { return data[GREEN]; }
		const Value getB() const { return data[BLUE]; }
		const Value getA() const { return data[ALPHA]; }

		PixelRGBA() : data{0} { }
		explicit PixelRGBA(Value r, Value g, Value b, Value a) {
			data[RED] = r;
			data[GREEN] = g;
			data[BLUE] = b;
			data[ALPHA] = a;
		}

		/// Writes a color of the same Byte_ type to this format.
		PixelRGBA& writeColor(const Color &color) {
			data[RED] = static_cast<Value>(color.getR() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[GREEN] = static_cast<Value>(color.getG() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[BLUE] = static_cast<Value>(color.getB() *
				static_cast<double>(ColorLimits<Value>::max()));
			data[ALPHA] = static_cast<Value>(color.getA() *
				static_cast<double>(ColorLimits<Value>::max()));
			return *this;
		}

		PixelRGBA& writePixel(const PixelRGBA<Value> &value) {
			data[RED] = value.getR();
			data[GREEN] = value.getG();
			data[BLUE] = value.getB();
			data[ALPHA] = value.getA();
			return *this;
		}

		PixelRGBA<Value>& operator=(const Color &color) { return writeColor(color); }
		PixelRGBA<Value>& operator=(const PixelRGBA<Value> &pixel) { return writePixel(pixel); }

		explicit PixelRGBA(const Color &color) { writeColor(color); }
		PixelRGBA(const PixelRGBA &pixel) { writePixel(pixel); }

		Value* getData() { return data.data(); }
		const Value* getData() const { return data.data(); }
	};

	/// This class represents a distance between two points on an
	/// image. This is similar to the definition of a vector in
	/// 2 dimensions. The class holds 2 member arguments defining
	/// the x and y offset on the image.
	class ImgDistance {
	protected:
		int64_t dx, dy;

	public:
		/// (1) Creates a distance with dx = 0 and dy = 0
		/// (2) Creates a distance with the given arguments
		/// (3) Creates a distance defined as p1 - p2 
		/// (4) Converts a ImgPoint to a ImgDistance
		explicit ImgDistance();
		explicit ImgDistance(int64_t dx, int64_t dy);
		explicit ImgDistance(ImgPoint p1, ImgPoint p2);
		explicit ImgDistance(const ImgPoint &point);

		inline int64_t getDX() const { return dx; }
		inline int64_t getDY() const { return dy; }

		/* Returns a scaled version of this ImgDistance */
		ImgDistance scaleX(int64_t scale) const;
		ImgDistance scaleY(int64_t scale) const;
		ImgDistance scale(int64_t scaleX, int64_t scaleY) const;
		ImgDistance scale(int64_t scale) const;

		/* Scales the coordinates by the given factors */
		ImgDistance& performScaleX(int64_t scale);
		ImgDistance& performScaleY(int64_t scale);
		ImgDistance& performScale(int64_t scaleX, int64_t scaleY);
		ImgDistance& performScale(int64_t scale);

		/* Basic arithmetic for addind and subtracting distances */
		ImgDistance operator+(ImgDistance dist) const;
		ImgDistance operator-(ImgDistance dist) const;
		ImgDistance& operator+=(ImgDistance dist);
		ImgDistance& operator-=(ImgDistance dist);

		/* Returns the pythogoras length of this ImgDistance */
		int64_t getLengthSquared() const;
		prec_t getLength() const;

	};

	class ImgPoint {
	protected:
		int64_t x_, y_;
	public:
		explicit ImgPoint();
		explicit ImgPoint(int64_t x, int64_t y);
		explicit ImgPoint(const ImgDistance &dist);
		inline int64_t getX() const { return x_; }
		inline int64_t getY() const { return y_; }

		ImgPoint operator+(ImgDistance dist) const;
		ImgPoint operator-(ImgDistance dist) const;

		ImgPoint& operator+=(ImgDistance dist);
		ImgPoint& operator-=(ImgDistance dist);
	};

	class ImgCircle {
	protected:
		ImgPoint center_;
		int64_t radiusx_, radiusy_;

	public:
		explicit ImgCircle();
		explicit ImgCircle(int64_t radiusx, int64_t radiusy, int64_t x, int64_t y);
		explicit ImgCircle(int64_t radiusx, int64_t radiusy, ImgPoint center);

		ImgPoint getCenter() const;
		int64_t getRadiusX() const;
		int64_t getRadiusY() const;

		bool contains(ImgPoint point) const;
	};

	class ImgRect {
	protected:
		int64_t xextent_, yextent_;
		ImgPoint center_;

	public:
		/// Creates that most tightly encloses the given values
		/// prec_t lowerLat: Lower latitude boundary
		/// prec_t upperLat: Upper latitude boundary
		/// prec_t lowerLon: Lower longitude boundary
		/// prec_t upperLon: Upper longitude boundary
		static ImgRect fromBorders(int64_t lowerLat, int64_t upperLat, int64_t lowerLon, int64_t upperLon);
		/// Creates a rect from a center point and the distances from
		/// the center to the sides.
		/// Point center: Center point of this rect
		/// prec_t latLength: Distance from the center to the latitude border
		/// prec_t lonLength: Distance from the center to the longitude border
		static ImgRect fromCenter(ImgPoint center, int64_t latLength, int64_t lonLength);
		/// Creates a rect from a center point and the distances from
		/// the center to the sides. This function has the same logic as
		/// fromCenter(Point, prec_t, prec_t)
		static ImgRect fromCenter(int64_t centerLat, int64_t centerLon, int64_t latLength, int64_t lonLength);
		/// Creates a rect that most tightly encloses a circle.
		static ImgRect fromCircle(const ImgCircle& circle);

		/// (1) Creates a rect that is equal to fromBorders(0, 0, 0, 0)
		explicit ImgRect();
		explicit ImgRect(ImgPoint center, int64_t latLength, int64_t lonLength);
		
		ImgPoint xHyH() const;
		ImgPoint xHyL() const;
		ImgPoint xLyH() const;
		ImgPoint xLyL() const;

		int64_t upperXBorder() const;
		int64_t lowerXBorder() const;
		int64_t upperYBorder() const;
		int64_t lowerYBorder() const;

		template<typename Exec_>
		void every(Exec_ &&func) {
			ImgPoint lowPoint = xLyL();
			ImgPoint highPoint = xHyH();
			for (int64_t x = lowPoint.getX(); x < highPoint.getX(); x++) {
				for (int64_t y = lowPoint.getY(); y < highPoint.getY(); y++) {
					func(x, y);
				}
			}
		}

		int64_t getXExtent() const;
		int64_t getYExtent() const;
		ImgPoint getCenter() const;

		bool contains(ImgPoint point) const;
		bool containsX(ImgPoint point) const;
		bool containsY(ImgPoint point) const;
	};

	/* Represents an image that
	 */
	template<template <class> class Pixel, class Value> // TODO rename
	class Image {
	protected:
		/// The dimensions of the image. The image data is
		/// stored in a single dimensional array where 2D
		/// access is simulated.
		size_t width, height;
		std::vector<Pixel<Value>> data;

		/// Returns whether the rect is contained in this image.
		bool checkBoundaries(const ImgRect& rect) const;
		/// Returns whether the point is contained in this image.
		bool checkBoundaries(size_t w, size_t h) const;
		/// Throws an exception if the rect does not fit in this image
		void throwBoundary(const ImgRect &rect) const;

	public:
		Image();
		explicit Image(size_t width, size_t height)
			: data(width * height) {
			this->width = width;
			this->height = height;
		}
		explicit Image(std::vector<Pixel<Value>> &&imageData, size_t width, size_t height)
			: data(move(imageData)) {
			this->width = width;
			this->height = height;
		}

		template<template <class> class NewPixel, class NewValue>
		Image<NewPixel, NewValue> convert(const Image<Pixel, Value> &image)
		{
			std::vector<NewPixel<NewValue>> convertData(width * height);
			for (size_t i = 0; i < width * height; i++) {
				convertData[i] = Color(data[i]);
			}
			return Image<NewPixel, NewValue>(
				std::move(convertData), width, height);
		} 

		Image(const Image<Pixel, Value> &) = delete;
		Image(Image<Pixel, Value> &&) = default;

		Image<Pixel, Value>& operator=(const Image<Pixel, Value> &) = delete;
		Image<Pixel, Value>& operator=(Image<Pixel, Value> &&) = default;

		Image<Pixel, Value> copy();
		Image<Pixel, Value> flipHorizontal();
		Image<Pixel, Value> flipVertical();
		void flipHorizontalInplace();
		void flipVerticalInplace();

		Image<Pixel, Value> exportImage(const ImgRect &rect) const;
		Image<Pixel, Value> ant(size_t aa);

		// Position based iterating // 

		template<typename Exec>
		void foreachPositionWidth(const Exec& exec) {
			for (size_t w = 0; w < width; w++) {
				for (size_t h = 0; h < height; h++) {
					exec(w, h);
				}
			}
		}

		template<typename Exec>
		void foreachPositionHeight(const Exec& exec) {
			for (size_t h = 0; h < height; h++) {
				for (size_t w = 0; w < width; w++) {
					exec(w, h);
				}
			}
		}

		template<typename Exec>
		void foreachPosition(const Exec &exec)
		{ foreachPositionWidth(); }
		
		// Index based iterating //

		template<typename Exec>
		void foreachIndex(const Exec& exec) {
			for (size_t i = 0; i < width * height; i++)
				exec(i);
		}

		// Mapping //
		template<typename Map>
		void map(const Map &mapFunction) {
			for (size_t i = 0; i < width * height; i++) {
				data[i] = mapFunction(data[i]);
			}
		}

		/// (1) Returns the x extent of the image
		/// (2) Returns the y extent of the image
		size_t getXExtent() const;
		size_t getYExtent() const;

		/// Returns the pixel value at a given index
		/// position.
		const Pixel<Value>& operator[](size_t index) const;
		Pixel<Value>& operator[](size_t index);

		/// (1) Returns the index of the passed x/y coordinate
		/// (2) Returns the image structure at a given x-y position
		/// (3) Returns the image structure at a given x-y position
		size_t getPostion(size_t x, size_t y) const;
		Pixel<Value>& get(size_t x, size_t y);
		const Pixel<Value>& get(size_t x, size_t y) const;

		/// Returns the raw data pointer
		unsigned char* raw();

		void drawRect(ImgRect x1, Color color);
		void drawCircle(ImgCircle circle, Color color);
		void drawLine(const ImgPoint &x1, const ImgPoint &x2, Color color, int32_t radius, prec_t accuracy);

		void set(const Color &color);
		void set(const Color &value, size_t index);
		void set(const Color &value, size_t w, size_t h);

		void set(const Pixel<Value> &value);
		void set(const Pixel<Value> &value, size_t index);
		void set(const Pixel<Value> &value, size_t w, size_t h);

		Pixel<Value>* getData();
		const Pixel<Value>* getData() const;
	};
} // namespace lt

#endif
