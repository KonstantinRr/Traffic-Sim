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

#define SPDLOG_FMT_EXTERNAL
#define _CRT_SECURE_NO_WARNINGS 1

#if defined(_WIN32)
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif


using float32 = float;
using float64 = double;

using prec_t = double;

#include <atomic>
#include <iostream>

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
