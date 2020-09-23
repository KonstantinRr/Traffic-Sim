/// BEGIN LICENSE
///
/// Copyright 2020 Konstantin Rolf <konstantin.rolf@gmail.com>
/// Permission is hereby granted, free of charge, to any person obtaining a copy of
/// this software and associated documentation files (the "Software"), to deal in
/// the Software without restriction, including without limitation the rights to
/// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/// of the Software, and to permit persons to whom the Software is furnished to do
/// so, subject to the following conditions:
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
/// END LICENSE

#include "com.h"

bool lt_check_gl_error(const char *cmd, int line, const char *file) {
     GLenum err = glGetError();
     const char* msg = nullptr;

     switch (err) {
     case GL_NO_ERROR:
         // printf("OK: %s\n", cmd);
         return false;

     case GL_INVALID_ENUM:
         msg = "invalid enumeration";
         break;

     case GL_INVALID_VALUE:
         msg = "invalid value";
         break;

     case GL_INVALID_OPERATION:
         msg = "invalid operation";
         break;

     case GL_INVALID_FRAMEBUFFER_OPERATION:
         msg = "invalid framebuffer operation";
         break;

     case GL_OUT_OF_MEMORY:
         msg = "out of memory";
         break;

     case GL_STACK_UNDERFLOW:
         msg = "stack underflow";
         break;

     case GL_STACK_OVERFLOW:
         msg = "stack overflow";
         break;

     default:
         msg = "unknown error";
         break;
     }

     fprintf(stderr, "OpenGL error (%s) during operation \"%s\" at %s:%d!\n",
        msg, cmd, file, line);
     return true;
}
