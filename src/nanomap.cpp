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

#include "nanomap.hpp"

using namespace traffic;

// ---- View Conversion ---- //

double toRadians(double degrees) {
    return degrees / (180.0 * 3.1415); // TODO
}
double toDegrees(double radians) {
    return radians * (180.0 / 3.1415); // TODO
}

nanogui::Matrix4f toView(const glm::mat4& value)
{
	nanogui::Matrix4f f;
	memcpy(f.m, glm::value_ptr(value), 16 * sizeof(float));
	return f;
}

nanogui::Matrix3f toView(const glm::mat3& value)
{
	nanogui::Matrix3f f;
	memcpy(f.m, glm::value_ptr(value), 9 * sizeof(float));
	return f;
}

nanogui::Matrix2f toView(const glm::mat2& value)
{
	nanogui::Matrix2f f;
	memcpy(f.m, glm::value_ptr(value), 4 * sizeof(float));
	return f;
}

bool MapCanvasNano::mouse_button_event(
	const Vector2i& p, int button, bool down, int modifiers) {
	Canvas::mouse_button_event(p, button, down, modifiers);
	Vector2d position = toView(
		k_canvas.viewToPlane(k_canvas.windowToView(toGLM(p))));
	if (button == GLFW_MOUSE_BUTTON_1 && down) {
		k_canvas.cb_leftclick().trigger(k_canvas.getCursor());
		return true;
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && down) {
		k_canvas.cb_rightclick().trigger(k_canvas.getCursor());
		return true;
	}
	return false;
}

bool MapCanvasNano::mouse_drag_event(
	const Vector2i& p, const Vector2i& rel, int button, int modifiers) {
	Canvas::mouse_drag_event(p, rel, button, modifiers);
	if (button == 0b01)
		k_canvas.applyTranslation(
            k_canvas.scaleWindowDistance(toGLM(rel)));
	else if (button == 0b10)
		k_canvas.applyZoom(rel.y());
	else if (button == 0b11)
		k_canvas.applyRotation(rel.y() * 0.01);
	return true;
}

bool MapCanvasNano::mouse_motion_event(
	const Vector2i& p, const Vector2i& rel, int button, int modifiers)
{
	Canvas::mouse_motion_event(p, rel, button, modifiers);
	glm::dvec2 cursor = k_canvas.viewToPlane(k_canvas.windowToView(toGLM(p))); // TODO change cursor
	k_canvas.cb_cursor_moved().trigger(cursor);
	return true;
}

bool MapCanvasNano::scroll_event(const Vector2i& p, const Vector2f& rel)
{
	Canvas::scroll_event(p, rel);
	k_canvas.setZoom(std::clamp(
		k_canvas.getZoom() * pow(0.94, -rel.y()), 2.0, 1000.0));
	return true;
}


void MapCanvasNano::updateKeys(double dt)
{
	GLFWwindow* window = screen()->glfw_window();
	double arrowSpeed = dt * 0.8;

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		k_canvas.applyZoom(4.0);
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		k_canvas.applyZoom(-4.0);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		k_canvas.applyTranslation(glm::dvec2(arrowSpeed, 0.0) / k_canvas.getZoom());
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		k_canvas.applyTranslation(glm::dvec2(-arrowSpeed, 0.0) / k_canvas.getZoom());

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		k_canvas.applyTranslation(glm::dvec2(0.0, -arrowSpeed) / k_canvas.getZoom());
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		k_canvas.applyTranslation(glm::dvec2(0.0, arrowSpeed) / k_canvas.getZoom());
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		k_canvas.setRotation(k_canvas.getRotation() + 0.01);
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		k_canvas.setRotation(k_canvas.getRotation() - 0.01);
}

void MapCanvasNano::update(double dt)
{
	updateKeys(dt);
}


MapCanvasNano::MapCanvasNano(Widget* parent, lt::SizedObject *sized,
	std::shared_ptr<traffic::OSMSegment> map)
	: nanogui::Canvas(parent, 1), k_canvas(map, sized) { }

bool MapCanvasNano::keyboard_event(int key, int scancode, int action, int modifiers)
{
	//if (key == GLFW_KEY_C && action == GLFW_PRESS) {
	//	m_render_chunk = !m_render_chunk;
	//}
	return true;
}



MapForm::MapForm(nanogui::Screen* parent, Vector2i pos, MapCanvasNano* canvas) : FormHelper(parent)
{
	setCanvas(canvas);
	set_fixed_size(Vector2i(100, 20));
	m_window = add_window(pos, "Position panel");

	add_group("Position");
	add_variable<double>("Latitude",
		[this](double value) { if (m_canvas) m_canvas->canvas().setLatitude(value); },
		[this]() { return m_canvas ? m_canvas->canvas().getLatitude() : 0.0; });
	add_variable<double>("Longitude",
		[this](double value) { if (m_canvas) m_canvas->canvas().setLongitude(value); },
		[this]() { return m_canvas ? m_canvas->canvas().getLongitude() : 0.0; });
	add_variable<double>("Zoom",
		[this](double value) { if (m_canvas) m_canvas->canvas().setZoom(value); },
		[this]() { return m_canvas ? m_canvas->canvas().getZoom() : 0.0; });
	add_variable<double>("Rotation",
		[this](double value) { if (m_canvas) m_canvas->canvas().setRotation(
            toRadians(std::fmod(value, 360.0))); },
		[this]() { return m_canvas ? std::fmod(toDegrees(
            m_canvas->canvas().getRotation()), 360.0) : 0.0; });
	add_button("Reset View", [this] { if (m_canvas) m_canvas->canvas().resetView(); });
	
	add_group("Cursor");
	add_variable<double>("Latitude",
		[this](double value) {},
		[this]() { return m_canvas ? m_canvas->canvas().getCursorLatitude() : 0.0; }, false);
	add_variable<double>("Longitude",
		[this](double val) {},
		[this]() { return m_canvas ? m_canvas->canvas().getCursorLongitude() : 0.0; }, false);
}

MapCanvasNano* MapForm::getCanvas() const noexcept
{
	return m_canvas;
}

void MapForm::setCanvas(MapCanvasNano* canvas) noexcept
{
	m_canvas = canvas;
	if (m_canvas)
	{
		m_canvas->canvas().cb_cursor_moved().listen([this](Vector2d) { refresh(); });
		m_canvas->canvas().cb_zoom_changed().listen([this](double) { refresh(); });
		m_canvas->canvas().cb_rotation_changed().listen([this](double) { refresh(); });
		m_canvas->canvas().cb_map_moved().listen([this](Vector2d) { refresh(); });
	}
}

MapInfo::MapInfo(nanogui::Screen* parent, Vector2i pos,
	traffic::World* world, MapCanvasNano* canvas) : FormHelper(parent)
{
	set_fixed_size(Vector2i(100, 20));
	m_window = add_window(pos, "Position panel");
	m_world = world;
	m_canvas = canvas;

	bool addBounds = false;
	bool addGeneralInfo = true;

	if (addBounds) {
		add_group("Bounds Info");
		add_variable<double>("Lat+",
			[this](double val) { },
			[this]() { return (m_world && m_world->getMap()) ?
				m_world->getMap()->getBoundingBox().upperLatBorder() : 0.0; }, false);
		add_variable<double>("Lat-",
			[this](double val) {},
			[this]() { return (m_world && m_world->getMap()) ?
				m_world->getMap()->getBoundingBox().lowerLatBorder() : 0.0; }, false);
		add_variable<double>("Lon+",
			[this](double val) {},
			[this]() { return (m_world && m_world->getMap()) ?
				m_world->getMap()->getBoundingBox().upperLonBorder() : 0.0; }, false);
		add_variable<double>("Lon-",
			[this](double val) {},
			[this]() { return (m_world && m_world->getMap()) ?
				m_world->getMap()->getBoundingBox().lowerLonBorder() : 0.0; }, false);
	}

	if (addGeneralInfo) {
		add_group("Memory Info");
		add_variable<size_t>("Nodes",
			[this](size_t) { },
			[this]() { return (m_world && m_world->getMap()) ?
				m_world->getMap()->getNodeCount() : 0; }, false);
		add_variable<size_t>("Ways",
			[this](size_t) {},
			[this]() { return (m_world && m_world->getMap()) ?
				m_world->getMap()->getWayCount() : 0; }, false);
		add_variable<size_t>("Relations",
			[this](size_t) {},
			[this]() { return (m_world && m_world->getMap()) ?
				m_world->getMap()->getRelationCount() : 0; }, false);
	}

	add_button("Choose File", [this]() {
		using namespace std;
		vector<pair<string, string>> vect{
			make_pair<string, string>("xmlmap", "OSM File format"),
			make_pair<string, string>("osm", "OSM File format"),
		};
		string file = nanogui::file_dialog(vect, false);
		if (m_world) {
			m_world->loadMap(file);
			if (m_canvas) {
				m_canvas->canvas().loadMap(m_world->getMap());
				m_canvas->canvas().loadHighwayMap(m_world->getHighwayMap());
			}
		}
	});

}

traffic::World* MapInfo::getWorld() const noexcept
{
	return m_world;
}

void MapInfo::setWorld(traffic::World* world) noexcept
{
	m_world = world;
}

MapCanvasNano* MapInfo::getCanvas() const noexcept { return m_canvas; }

void MapInfo::setCanvas(MapCanvasNano* canvas) { m_canvas = canvas; }

MapDialogPath::MapDialogPath(
	nanogui::Screen* parent, Vector2i pos, World *world,
	MapCanvasNano* canvas, MapContextDialog *contextMenu)
	: nanogui::FormHelper(parent)
{
	clear();

	set_fixed_size(Vector2i(100, 20));
	m_window = add_window(pos, "Position panel");
	m_canvas = canvas;
	m_world = world;
	k_context = contextMenu;

	add_group("Start");
	add_variable<double>("Latitude", m_start_lat);
	add_variable<double>("Longitude", m_start_lon);
	add_group("End");
	add_variable<double>("Latitude", m_stop_lat);
	add_variable<double>("Longitude", m_stop_lon);
	add_variable<double>("Distance",
		[this](double val) { },
		[this]() { return m_canvas ?
			m_canvas->canvas().getDistance(
				toGLM(getStart()), toGLM(getStop())) : 0.0; }, false);
	add_button("Calculate Path", [this](){
		auto& graph = m_world->getGraph();
		GraphNode& idStart = graph->findClosestNode(Point(m_start_lat, m_start_lon));
		GraphNode& idStop = graph->findClosestNode(Point(m_stop_lat, m_stop_lon));
		std::cout << "Searching route from " << idStart.nodeID << " " << idStop.nodeID << "\n";
		Route r = graph->findRoute(idStart.nodeID, idStop.nodeID);
		for (int64_t id : r.nodes) {
			std::cout << "Node: " << id << "\n";
		}
		m_canvas->canvas().loadRoute(r, graph->getXMLMap());

	});

	if (k_context)
	{
		k_context->addContextButton("Set Start", [this]() {
			if (m_canvas) {
				Vector2d pos = toView(m_canvas->canvas().windowToPosition(
					toGLM(k_context->position())));
				setStart(pos);
				m_canvas->request_focus(); // moves focus back to canvas
			}
		});
		k_context->addContextButton("Set End", [this]() {
			if (m_canvas) {
				Vector2d pos = toView(m_canvas->canvas().windowToPosition(
					toGLM(k_context->position())));
				setStop(pos);
				m_canvas->request_focus(); // moves focus back to canvas
			}
		});
	}
}

void MapDialogPath::clear()
{
	m_start_lat = 0.0, m_start_lon = 0.0;
	m_stop_lat = 0.0, m_stop_lon = 0.0;
}

void MapDialogPath::setStart(Vector2d start) {
	m_start_lat = start.x();
	m_start_lon = start.y();
	refresh();
}
void MapDialogPath::setStop(Vector2d stop) {
	m_stop_lat = stop.x();
	m_stop_lon = stop.y();
	refresh();
}

Vector2d MapDialogPath::getStart() const { return Vector2d(m_start_lat, m_start_lon); }
Vector2d MapDialogPath::getStop() const { return Vector2d(m_stop_lat, m_stop_lon); }


MapContextDialog::MapContextDialog(nanogui::Widget* parent, MapCanvasNano *canvas)
	: nanogui::Window(parent, "Context Menu")
{
	set_modal(true);
	k_canvas = canvas;

	set_width(100);
	set_layout(new nanogui::BoxLayout(
		nanogui::Orientation::Vertical,
		nanogui::Alignment::Fill, 0, 0));

	if (k_canvas)
	{
		k_canvas->canvas().cb_leftclick().listen([this](Vector2d) {
			closeListener().trigger();
			set_visible(false);
		});
		k_canvas->canvas().cb_rightclick().listen([this](Vector2d pos) {
			openListener().trigger();

			Vector2d p1 = toView(k_canvas->canvas().planeToView((
				k_canvas->canvas().getCursorPlane())));
			Vector2i p2 = toView(k_canvas->canvas().viewToWindow(toGLM(p1)));
			set_position(p2);
			set_visible(true);
		});
	}
}

void MapContextDialog::addSeparator() {

}
void MapContextDialog::addContextButton(
	const std::string &title, std::function<void()> callback)
{
	nanogui::Button *obj = new nanogui::Button(this, title);
	obj->set_callback(callback);
}

Listener<void()>& MapContextDialog::openListener() { return k_listener_open; }
Listener<void()>& MapContextDialog::closeListener() { return k_listener_close; }
