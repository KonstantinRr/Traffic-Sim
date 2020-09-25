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

#ifndef ENGINE_H
#define ENGINE_H

#ifndef SPDLOG_FMT_EXTERNAL
#	define SPDLOG_FMT_EXTERNAL
#endif

#ifndef NANOGUI_USE_OPENGL
	#define NANOGUI_USE_OPENGL
#endif

#define _CRT_SECURE_NO_WARNINGS 1

#if defined(_WIN32)
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif


#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <iostream>

#include <robin_hood.h>

using float32 = float;
using float64 = double;

using prec_t = float;
using map_index_t = uint32_t;
using map_t = robin_hood::unordered_flat_map<int64_t, map_index_t>;
using chunkmap_t = robin_hood::unordered_flat_map<int64_t, map_index_t>;

template<typename c_type>
using mapid_t = robin_hood::unordered_flat_map<int64_t, c_type>;
class SizeObject
{
public:
	virtual bool hasManagedSize() const;
	virtual size_t getManagedSize() const;
	virtual size_t getSize() const = 0;
};

template<typename T> size_t getSizeOfObjects(const std::vector<T>& vec)
{
	size_t size = sizeof(vec);
	if (!vec.empty()) {
		if (vec[0].hasManagedSize()) {
			for (const T& instance : vec) {
				size += instance.getManagedSize();
			}
		}
		size += vec.capacity() * sizeof(T);
	}
	return size;
}

template<typename key_t, typename val_t>
size_t getMapCapacitySize(const std::unordered_map<key_t, val_t>& map) {
	size_t count = 0;
	for (size_t i = 0; i < map.bucket_count(); ++i) {
		size_t bucket_size = map.bucket_size(i);
		if (bucket_size == 0) {
			count++;
		}
		else {
			count += bucket_size;
		}
	}
	return count;
}

class AtomicLock
{
protected:
	std::atomic<bool> plock = { 0 };
	bool doLock;

public:
	AtomicLock(bool doLock = true);
	AtomicLock(const AtomicLock&) = delete;
	AtomicLock(AtomicLock&&) = delete;

	void lock() noexcept;
	bool try_lock() noexcept;
	void unlock() noexcept;

	AtomicLock& operator=(const AtomicLock&) = delete;
	AtomicLock& operator=(AtomicLock&&) = delete;
};

#endif
