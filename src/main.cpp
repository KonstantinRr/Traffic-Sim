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
	TrafficApplication() : nanogui::Screen(
		Vector2i(800, 600), "TrafficSim", true),
		pool(12)
	{
		using namespace nanogui;
		auto map = initMap(pool);
		world = std::make_shared<World>(map);
		auto points = std::make_shared<std::vector<glm::vec2>>(generateMesh(*map));
		auto colors = std::make_shared<std::vector<glm::vec3>>(points->size(), glm::vec3( 1.0f, 1.0f, 1.0f));
		//for (size_t i = 0; i < colors->size(); i++)
		//	(*colors)[i] = glm::vec3(rand() % 256 / 255.0f, rand() % 256 / 255.0f, rand() % 256 / 255.0f);
		//auto chunks = std::make_shared<std::vector<glm::vec2>>(generateChunkMesh(*world));


		m_canvas = new MapCanvas(this, map);
		m_canvas->set_layout(new FullscreenLayout());
		m_canvas->setData(colors, points);
		//m_canvas->setChunkData(chunks);
		m_canvas->setActive(true);
		m_canvas->set_background_color({ 100, 100, 100, 255 });


		MapForm* form = new MapForm(this, m_canvas);
		FormHelper *gui = new FormHelper(this);
		gui->set_fixed_size(Vector2i(100, 20));
		ref<Window> window = gui->add_window(Vector2i(10, 10), "Action Panel");

		int64_t a = 0;
		int32_t ver = 0;
		gui->add_group("General");
		gui->add_variable("ID", a);
		gui->add_variable("Version", ver);

		gui->add_button("Choose File", [this, &a, gui]() {
			using namespace std;
			vector<pair<string, string>> vect{
				make_pair<string, string>("xmlmap", "OSM File format"),
				make_pair<string, string>("osm", "OSM File format"),
			};
			a = 10;
			gui->refresh();
			file_dialog(vect, true);
		});

		gui->refresh();

		// Applies the forms
		m_canvas->setForm(form);


		perform_layout();
		lastTime = glfwGetTime();
	}

	virtual bool keyboard_event(int key, int scancode, int action, int modifiers) {
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

MapForm::MapForm(nanogui::Screen* parent, MapCanvas *canvas) : FormHelper(parent)
{
	m_canvas = canvas;
	set_fixed_size(Vector2i(100, 20));
	m_window = add_window(Vector2i(10, 10), "Position panel");
	add_group("Position");
	add_variable<double>("Latitude",
		[this](double value) { if (this->m_canvas) this->m_canvas->setLatitude(value); },
		[this]() { return this->m_canvas ? (double)this->m_canvas->getLatitude() : 0.0; });
	add_variable<double>("Longitude",
		[this](double value) { if (this->m_canvas) this->m_canvas->setLongitude(value); },
		[this]() { return this->m_canvas ? (double)this->m_canvas->getLongitude() : 0.0f; });
	add_variable<double>("Zoom",
		[this](double value) { if (m_canvas) m_canvas->setZoom(value); },
		[this]() { return m_canvas ? m_canvas->getZoom() : 0.0; });

	add_group("Cursor");
	add_variable<double>("Latitude",
		[this](double value) { },
		[this]() { return m_canvas ? m_canvas->getCursorLatitude() : 0.0; }, false);
	add_variable<double>("Longitude",
		[this](double val) { },
		[this]() { return m_canvas ? m_canvas->getCursorLongitude() : 0.0; }, false);

}

MapCanvas* MapForm::getCanvas() const noexcept
{
	return m_canvas;
}

void MapForm::setCanvas(MapCanvas* canvas) noexcept
{
	m_canvas = canvas;
}
