/*
	src/example4.cpp -- C++ version of an example application that shows
	how to use the OpenGL widget. For a Python implementation, see
	'../python/example4.py'.

	NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
	The widget drawing code is based on the NanoVG demo application
	by Mikko Mononen.

	All rights reserved. Use of this source code is governed by a
	BSD-style license that can be found in the LICENSE.txt file.
*/

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

#include <cptl.hpp>

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

class FullscreenLayout : public nanogui::Layout
{
public:
	virtual void perform_layout(NVGcontext* ctx, nanogui::Widget* widget) const override {
		
	}

	virtual Vector2i preferred_size(NVGcontext* ctx, const nanogui::Widget* widget) const override {
		return widget->parent()->size();
	}
};

std::shared_ptr<OSMSegment> initMap(ctpl::thread_pool &pool)
{
	// Groningen coordinates
	// tl,tr [53.265301,6.465842][53.265301,6.675939]
	// br,bl [53.144829,6.675939][53.144829, 6.465842]	
	//Rect initRect = Rect::fromBorders(
	//	53.144829, 53.265301, 6.465842, 6.675939);

	// Warendorf coordinates
	// tl,tr [51.9362,7.9553][51.9362,8.0259]
	// br,bl [51.9782,8.0259][51.9362,7.9553]
	Rect initRect = Rect::fromBorders(
		51.9362, 51.9782, 7.9553, 8.0259);

	ParseTimings timings;
	ParseArguments args;
	args.file = "warendorf.xmlmap";
	args.threads = 8;
	args.pool = &pool;
	args.timings = &timings;
	auto map = std::make_shared<OSMSegment>(
		parseXMLMap(args));
	timings.summary();
	map->summary();
	
	//*map = map->findSquareNodes(initRect);
	/*
	*map = map->findNodes(
		[](const OSMNode& nd) { return nd.hasTag("highway"); },
		[](const OSMWay& wd) { return wd.hasTag("highway"); },
		[](const OSMWay&, const OSMNode&) { return true; }
	);
	*/
	
	map->summary();
	return map;

	//size_t size = 8192;
	//auto img = make_shared<ImageRGB8>(drawMap(*map, size, size));
	//writeImage(*img, "file.png", "title");
	//printf("Creating Graph Map\n");
	//int64_t start = (*map->getWays())[10].getNodes()[0];
	//int64_t end = (*map->getWays())[56].getNodes()[0];
	//Graph graph(map);
	//graph.checkConsistency();
	//auto route = graph.findRoute(start, end);
	////renderMap();
	//printf("Finished %d\n", route.nodes.size());
}



class TrafficApplication : public nanogui::Screen
{
public:
	TrafficApplication();

	virtual bool resize_event(const Vector2i& size) override {
		nanogui::Screen::resize_event(size);
		m_canvas->set_size(size);
		return true;
	}

	virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override {
		if (Screen::keyboard_event(key, scancode, action, modifiers))
			return true;
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			set_visible(false);
			return true;
		}
		return false;
	}

	void update() {
		double nextTime = glfwGetTime();
		double dt = nextTime - lastTime;
		m_canvas->update(dt);
		lastTime = nextTime;
		
	}

	virtual void draw(NVGcontext* ctx) {
		update();
		Screen::draw(ctx);
	}

protected:
	double lastTime;
	ctpl::thread_pool pool;
	MapCanvas* m_canvas;

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
	Vector2i(800, 600), "TrafficSim", true),
	pool(12)
{
	using namespace nanogui;
	auto map = initMap(pool);
	world = std::make_shared<World>(map);
	auto points = std::make_shared<std::vector<glm::vec2>>(generateMesh(*map));
	auto colors = std::make_shared<std::vector<glm::vec3>>(points->size(), glm::vec3(1.0f, 1.0f, 1.0f));


	m_canvas = new MapCanvas(this, map);
	m_canvas->set_layout(new FullscreenLayout());
	m_canvas->setData(colors, points);
	//m_canvas->setChunkData(chunks);
	m_canvas->setActive(true);
	m_canvas->set_background_color({ 100, 100, 100, 255 });

	uiMap = new MapForm(this, {10, 10}, m_canvas);
	uiInfo = new MapInfo(this, {10, 340}, world.get());

	// Applies the forms
	m_canvas->setForm(uiMap);


	perform_layout();
	//uiMap->window()->set_position({10, 10});
	//uiInfo->window()->set_position({ 10, 10 + uiMap->window()->height() });
	//perform_layout();

	lastTime = glfwGetTime();
}