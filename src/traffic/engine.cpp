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

AtomicLock::AtomicLock(bool doLock) {
	this->doLock = doLock;
}

void AtomicLock::lock() noexcept {
	this->doLock = doLock;
	if (doLock) {
		for (;;) {
			// Optimistically assume the lock is free on first the try
			if (!plock.exchange(true, std::memory_order_acquire)) {
				return;
			}
			// Wait for lock to be released without generating cache misses
			while (plock.load(std::memory_order_relaxed)) {
				// Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
				// hyper-threads
#if defined(__GNUC__)
				__builtin_ia32_pause();
#elif defined(_MSC_VER )
				_mm_pause();
#endif
			}
		}
	}
}

bool AtomicLock::try_lock() noexcept {
	// First do a relaxed load to check if lock is free in order to prevent
	// unnecessary cache misses if someone does while(!try_lock())
	return !plock.load(std::memory_order_relaxed) &&
		!plock.exchange(true, std::memory_order_acquire);
}


void AtomicLock::unlock() noexcept {
	if (doLock) {
		plock.store(false, std::memory_order_release);
	}
}

bool SizeObject::hasManagedSize() const
{
	return false;
}

size_t SizeObject::getManagedSize() const
{
	return 0;
}
