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

#ifndef LT_WINDOW_HPP
#define LT_WINDOW_HPP

#include "module.hpp"
#include <GLFW/glfw3.h>
#include "shader.hpp"

namespace lt
{
    class SizedObject {
    public:
        virtual int width();
        virtual int height();

        void setWidth(int width);
        void setHeight(int height);
    protected:
        int w, h;
    };

    class Engine {
    public:
        void init(const std::string &name, size_t width, size_t height);
        void mainloop();
        void exit();

        void setPipeline(lt::render::Renderable *pipeline);

    protected:
        lt::render::Renderable *k_pipeline = nullptr;
        GLFWwindow* k_window = nullptr;
    };
}

#endif
