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

#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanogui/button.h>
#include <nanogui/canvas.h>
#include <nanogui/shader.h>
#include <nanogui/renderpass.h>
#include <nanogui/formhelper.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "mapcanvas.h"

#include "traffic/osm.h"
#include "traffic/osm_graph.h"
#include "traffic/osm_mesh.h"
#include "traffic/parser.hpp"
#include "traffic/render.hpp"
#include "traffic/agent.h"

using namespace traffic;
using namespace glm;

#define NANOGUI_USE_OPENGL

using nanogui::Vector3f;
using nanogui::Vector2f;
using nanogui::Vector2i;
using nanogui::Shader;
using nanogui::Canvas;
using nanogui::ref;
using Vector2d = nanogui::Array<double, 2>;
using Vector3d = nanogui::Array<double, 3>;
using Vector4d = nanogui::Array<double, 4>;

using view_t = double;

class FullscreenLayout : public nanogui::Layout {
public:
	virtual void perform_layout(NVGcontext* ctx, nanogui::Widget* widget) const override;
	virtual Vector2i preferred_size(NVGcontext* ctx, const nanogui::Widget* widget) const override;
};

class TrafficApplication : public nanogui::Screen {
public:
	TrafficApplication();
	// ---- Events ---- //
	virtual bool resize_event(const Vector2i& size) override;
	virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;

	void update();

	virtual void draw(NVGcontext* ctx) override;

protected:
	double lastTime;
	ref<MapCanvas> m_canvas;

	ref<ConcurrencyManager> manager;
	ref<MapInfo> uiInfo;
	ref<MapForm> uiMap;
	std::shared_ptr<World> world;
};


int main(int argc, char** argv)
{
	try
	{
		nanogui::init();
		{
			nanogui::ref<TrafficApplication> app = new TrafficApplication();
			app->draw_all();
			app->set_visible(true);
			nanogui::mainloop((float)(1.0 / 60.0 * 1000.0));
		}
		nanogui::shutdown();
	}
	catch (const std::runtime_error& e)
	{
		std::string error_msg = std::string(
			"Caught a fatal error: ") + std::string(e.what());
#if defined(_WIN32)
		MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
#else
		std::cerr << error_msg << std::endl;
#endif
		return -1;
	}

	return 0;
}

TrafficApplication::TrafficApplication() : nanogui::Screen(
	Vector2i(800, 600), "TrafficSim", true)
{

	using namespace nanogui;
	manager = new ConcurrencyManager();

	world = std::make_shared<World>(manager.get());
	world->loadMap("maps/warendorf.xmlmap");
	auto points = std::make_shared<std::vector<glm::vec2>>(generateMesh(*world->getMap()));
	auto colors = std::make_shared<std::vector<glm::vec3>>(points->size(), glm::vec3(1.0f, 1.0f, 1.0f));


	m_canvas = new MapCanvas(this, world->getMap());
	m_canvas->set_layout(new FullscreenLayout());
	m_canvas->setData(colors, points);
	//m_canvas->setChunkData(chunks);
	m_canvas->setActive(true);
	m_canvas->set_background_color({ 100, 100, 100, 255 });

	uiMap = new MapForm(this, {10, 10}, m_canvas.get());
	uiInfo = new MapInfo(this, {10, 340}, world.get());

	// Applies the forms
	m_canvas->setForm(uiMap);

	perform_layout();

	// collects the initial time stamp for the main loop //
	lastTime = glfwGetTime();
}

bool TrafficApplication::resize_event(const Vector2i& size)
{
	nanogui::Screen::resize_event(size);
	m_canvas->set_size(size);
	return true;
}

bool TrafficApplication::keyboard_event(int key, int scancode, int action, int modifiers)
{
	if (Screen::keyboard_event(key, scancode, action, modifiers))
		return true;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		set_visible(false);
		return true;
	}
	return false;
}

void TrafficApplication::update()
{
	double nextTime = glfwGetTime();
	double dt = nextTime - lastTime;
	m_canvas->update(dt);
	lastTime = nextTime;

}

void TrafficApplication::draw(NVGcontext* ctx)
{
	update();
	Screen::draw(ctx);
}

void FullscreenLayout::perform_layout(NVGcontext* ctx, nanogui::Widget* widget) const
{
}

Vector2i FullscreenLayout::preferred_size(NVGcontext* ctx, const nanogui::Widget* widget) const
{
	return widget->parent()->size();
}