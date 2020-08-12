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

#include "traffic/osm_mesh.h"

using namespace traffic;

MapCanvas::MapCanvas(Widget* parent, std::shared_ptr<OSMSegment> map, MapForm* form) : Canvas(parent, 1) {
	using namespace nanogui;
	this->map = map;
	m_active = false;
	m_form = form;
	m_render_chunk = false;

	auto centerPoint = sphereToPlane(
		map->getBoundingBox().getCenter().toVec());
	position = { (float)-centerPoint.x, (float)-centerPoint.y };
	m_zoom = 25.0f;
	refreshView();

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
	setPosition(position + scaleWindowDistance(rel) / m_zoom);
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
	double v = 0.0;
	double arrowSpeed = dt * 0.8;

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) v += dt;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) v -= dt;

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		setPosition(position + Vector2d(arrowSpeed, 0.0) / m_zoom);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		setPosition(position + Vector2d(-arrowSpeed, 0.0) / m_zoom);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		setPosition(position + Vector2d(0.0, -arrowSpeed) / m_zoom);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		setPosition(position + Vector2d(0.0, arrowSpeed) / m_zoom);

	setZoom(std::clamp(m_zoom * pow(0.95, v), 2.0, 1000.0));
}

void MapCanvas::update(double dt)
{
	updateKeys(dt);
	if (m_mark_update) {
		if (m_form) m_form->refresh();
		m_mark_update = false;
	}
}

Vector2d MapCanvas::forwardTransform(const Vector2d& pos) const
{
	Vector2d f = pos + position;
	f = f * Vector2d(m_zoom, m_zoom * width() / height());
	return f;
}

Vector2d MapCanvas::inverseTransform(const Vector2d& pos) const
{
	Vector2d f = pos / Vector2d(m_zoom, m_zoom * width() / height());
	f = f - position;
	return f;
}

Matrix4f MapCanvas::createTransform() const
{
	Matrix4f matrix = Matrix4f::translate(
		Vector3f(position.x(), position.y(), 0.0f));
	Matrix4f scale = Matrix4f::scale(
		Vector3f(m_zoom, m_zoom * width() / height(), 1.0f));
	return scale * matrix;
}

void MapCanvas::draw_contents()
{
	using namespace nanogui;
	if (m_active) {
		Matrix4f transform = createTransform();
	
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
/*
nanogui::Vector4f toView(const glm::vec4& value)
{
	return nanogui::Vector4f(value.x, value.y, value.z, value.w);
}

nanogui::Vector3f toView(const glm::vec3& value)
{
	return nanogui::Vector3f(value.x, value.y, value.z);
}

nanogui::Vector2f toView(const glm::vec2& value)
{
	return nanogui::Vector2f(value.x, value.y);
}
*/
