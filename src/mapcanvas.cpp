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

#include "mapcanvas.h"

#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "traffic/osm_mesh.h"

using namespace traffic;
using namespace glm;

// ---- View Conversion ---- //

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

// ---- MapCanvas ---- //

MapCanvas::MapCanvas(Widget* parent,
	std::shared_ptr<OSMSegment> world) : Canvas(parent, 1)
{
	m_active = false;
	m_render_chunk = false;
	m_mark_update = false;
	m_update_view = true;

	m_min_zoom = 2.0;
	m_max_zoom = 1000.0;

	// loadMap includes resetView
	if (world) loadMap(world);
	else resetView();

	// custom shader
	using namespace lt::render::shader;
	try {
		l_shader = std::make_shared<LineMemoryShader>();
		entities = std::make_shared<RenderList<Entity2D>>();
		l_comp = std::make_shared<RenderComponent< LineStageBuffer, LineMemoryShader>>();
		
		l_shader->create();
		l_comp->setShader(l_shader);
		l_comp->stageBuffer().renderList = entities;
		l_pipeline.addStage(l_comp);
		m_success = true;
	}
	catch (...) {
		m_success = false;
	}
}

Vector2d MapCanvas::scaleWindowDistance(Vector2i vec) {
	return Vector2d(
		vec.x() * 2.0 / width(),
		-vec.y() * 2.0 / width());
}

void MapCanvas::applyTranslation(Vector2d rel)
{
	Vector2d mx = toView(glm::rotate(toGLM(rel / m_zoom), -m_rotation));
	setPosition(position - mx);
}

void MapCanvas::applyZoom(double iterations)
{
	double scale = pow(0.99, iterations);
	double clamped = std::clamp(m_zoom * scale, m_min_zoom, m_max_zoom);
	setZoom(clamped);
}

void MapCanvas::applyRotation(double radians)
{
	setRotation(m_rotation + radians);
}

void MapCanvas::resetView()
{
	auto centerPoint = sphereToPlane(toGLM(getCenter()));
	position = { centerPoint.x, centerPoint.y };
	cursor = { 0.0, 0.0 };
	m_zoom = 25.0;
	m_rotation = 0.0;

	cb_map_moved().trigger(getPosition());
	cb_cursor_moved().trigger(getCursor());
	cb_rotation_changed().trigger(getRotation());
	cb_zoom_changed().trigger(getZoom());
}

void MapCanvas::setLatitude(double lat)
{
	setPosition(Vector2d(
		latitudeToPlane(lat, toGLM(getCenter())),
		position.y()));
}

void MapCanvas::setLongitude(double lon)
{
	setPosition(Vector2d(
		position.x(),
		longitudeToPlane(lon, toGLM(getCenter()))));
}

void MapCanvas::setLatLon(double lat, double lon)
{
	setPosition(Vector2d(
		latitudeToPlane(lat, toGLM(getCenter())),
		longitudeToPlane(lon, toGLM(getCenter()))));
}

void MapCanvas::setPosition(Vector2d pos)
{
	position = pos;
	cb_map_moved().trigger(getPosition());
	cb_view_changed().trigger(Rect());
}

void MapCanvas::setZoom(double zoom)
{
	m_zoom = zoom;
	cb_zoom_changed().trigger(getZoom());
	cb_view_changed().trigger(Rect());
}

void MapCanvas::setRotation(double rotation)
{
	m_rotation = rotation;
	cb_rotation_changed().trigger(getRotation());
}

double MapCanvas::getLatitude() const { return planeToLatitude(position.x(), toGLM(getCenter())); }
double MapCanvas::getLongitude() const { return planeToLongitude(position.y(), toGLM(getCenter())); }
double MapCanvas::getCursorLatitude() const { return planeToLatitude(cursor.x(), toGLM(getCenter())); }
double MapCanvas::getCursorLongitude() const { return planeToLongitude(cursor.y(), toGLM(getCenter())); }
Vector2d MapCanvas::getCursor() const { return Vector2d(getCursorLatitude(), getCursorLongitude()); }
Vector2d MapCanvas::getPosition() const { return Vector2d(getLatitude(), getLongitude()); }
Vector2d MapCanvas::getPositionPlane() const { return position;}
Vector2d MapCanvas::getCursorPlane() const { return cursor; }

double MapCanvas::getZoom() const { return m_zoom; }
double MapCanvas::getRotation() const { return m_rotation; }
double MapCanvas::getMinZoom() const { return m_min_zoom; }
double MapCanvas::getMaxZoom() const { return m_max_zoom; }

double MapCanvas::getDistance(Vector2d p1, Vector2d p2) const {
	return distance(toGLM(p1), toGLM(p2));
}

Vector2d MapCanvas::getCenter() const
{
	if (m_map) {
		return toView(m_map->getBoundingBox().getCenter().toVec());
	}
	else {
		return Vector2d(0.0, 0.0);
	}
}

void MapCanvas::loadMap(std::shared_ptr<traffic::OSMSegment> map)
{
	if (map) {
		m_map = map;
		l_mesh_map = genMeshFromMap(*map, { 1.0f, 1.0f, 1.0f});
		resetView();
	}
}

void MapCanvas::loadHighwayMap(std::shared_ptr<traffic::OSMSegment> map)
{
	if (map) {
		m_highway_map = map;
		l_mesh_highway = genMeshFromMap(*map, { 1.0f, 0.0f, 0.0f });
		resetView();
	}
}

void MapCanvas::loadRoute(const Route& route, std::shared_ptr<traffic::OSMSegment> map)
{
	std::vector<vec2> points = generateRouteMesh(route, *map);
	std::vector<vec3> colors(points.size(), glm::vec3(0.0f, 0.0f, 1.0f));
	l_mesh_routes.push_back(genMesh(std::move(points), std::move(colors)));
}

void MapCanvas::clearRoutes()
{
	l_mesh_routes.clear();
}



bool MapCanvas::hasMap() const { return m_map.get(); }

bool MapCanvas::mouse_button_event(
	const Vector2i& p, int button, bool down, int modifiers) {
	Canvas::mouse_button_event(p, button, down, modifiers);
	Vector2d position = viewToPlane(windowToView(p));
	if (button == GLFW_MOUSE_BUTTON_1 && down) {
		cb_leftclick().trigger(getCursor());
		return true;
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && down) {
		cb_rightclick().trigger(getCursor());
		return true;
	}
	return false;
}

bool MapCanvas::mouse_drag_event(
	const Vector2i& p, const Vector2i& rel, int button, int modifiers) {
	Canvas::mouse_drag_event(p, rel, button, modifiers);
	if (button == 0b01)
		applyTranslation(scaleWindowDistance(rel));
	else if (button == 0b10)
		applyZoom(rel.y());
	else if (button == 0b11)
		applyRotation(rel.y() * 0.01);
	return true;
}

bool MapCanvas::mouse_motion_event(
	const Vector2i& p, const Vector2i& rel, int button, int modifiers)
{
	Canvas::mouse_motion_event(p, rel, button, modifiers);
	cursor = viewToPlane(windowToView(p));
	cb_cursor_moved().trigger(getCursor());
	return true;
}

bool MapCanvas::scroll_event(const Vector2i& p, const Vector2f& rel)
{
	Canvas::scroll_event(p, rel);
	setZoom(std::clamp(m_zoom * pow(0.94, -rel.y()), 2.0, 1000.0));
	return true;
}

void MapCanvas::setChunkMesh(const std::vector<glm::vec2> &points)
{
}

void MapCanvas::setActive(bool active)
{
	m_active = active;
}

void MapCanvas::updateKeys(double dt)
{
	GLFWwindow* window = screen()->glfw_window();
	double arrowSpeed = dt * 0.8;

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		applyZoom(4.0);
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		applyZoom(-4.0);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		applyTranslation(Vector2d(arrowSpeed, 0.0) / m_zoom);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		applyTranslation(Vector2d(-arrowSpeed, 0.0) / m_zoom);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		applyTranslation(Vector2d(0.0, -arrowSpeed) / m_zoom);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		applyTranslation(Vector2d(0.0, arrowSpeed) / m_zoom);
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		setRotation(m_rotation + 0.01);
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		setRotation(m_rotation - 0.01);
}

void MapCanvas::update(double dt)
{
	updateKeys(dt);
}

// ---- Mesh ---- //

std::shared_ptr<Transformed4DEntity2D> MapCanvas::genMeshFromMap(
	const OSMSegment& seg, glm::vec3 color)
{
	std::vector<vec2> points = generateMesh(seg);
	std::vector<vec3> colors(points.size(), color);
	return genMesh(std::move(points), std::move(colors));
}

std::shared_ptr<Transformed4DEntity2D> MapCanvas::genMesh(
	std::vector<glm::vec2>&& points, std::vector<glm::vec3>&& colors)
{
	lt::resource::MeshBuilder2D builder;
	builder.setVertices(std::move(points));
	builder.setColors(std::move(colors));
	auto exp = builder.exporter()
		.addVertex().addColor()
		.exportData();

	std::shared_ptr<GLModel> model = std::make_shared<GLModel>(exp);
	return std::make_shared<Transformed4DEntity2D>(0, model);
}

void MapCanvas::clearMesh() {
	l_mesh_highway = nullptr;
	l_mesh_map = nullptr;
	l_mesh_routes.clear();
}

Vector2d MapCanvas::windowToView(Vector2i vec) const {
	return Vector2d(
		vec.x() * 2.0 / width() - 1.0,
		(height() - vec.y()) * 2.0 / height() - 1.0);
}

Vector2i MapCanvas::viewToWindow(Vector2d vec) const {
	return Vector2i(
		(int32_t)((vec.x() + 1.0) / 2.0 * width()),
		-((int32_t)((vec.y() + 1.0) / 2.0 * height()) - height())
	);
}

Vector2d MapCanvas::planeToView(const Vector2d& pos) const
{
	glm::dvec2 f = toGLM(pos - position);
	f = glm::rotate(f, m_rotation);
	f = f * dvec2(m_zoom, m_zoom * width() / height());
	return toView(f);
}

Vector2d MapCanvas::viewToPlane(const Vector2d& pos) const
{
	glm::dvec2 f = toGLM(pos) / dvec2(m_zoom, m_zoom * width() / height());
	f = glm::rotate(f, -m_rotation);
	f = f + toGLM(position);
	return toView(f);
}

Vector2d MapCanvas::planeToPosition(const Vector2d& pos) const
{
	return toView(planeToSphere(
		toGLM(pos), toGLM(getCenter())));
}

Vector2d MapCanvas::positionToPlane(const Vector2d& pos) const
{
	return toView(sphereToPlane(
		toGLM(pos), toGLM(getCenter())));
}

Vector2d MapCanvas::windowToPosition(Vector2i vec) const
{
	Vector2d view = windowToView(vec);
	Vector2d plane = viewToPlane(view);
	Vector2d pos = planeToPosition(plane);
	return pos;
}

Vector2i MapCanvas::positionToWindow(Vector2d vec) const
{
	Vector2d plane = positionToPlane(vec);
	Vector2d view = planeToView(plane);
	Vector2i window = viewToWindow(view);
	return window;
}

Matrix3f MapCanvas::transformPlaneToView3D() const
{
	glm::mat3 matrix(1.0f);
	glm::translate(matrix, vec2(toGLM(-position)));
	glm::rotate(matrix, (float)m_rotation);
	glm::scale(matrix, glm::vec2(m_zoom, m_zoom * width() / height()));
	return toView(matrix);
}

Matrix4f MapCanvas::transformPlaneToView4D() const {
	Matrix4f translate = Matrix4f::translate(
		Vector3f(-position.x(), -position.y(), 0.0f));
	Matrix4f rotation = Matrix4f::rotate(Vector3f(0.0f, 0.0f, 1.0f), m_rotation);
	Matrix4f scale = Matrix4f::scale(
		Vector3f(m_zoom, m_zoom * width() / height(), 1.0f));
	return scale * rotation * translate;
}
glm::mat3 MapCanvas::transformPlaneToView4DGLM() const {
	glm::mat3 mat(1.0f);
	mat = glm::translate(mat, glm::vec2(-position.x(), -position.y()));
	mat = glm::rotate(mat, (float)m_rotation);
	mat = glm::scale(mat, glm::vec2(m_zoom, m_zoom * width() / height()));
	return mat;
}

void MapCanvas::draw_contents()
{
	using namespace nanogui;
	if (m_active && m_success && hasMap()) {
		auto transform = transformPlaneToView4D();
		// Chunk rendering
		if (m_render_chunk)
		{

		}
		
		
		entities->clear();
		for (const auto& t : l_mesh_routes) {
			t->setTransform4D(toGLM(transform));
			entities->add(t);
		}
		l_mesh_map->setTransform4D(toGLM(transform));
		l_mesh_highway->setTransform4D(toGLM(transform));
		entities->add(l_mesh_map);
		entities->add(l_mesh_highway);
		l_pipeline.render();
	}
}

bool MapCanvas::keyboard_event(int key, int scancode, int action, int modifiers)
{
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		m_render_chunk = !m_render_chunk;
	}
	return true;
}


MapForm::MapForm(nanogui::Screen* parent, Vector2i pos, MapCanvas* canvas) : FormHelper(parent)
{
	setCanvas(canvas);
	set_fixed_size(Vector2i(100, 20));
	m_window = add_window(pos, "Position panel");

	add_group("Position");
	add_variable<double>("Latitude",
		[this](double value) { if (m_canvas) m_canvas->setLatitude(value); },
		[this]() { return m_canvas ? m_canvas->getLatitude() : 0.0; });
	add_variable<double>("Longitude",
		[this](double value) { if (m_canvas) m_canvas->setLongitude(value); },
		[this]() { return m_canvas ? m_canvas->getLongitude() : 0.0; });
	add_variable<double>("Zoom",
		[this](double value) { if (m_canvas) m_canvas->setZoom(value); },
		[this]() { return m_canvas ? m_canvas->getZoom() : 0.0; });
	add_variable<double>("Rotation",
		[this](double value) { if (m_canvas) m_canvas->setRotation(radians(std::fmod(value, 360.0))); },
		[this]() { return m_canvas ? std::fmod(degrees(m_canvas->getRotation()), 360.0) : 0.0; });
	add_button("Reset View", [this] { if (m_canvas) m_canvas->resetView(); });
	
	add_group("Cursor");
	add_variable<double>("Latitude",
		[this](double value) {},
		[this]() { return m_canvas ? m_canvas->getCursorLatitude() : 0.0; }, false);
	add_variable<double>("Longitude",
		[this](double val) {},
		[this]() { return m_canvas ? m_canvas->getCursorLongitude() : 0.0; }, false);
}

MapCanvas* MapForm::getCanvas() const noexcept
{
	return m_canvas;
}

void MapForm::setCanvas(MapCanvas* canvas) noexcept
{
	m_canvas = canvas;
	if (m_canvas)
	{
		m_canvas->cb_cursor_moved().listen([this](Vector2d) { refresh(); });
		m_canvas->cb_zoom_changed().listen([this](double) { refresh(); });
		m_canvas->cb_rotation_changed().listen([this](double) { refresh(); });
		m_canvas->cb_map_moved().listen([this](Vector2d) { refresh(); });
	}
}

MapInfo::MapInfo(nanogui::Screen* parent, Vector2i pos,
	traffic::World* world, MapCanvas* canvas) : FormHelper(parent)
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
				m_canvas->loadMap(m_world->getMap());
				m_canvas->loadHighwayMap(m_world->getHighwayMap());
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

MapCanvas* MapInfo::getCanvas() const noexcept { return m_canvas; }

void MapInfo::setCanvas(MapCanvas* canvas) { m_canvas = canvas; }

MapDialogPath::MapDialogPath(
	nanogui::Screen* parent, Vector2i pos, World *world, MapCanvas* canvas, MapContextDialog *contextMenu)
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
			m_canvas->getDistance(getStart(), getStop()) : 0.0; }, false);
	add_button("Calculate Path", [this](){
		auto& graph = m_world->getGraph();
		GraphNode& idStart = graph->findClosestNode(Point(m_start_lat, m_start_lon));
		GraphNode& idStop = graph->findClosestNode(Point(m_stop_lat, m_stop_lon));
		std::cout << "Searching route from " << idStart.nodeID << " " << idStop.nodeID << "\n";
		Route r = graph->findRoute(idStart.nodeID, idStop.nodeID);
		for (int64_t id : r.nodes) {
			std::cout << "Node: " << id << "\n";
		}
		m_canvas->loadRoute(r, graph->getXMLMap());

	});

	if (k_context)
	{
		k_context->addContextButton("Set Start", [this]() {
			if (m_canvas) {
				Vector2d pos = m_canvas->windowToPosition(k_context->position());
				setStart(pos);
				m_canvas->request_focus(); // moves focus back to canvas
			}
		});
		k_context->addContextButton("Set End", [this]() {
			if (m_canvas) {
				Vector2d pos = m_canvas->windowToPosition(k_context->position());
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


MapContextDialog::MapContextDialog(nanogui::Widget* parent, MapCanvas *canvas)
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
		k_canvas->cb_leftclick().listen([this](Vector2d) {
			closeListener().trigger();
			set_visible(false);
		});
		k_canvas->cb_rightclick().listen([this](Vector2d pos) {
			openListener().trigger();

			Vector2d p1 = k_canvas->planeToView(k_canvas->getCursorPlane());
			Vector2i p2 = k_canvas->viewToWindow(p1);
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
