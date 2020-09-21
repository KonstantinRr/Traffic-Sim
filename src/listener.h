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

#include "traffic/engine.h"

#include <vector>

template<typename Type>
struct CallbackForm {
	int32_t id;
	std::function<Type> function;

	template<typename Param>
	CallbackForm(int32_t id, Param && func)
		: id(id), function(func) { }
};

template<typename Type>
struct CallbackReturn {
	int32_t id;
	std::vector<CallbackForm<Type>> *parent;

	CallbackReturn(int32_t id, std::vector<CallbackForm<Type>>* parent)
		: id(id), parent(parent) { }

	bool isActive() const { return id != -1; }
	void remove() {
		for (size_t i = 0; i < parent->size();) {
			if ((*parent)[i].id == id) {
				parent->erase(parent->begin() + i);
			}
			else {
				i++;
			}
		}
		id = -1;
	}
};

template<typename Type>
class Listener {
protected:
	std::vector<CallbackForm<Type>> callbacks;

public:
	template<typename FType>
	CallbackReturn<Type> listen(FType && function) {
		int32_t id = callbacks.empty() ? 0 : callbacks.back().id + 1;
		callbacks.push_back(
			CallbackForm<Type>(
				id, std::function<Type>(function)
			)
		);
		return CallbackReturn<Type>(id, &callbacks);
	}

	template<typename ... Args>
	void trigger(Args&& ... args) {
		for (const auto & ref : callbacks) {
			ref.function(std::forward<Args>(args)...);
		}
	}

	inline void clear() {
		callbacks.clear();
	}
};
