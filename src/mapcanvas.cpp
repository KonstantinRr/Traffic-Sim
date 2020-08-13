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

MapCanvas::MapCanvas(Widget* parent, std::shared_ptr<OSMSegment> map, MapForm* form) : Canvas(parent, 1) {
	using namespace nanogui;
	this->map = map;
	m_active = false;
	m_form = form;
	m_render_chunk = false;

	resetView();

	// Defines the used shaders
	m_chunk_shader = new Shader(
		render_pass(), "shader_chunk",
		getChunkVertex(), getChunkFragment()
	);
	m_shader = new Shader(
		render_pass(), "shader_map",
		getLineVertex(), getLineFragment()
	);
}

Vector2d MapCanvas::scaleWindowDistance(Vector2i vec) {
	return Vector2d(
		vec.x() * 2.0 / width(),
		-vec.y() * 2.0 / width()
	);
}

Vector2d MapCanvas::transformWindow(Vector2i vec)
{
	return Vector2d(
		vec.x() * 2.0 / width() - 1.0,
		(height() - vec.y()) * 2.0 / height() - 1.0
	);
}

void MapCanvas::applyTranslation(Vector2d rel)
{
	setPosition(position - toView(glm::rotate(toGLM(rel / m_zoom), -m_rotation)));
}

void MapCanvas::applyZoom(double iterations)
{
	setZoom(std::clamp(m_zoom * pow(0.99, iterations), 2.0, 1000.0));
}

void MapCanvas::applyRotation(double radians)
{
	setRotation(m_rotation + radians);
}

void MapCanvas::resetView()
{
	auto centerPoint = sphereToPlane(
		map->getBoundingBox().getCenter().toVec());
	position = { (float)centerPoint.x, (float)centerPoint.y };
	m_zoom = 25.0f;
	m_rotation = 0.0f;
	refreshView();
}

void MapCanvas::setLatitude(double lat)
{
	position.x() = latitudeToPlane(lat, map->getBoundingBox().getCenter().toVec());
	refreshView();
}

void MapCanvas::setLongitude(double lon)
{
	position.y() = longitudeToPlane(lon, map->getBoundingBox().getCenter().toVec());
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

double MapCanvas::getLatitude() const
{
	return planeToLatitude(position.x(), map->getBoundingBox().getCenter().toVec());
}

double MapCanvas::getLongitude() const
{
	return planeToLongitude(position.y(), map->getBoundingBox().getCenter().toVec());
}

double MapCanvas::getCursorLatitude() const
{
	return planeToLatitude(cursor.x(), map->getBoundingBox().getCenter().toVec());
}

double MapCanvas::getCursorLongitude() const
{
	return planeToLongitude(cursor.y(), map->getBoundingBox().getCenter().toVec());
}

double MapCanvas::getZoom() const
{
	return m_zoom;
}

double MapCanvas::getRotation() const
{
	return m_rotation;
}

void MapCanvas::refreshView()
{
	m_mark_update = true;
}

MapForm* MapCanvas::getForm() const
{
	return m_form;
}

void MapCanvas::setForm(MapForm* form)
{
	m_form = form;
}

bool MapCanvas::mouse_button_event(
	const Vector2i& p, int button, bool down, int modifiers) {
	return Canvas::mouse_button_event(p, button, down, modifiers);
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

void MapCanvas::setData(std::shared_ptr<std::vector<glm::vec3>> colors, std::shared_ptr<std::vector<glm::vec2>> points)
{
	using namespace nanogui;
	pointsSize = points->size();
	m_shader->set_buffer("vVertex", VariableType::Float32,
		{ points->size(), 2 }, points->data());
	m_shader->set_buffer("color", VariableType::Float32,
		{ colors->size(), 3 }, colors->data());
}

void MapCanvas::setChunkData(std::shared_ptr<std::vector<glm::vec2>> points)
{
	using namespace nanogui;
	chunksSize = points->size();
	m_chunk_shader->set_buffer("vVertex", VariableType::Float32,
		{ points->size(), 2 }, points->data());
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
	if (m_active) {
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

MapForm::MapForm(nanogui::Screen* parent, Vector2i pos, MapCanvas* canvas) : FormHelper(parent)
{
	m_canvas = canvas;
	set_fixed_size(Vector2i(100, 20));
	m_window = add_window(pos, "Position panel");
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

MapInfo::MapInfo(nanogui::Screen* parent, Vector2i pos, traffic::World* world) : FormHelper(parent)
{
	set_fixed_size(Vector2i(100, 20));
	m_window = add_window(pos, "Position panel");
	m_world = world;

	bool addBounds = true;
	bool addGeneralInfo = true;

	if (addBounds) {
		add_group("Bounds Info");
		add_variable<double>("Lat+",
			[this](double val) { },
			[this]() { return m_world ? m_world->getMap()->getBoundingBox().upperLatBorder() : 0; }, false);
		add_variable<double>("Lat-",
			[this](double val) {},
			[this]() { return m_world ? m_world->getMap()->getBoundingBox().lowerLatBorder() : 0; }, false);
		add_variable<double>("Lon+",
			[this](double val) {},
			[this]() { return m_world ? m_world->getMap()->getBoundingBox().upperLonBorder() : 0; }, false);
		add_variable<double>("Lon-",
			[this](double val) {},
			[this]() { return m_world ? m_world->getMap()->getBoundingBox().lowerLonBorder() : 0; }, false);
	}

	if (addGeneralInfo) {
		add_group("Memory Info");
		add_variable<size_t>("Nodes",
			[this](size_t) { },
			[this]() { return m_world ? m_world->getMap()->getNodeCount() : 0; }, false);
		add_variable<size_t>("Ways",
			[this](size_t) {},
			[this]() { return m_world ? m_world->getMap()->getWayCount() : 0; }, false);
		add_variable<size_t>("Relations",
			[this](size_t) {},
			[this]() { return m_world ? m_world->getMap()->getRelationCount() : 0; }, false);
	}

	add_button("Choose File", [this]() {
		using namespace std;
		vector<pair<string, string>> vect{
			make_pair<string, string>("xmlmap", "OSM File format"),
			make_pair<string, string>("osm", "OSM File format"),
		};
		nanogui::file_dialog(vect, true);
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
