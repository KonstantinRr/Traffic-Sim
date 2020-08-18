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
	std::shared_ptr<OSMSegment> world, MapForm* form) : Canvas(parent, 1)
{
	m_active = false;
	m_render_chunk = false;
	m_mark_update = false;
	m_update_view = true;

	m_form = form;

	m_min_zoom = 2.0;
	m_max_zoom = 1000.0;

	if (world) loadMap(world);
	else resetView();

	try {
		// Defines the used shaders
		
		m_chunk_shader = new Shader(
			render_pass(), "shader_chunk",
			getChunkVertex(), getChunkFragment());
		m_shader = new Shader(
			render_pass(), "shader_map",
			getLineVertex(), getLineFragment());
		m_success = true;
	} catch ( ... ) {
		//printf("%s\n", e.what());
		m_success = false;
	}
}

Vector2d MapCanvas::scaleWindowDistance(Vector2i vec) {
	return Vector2d(
		vec.x() * 2.0 / width(),
		-vec.y() * 2.0 / width());
}

Vector2d MapCanvas::transformWindow(Vector2i vec) {
	return Vector2d(
		vec.x() * 2.0 / width() - 1.0,
		(height() - vec.y()) * 2.0 / height() - 1.0);
}

void MapCanvas::applyTranslation(Vector2d rel)
{
	setPosition(position - toView(glm::rotate(toGLM(rel / m_zoom), -m_rotation)));
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
	refreshView();
}

void MapCanvas::setLatitude(double lat)
{
	position.x() = latitudeToPlane(lat, toGLM(getCenter()));
	refreshView();
}

void MapCanvas::setLongitude(double lon)
{
	position.y() = longitudeToPlane(lon, toGLM(getCenter()));
	refreshView();
}

void MapCanvas::setLatLon(double lat, double lon)
{
	setLatitude(lat);
	setLongitude(lon);
	refreshView();
}

void MapCanvas::setPosition(Vector2d pos)
{
	position = pos;
	refreshView();
}

void MapCanvas::setZoom(double zoom)
{
	m_zoom = zoom;
	refreshView();
}

void MapCanvas::setRotation(double rotation)
{
	m_rotation = rotation;
	refreshView();
}

double MapCanvas::getLatitude() const { return planeToLatitude(position.x(), toGLM(getCenter())); }
double MapCanvas::getLongitude() const { return planeToLongitude(position.y(), toGLM(getCenter())); }
double MapCanvas::getCursorLatitude() const { return planeToLatitude(cursor.x(), toGLM(getCenter())); }
double MapCanvas::getCursorLongitude() const { return planeToLongitude(cursor.y(), toGLM(getCenter())); }

double MapCanvas::getZoom() const { return m_zoom; }
double MapCanvas::getRotation() const { return m_rotation; }
double MapCanvas::getMinZoom() const { return m_min_zoom; }
double MapCanvas::getMaxZoom() const { return m_max_zoom; }

Vector2d MapCanvas::getCenter() const
{
	if (m_map) {
		return toView(m_map->getBoundingBox().getCenter().toVec());
	}
	else {
		return Vector2d(0.0, 0.0);
	}
}

void MapCanvas::refreshView()
{
	m_mark_update = true;
}

void MapCanvas::loadMap(std::shared_ptr<traffic::OSMSegment> map)
{
	if (map) {
		m_map = map;
		genMesh();
		resetView();
	}
}

bool MapCanvas::hasMap() const { return m_map.get(); }
MapForm* MapCanvas::getForm() const { return m_form; }
void MapCanvas::setForm(MapForm* form) { m_form = form; }

bool MapCanvas::mouse_button_event(
	const Vector2i& p, int button, bool down, int modifiers) {
	Canvas::mouse_button_event(p, button, down, modifiers);
	Vector2d position = inverseTransform(transformWindow(p));
	if (button == GLFW_MOUSE_BUTTON_1) {
		//for (auto & cb : m_cb_leftclick)
		//	cb.function(position);
		return true;
	}
	else if (button == GLFW_MOUSE_BUTTON_2) {
		//for (auto & cb : m_cb_rightclick)
		//	cb.function(position);
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
	cursor = inverseTransform(transformWindow(p));
	refreshView();
	return true;
}

bool MapCanvas::scroll_event(const Vector2i& p, const Vector2f& rel)
{
	Canvas::scroll_event(p, rel);
	setZoom(std::clamp(m_zoom * pow(0.94, -rel.y()), 2.0, 1000.0));
	return true;
}

void MapCanvas::setMesh(const std::vector<glm::vec3>& colors,
	const std::vector<glm::vec2>& points)
{
	if (m_success) {
		pointsSize = points.size();
		m_shader->set_buffer("vVertex", nanogui::VariableType::Float32,
			{ points.size(), 2 }, points.data());
		m_shader->set_buffer("color", nanogui::VariableType::Float32,
			{ colors.size(), 3 }, colors.data());
	}
}

void MapCanvas::setChunkMesh(const std::vector<glm::vec2> &points)
{
	if (m_success) {
		chunksSize = points.size();
		m_chunk_shader->set_buffer("vVertex", nanogui::VariableType::Float32,
			{ points.size(), 2 }, points.data());
	}
}

void MapCanvas::setActive(bool active)
{
	m_active = true;
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
	if (m_mark_update && m_update_view) {
		if (m_form) m_form->refresh();
		m_mark_update = false;
	}
}

void MapCanvas::clearCallbacks()
{
	clearCallbacksLeftClick();
	clearCallbacksRightClick();
}

void MapCanvas::clearCallbacksLeftClick() { m_cb_leftclick.clear(); }
void MapCanvas::clearCallbacksRightClick() { m_cb_rightclick.clear(); }

void MapCanvas::genMesh()
{
	if (m_success && m_map) {
		std::vector<vec2> points = generateMesh(*m_map);
		std::vector<vec3> colors(points.size(), glm::vec3(1.0f, 1.0f, 1.0f));
		setMesh(colors, points);
	}
}

void MapCanvas::clearMesh()
{
	pointsSize = 0;
	chunksSize = 0;
}

Vector2d MapCanvas::forwardTransform(const Vector2d& pos) const
{
	glm::dvec2 f = toGLM(pos - position);
	f = glm::rotate(f, m_rotation);
	f = f * dvec2(m_zoom, m_zoom * width() / height());
	return toView(f);
}

Vector2d MapCanvas::inverseTransform(const Vector2d& pos) const
{
	glm::dvec2 f = toGLM(pos) / dvec2(m_zoom, m_zoom * width() / height());
	f = glm::rotate(f, -m_rotation);
	f = f + toGLM(position);
	return toView(f);
}

Matrix3f MapCanvas::createTransform3D() const
{
	glm::mat3 matrix(1.0f);
	glm::translate(matrix, vec2(toGLM(-position)));
	glm::rotate(matrix, (float)m_rotation);
	glm::scale(matrix, glm::vec2(m_zoom, m_zoom * width() / height()));
	return toView(matrix);
}

Matrix4f MapCanvas::createTransform4D() const {
	Matrix4f translate = Matrix4f::translate(
		Vector3f(-position.x(), -position.y(), 0.0f));
	Matrix4f rotation = Matrix4f::rotate(Vector3f(0.0f, 0.0f, 1.0f), m_rotation);
	Matrix4f scale = Matrix4f::scale(
		Vector3f(m_zoom, m_zoom * width() / height(), 1.0f));
	return scale * rotation * translate;
}

void MapCanvas::draw_contents()
{
	using namespace nanogui;
	if (m_active && m_success && hasMap()) {
		auto transform = createTransform4D();
	
		// Chunk rendering
		if (m_render_chunk)
		{
			m_chunk_shader->set_uniform("mvp", transform);
			m_chunk_shader->set_uniform("color", Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
			m_chunk_shader->begin();
			m_shader->draw_array(Shader::PrimitiveType::Line, 0, chunksSize, false);
			m_chunk_shader->end();
		}

		// renders the 
		m_shader->set_uniform("mvp", transform);
		m_shader->begin();
		m_shader->draw_array(Shader::PrimitiveType::Line, 0, pointsSize, false);
		m_shader->end();
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
	m_canvas = canvas;
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

MapDialogPath::MapDialogPath(nanogui::Screen* parent, Vector2i pos, MapCanvas* canvas)
	: nanogui::FormHelper(parent)
{
	clear();

	set_fixed_size(Vector2i(100, 20));
	m_window = add_window(pos, "Position panel");
	m_canvas = canvas;

	add_group("Start");
	add_variable<double>("Latitude", m_start.x());
	add_variable<double>("Longitude", m_start.y());
	add_group("End");
	add_variable<double>("Latitude", m_stop.x());
	add_variable<double>("Longitude", m_stop.y());
	add_button("Calculate Path", [this](){});
}

void MapDialogPath::clear()
{
	m_start = { 0.0, 0.0 };
	m_stop = { 0.0, 0.0 };
}


MapDetail::MapDetail(nanogui::Widget* parent) : nanogui::Widget(parent)
{
	m_box = new nanogui::TextBox(this, "Hello World");
	add_child(m_box);
}
