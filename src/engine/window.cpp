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

#include "window.hpp"

#include <spdlog/spdlog.h>

using namespace lt;

void error_callback(int error, const char* description) {
    spdlog::error("Captured GLFW Error: {}", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void glFramebufferChanged(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


void Engine::init(const std::string &name, size_t width, size_t height) {
    spdlog::info("Initializing GLFW Environment");
    if (!glfwInit()) {
        // Initialization failed
        const char *errorMSG = "GLFW Initialization failed!";
        spdlog::error(errorMSG);
        throw std::runtime_error(errorMSG);
    }
    glfwSetErrorCallback(error_callback);

    spdlog::info("Creating GLFW Window");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    k_window = glfwCreateWindow(
        static_cast<int>(width), static_cast<int>(height),
        name.c_str(), NULL, NULL);
    if (!k_window) {
        const char *errorMSG = "GLFW Window Initialization failed!";
        spdlog::error(errorMSG);
        throw std::runtime_error(errorMSG);
        // Window or OpenGL context creation failed
    }
    glfwSetKeyCallback(k_window, key_callback);
    
    glfwMakeContextCurrent(k_window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        const char *errorMSG = "GLAD Initialization failed!";
        spdlog::error(errorMSG);
        throw std::runtime_error(errorMSG);
    }

    // initializes the window sizes
    int w, h;
    glfwGetFramebufferSize(k_window, &w, &h);
    glViewport(0, 0, w, h);
}

void Engine::mainloop() {
    glfwSwapInterval(1); // VSYNC
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(k_window)) {
        double nextTime = glfwGetTime();
        double dt = nextTime - lastTime;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        if (k_pipeline)
            k_pipeline->render();

        glfwSwapBuffers(k_window);
        glfwPollEvents();
    }
}

void Engine::setPipeline(lt::render::Renderable *pipeline) {
    k_pipeline = pipeline;
}

void Engine::exit() {
    spdlog::info("Terminating GLFW Environment");
    if (k_window)
        glfwDestroyWindow(k_window);
    glfwTerminate();
}

int SizedObject::width() { return w; }
int SizedObject::height() { return h; }

void SizedObject::setWidth(int width) { w = width; }
void SizedObject::setHeight(int height) { h = height; }