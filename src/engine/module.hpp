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

#ifndef LT_MODULE_H
#define LT_MODULE_H

// Enum containing platform types
#define LT_TARGET_LINUX 0 
#define LT_TARGET_WIN 1

// __unix__ is usually defined by compilers targeting Unix systems
#ifdef __unix__
    #define LT_PLATFORM_TARGET LT_TARGET_LINUX
    #include <unistd.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #define EXPORT
// _Win32 is usually defined by compilers targeting 32 or 64 bit Windows systems
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)   
    #define LT_PLATFORM_TARGET LT_TARGET_WIN
    #define _CRT_SECURE_NO_WARNINGS 1
    #define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
    #define NOMINMAX // Exclude min max macros
    #if defined(APIENTRY)
        #undef APIENTRY
    #endif
    #include <stdio.h>
    #include <windows.h> // Windows Header Files

    #ifdef DLL_MAKE
        #define EXPORT __declspec(dllexport)
    #else
        #define EXPORT __declspec(dllimport)
    #endif

    #ifdef _WIN64
        // Specific Windows (64-bit only)
    #else
        // Specific something for Windows (32-bit only)
    #endif
#endif

#include <assert.h>
#include <exception>
#include <iostream>

bool lt_check_gl_error(const char* cmd, int line, const char* file);

#ifndef LT_DEFINE_GL_MACRO
    #define LT_DEFINE_GL_MACRO 1
#endif

#ifdef LT_DEFINE_GL_MACRO
    #if defined(TARGET_RELEASE)
        #define CGL(cmd) cmd
    #else
        #define CGL(cmd)                          \
        do {                                    \
            cmd;                                \
            (void) lt_check_gl_error(#cmd, __LINE__, __FILE__); \
        } while (0)
    #endif
#else // defines empty macros
    #define CGL(cmd) cmd
#endif

#if !defined(GL_STACK_OVERFLOW)
    #define GL_STACK_OVERFLOW 0x0503
#endif

#if !defined(GL_STACK_UNDERFLOW)
    #define GL_STACK_UNDERFLOW 0x0504
#endif


#ifdef LT_NANOGUI_BACKEND
    #ifndef NANOGUI_USE_OPENGL
        #define NANOGUI_USE_OPENGL
    #endif
    #include <nanogui/screen.h>
    #include <nanogui/opengl.h>
    #include <nanogui/window.h>
    #include <nanogui/button.h>
    #include <nanogui/canvas.h>
    #include <nanogui/shader.h>
    #include <nanogui/renderpass.h>
    #include <GLFW/glfw3.h>
#elif LT_GLFW_BACKEND
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad/glad.h>
#else
    #error "Could not find valid backend"
#endif

#endif
