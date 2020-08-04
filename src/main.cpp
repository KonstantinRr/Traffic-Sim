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
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include <cptl.hpp>

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

std::shared_ptr<XMLMap> initMap(ctpl::thread_pool &pool)
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

	auto map = std::make_shared<XMLMap>(
		parseXMLMap("warendorf.xmlmap", pool));

	map->summary();
	
	*map = map->findSquareNodes(initRect);
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



class MapCanvas : public Canvas 
{
protected:
	ref<Shader> m_shader;
	ref<Shader> m_chunk_shader;
	
	std::shared_ptr<XMLMap> map;
	std::shared_ptr<std::vector<glm::vec3>> colors;
	std::shared_ptr<std::vector<glm::vec2>> points;
	std::shared_ptr<std::vector<glm::vec2>> chunks;
	bool m_active;
	bool m_render_chunk;

	Vector2d position;
	double m_zoom;

public:
	MapCanvas(Widget* parent, std::shared_ptr<XMLMap> map) : Canvas(parent, 1) {
		using namespace nanogui;
		this->map = map;
		m_active = false;
		m_render_chunk = true;

		auto centerPoint = sphereToPlane(map->getRect().getCenter().toVec());
		position = {
			static_cast<float>(-centerPoint.x),
			static_cast<float>(-centerPoint.y)};
		m_zoom = 25.0f;
		m_chunk_shader = new Shader(
			render_pass(), "shader_chunk",
			getChunkVertex(), getChunkFragment()
		);
		m_shader = new Shader(
			render_pass(), "shader_map",
			getLineVertex(), getLineFragment()
		);
	}

	Vector2d transformView(Vector2i vec) {
		return Vector2d(
			(double)vec.x() * 2.0 / width(),
			(double)-vec.y() * 1.0 / height()
		);
	}

	
	virtual bool mouse_button_event(
		const Vector2i& p, int button, bool down, int modifiers) override
	{
		return Canvas::mouse_button_event(p, button, down, modifiers);
	}

	virtual bool mouse_drag_event(
		const Vector2i& p, const Vector2i& rel, int button, int modifiers) override
	{
		Canvas::mouse_drag_event(p, rel, button, modifiers);
		position += transformView(rel) / m_zoom;
		return true;
	}
	
	virtual bool scroll_event(const Vector2i& p, const Vector2f& rel)
	{
		Canvas::scroll_event(p, rel);
		m_zoom = std::clamp(m_zoom * pow(0.94, -rel.y()), 2.0, 1000.0);
		return true;
	}

	void setData(
		std::shared_ptr<std::vector<glm::vec3>> colors,
		std::shared_ptr<std::vector<glm::vec2>> points) {
		using namespace nanogui;
		this->colors = colors;
		this->points = points;
		m_shader->set_buffer("vVertex", VariableType::Float32,
			{ points->size(), 2 }, points->data());
		m_shader->set_buffer("color", VariableType::Float32,
			{ colors->size(), 3 }, colors->data());
	}

	void setChunkData(
		std::shared_ptr<std::vector<glm::vec2>> points) {
		using namespace nanogui;
		this->chunks = points;
		m_chunk_shader->set_buffer("vVertex", VariableType::Float32,
			{ chunks->size(), 2 }, chunks->data());
	}

	void setActive(bool active) {
		m_active = true;
	}

	void set_zoom(double zoom) {
		m_zoom = zoom;
	}

	void updateKeys() {
		//TrafficApplication* app = static_cast<TrafficApplication*>(parent());
		/*
		window().
		int v = 0;
		double arrowSpeed = 0.05;
		case GLFW_KEY_E: v = -1; break;
		case GLFW_KEY_R: v = 1; break;
		case GLFW_KEY_LEFT:		position += Vector2d(-arrowSpeed, 0.0) / m_zoom; break;
		case GLFW_KEY_RIGHT:	position += Vector2d(arrowSpeed, 0.0) / m_zoom; break;
		case GLFW_KEY_UP:		position += Vector2d(0.0, -arrowSpeed) / m_zoom; break;
		case GLFW_KEY_DOWN:		position += Vector2d(0.0, arrowSpeed) / m_zoom; break;
		m_zoom = std::clamp(m_zoom * pow(0.94, v), 2.0, 1000.0);
		*/
	}

	virtual void draw_contents() override {
		using namespace nanogui;
		set_size(parent()->size());
		if (m_active) {
			Matrix4f matrix = Matrix4f::translate(
				Vector3f(position.x(), position.y(), 0.0f));
			Matrix4f scale = Matrix4f::scale(
				Vector3f(m_zoom, m_zoom * width() / height(), 1.0f));

			if (m_render_chunk)
			{
				m_chunk_shader->set_uniform("mvp", scale * matrix);
				m_chunk_shader->set_uniform("color", Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
				m_chunk_shader->begin();
				m_shader->draw_array(Shader::PrimitiveType::Line, 0, chunks->size(), false);
				m_chunk_shader->end();
			}

			m_shader->set_uniform("mvp", scale * matrix);
			m_shader->begin();
			m_shader->draw_array(Shader::PrimitiveType::Line, 0, points->size(), false);
			m_shader->end();
		}
	}

	virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override {
		int v = 0;
		double arrowSpeed = 0.05;
		switch (key) {
			case GLFW_KEY_E: v = -1; break;
			case GLFW_KEY_R: v = 1; break;
			case GLFW_KEY_LEFT:		position += Vector2d(-arrowSpeed, 0.0) / m_zoom; break;
			case GLFW_KEY_RIGHT:	position += Vector2d(arrowSpeed, 0.0) / m_zoom; break;
			case GLFW_KEY_UP:		position += Vector2d(0.0, -arrowSpeed) / m_zoom; break;
			case GLFW_KEY_DOWN:		position += Vector2d(0.0, arrowSpeed) / m_zoom; break;
		}
		m_zoom = std::clamp(m_zoom * pow(0.94, v), 2.0, 1000.0);
		return v != 0;
	}
};

class TrafficApplication : public nanogui::Screen
{
public:
	TrafficApplication() : nanogui::Screen(
		Vector2i(800, 600), "TrafficSim", true),
		pool(8)
	{
		using namespace nanogui;
		auto map = initMap(pool);
		world = std::make_shared<World>(map, 0.005);
		auto points = std::make_shared<std::vector<glm::vec2>>(generateMesh(*map));
		auto colors = std::make_shared<std::vector<glm::vec3>>(points->size(), glm::vec3(1.0f, 1.0f, 1.0f));
		auto chunks = std::make_shared<std::vector<glm::vec2>>(generateChunkMesh(*world));

		m_canvas = new MapCanvas(this, map);
		m_canvas->setData(colors, points);
		m_canvas->setChunkData(chunks);
		m_canvas->setActive(true);
		m_canvas->set_background_color({ 100, 100, 100, 255 });

		Window* window = new Window(this, "Canvas widget demo");
		window->set_position(Vector2i(15, 15));
		window->set_layout(new GroupLayout());

		Widget* tools = new Widget(window);
		tools->set_layout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 5));

		Button* b0 = new Button(tools, "Random Background");
		b0->set_callback([this]() {
			m_canvas->set_background_color(
				Vector4i(rand() % 256, rand() % 256, rand() % 256, 255));
			});

		Button* b1 = new Button(tools, "Random Rotation");
		b1->set_callback([this]() {
			//m_canvas->set_rotation((float)Pi * rand() / (float)RAND_MAX);
			});

		perform_layout();
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

	virtual void draw(NVGcontext* ctx) {
		Screen::draw(ctx);
	}

protected:
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
			nanogui::mainloop(1 / 60.f * 1000);
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
