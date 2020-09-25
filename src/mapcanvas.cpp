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

#include "traffic/engine.h"
#include "traffic/osm_mesh.h"

#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace traffic;
using namespace glm;

// ---- MapCanvas ---- //

MapCanvas::MapCanvas(std::shared_ptr<OSMSegment> world,
	lt::SizedObject *size)
{
	l_size = size;
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
	using namespace lt::render;
	try {
		l_shader = std::make_shared<LineMemoryShader>();
		entities = std::make_shared<RenderList<lt::Entity2D>>();
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

lt::render::RenderPipeline& MapCanvas::getPipeline() {
	return l_pipeline;
}

glm::dvec2 MapCanvas::scaleWindowDistance(glm::ivec2 vec) {
	return glm::dvec2(
		vec.x * 2.0 / l_size->width(), // TODO
		-vec.y * 2.0 / l_size->width()); // TODO
}

void MapCanvas::applyTranslation(glm::dvec2 rel)
{
	glm::dvec2 mx = glm::rotate(rel / m_zoom, -m_rotation);
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
	auto centerPoint = sphereToPlane(getCenter());
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
	setPosition(glm::dvec2(
		latitudeToPlane(lat, getCenter()),
		position.y));
}

void MapCanvas::setLongitude(double lon)
{
	setPosition(glm::dvec2(
		position.x,
		longitudeToPlane(lon, getCenter())));
}

void MapCanvas::setLatLon(double lat, double lon)
{
	setPosition(glm::dvec2(
		latitudeToPlane(lat, getCenter()),
		longitudeToPlane(lon, getCenter())));
}

void MapCanvas::setPosition(glm::dvec2 pos)
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

double MapCanvas::getLatitude() const { return planeToLatitude(position.x, getCenter()); }
double MapCanvas::getLongitude() const { return planeToLongitude(position.y, getCenter()); }
double MapCanvas::getCursorLatitude() const { return planeToLatitude(cursor.x, getCenter()); }
double MapCanvas::getCursorLongitude() const { return planeToLongitude(cursor.y, getCenter()); }
glm::dvec2 MapCanvas::getCursor() const { return glm::dvec2(getCursorLatitude(), getCursorLongitude()); }
glm::dvec2 MapCanvas::getPosition() const { return glm::dvec2(getLatitude(), getLongitude()); }
glm::dvec2 MapCanvas::getPositionPlane() const { return position;}
glm::dvec2 MapCanvas::getCursorPlane() const { return cursor; }

double MapCanvas::getZoom() const { return m_zoom; }
double MapCanvas::getRotation() const { return m_rotation; }
double MapCanvas::getMinZoom() const { return m_min_zoom; }
double MapCanvas::getMaxZoom() const { return m_max_zoom; }

double MapCanvas::getDistance(glm::dvec2 p1, glm::dvec2 p2) const {
	return distance(p1, p2);
}

glm::dvec2 MapCanvas::getCenter() const
{
	if (m_map) {
		return m_map->getBoundingBox().getCenter().toVec();
	}
	else {
		return glm::dvec2(0.0, 0.0);
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


/*
void MapCanvas::setChunkMesh(const std::vector<glm::vec2> &points)
{
}
*/

void MapCanvas::setActive(bool active)
{
	m_active = active;
}

// ---- Mesh ---- //

std::shared_ptr<lt::Transformed4DEntity2D> MapCanvas::genMeshFromMap(
	const OSMSegment& seg, glm::vec3 color)
{
	std::vector<vec2> points = generateMesh(seg);
	std::vector<vec3> colors(points.size(), color);
	return genMesh(std::move(points), std::move(colors));
}

std::shared_ptr<lt::Transformed4DEntity2D> MapCanvas::genMesh(
	std::vector<glm::vec2>&& points, std::vector<glm::vec3>&& colors)
{
	lt::resource::MeshBuilder2D builder;
	builder.setVertices(std::move(points));
	builder.setColors(std::move(colors));
	//builder.centerModel();
	//builder.unitize();
	auto exp = builder.exporter()
		.addVertex().addColor()
		.exportData();

	auto model = std::make_shared<lt::GLModel>(exp);
	return std::make_shared<lt::Transformed4DEntity2D>(0, model);
}

void MapCanvas::clearMesh() {
	l_mesh_highway = nullptr;
	l_mesh_map = nullptr;
	l_mesh_routes.clear();
}

glm::dvec2 MapCanvas::windowToView(glm::ivec2 vec) const {
	return glm::dvec2(
		vec.x * 2.0 / l_size->width() - 1.0,
		(l_size->height() - vec.y) * 2.0 / l_size->height() - 1.0);
}

glm::ivec2 MapCanvas::viewToWindow(glm::dvec2 vec) const {
	return glm::ivec2(
		(int32_t)((vec.x + 1.0) / 2.0 * l_size->width()),
		-((int32_t)((vec.y + 1.0) / 2.0 * l_size->height()) - l_size->height())
	);
}

glm::dvec2 MapCanvas::planeToView(const glm::dvec2& pos) const
{
	glm::dvec2 f = pos - position;
	f = glm::rotate(f, m_rotation);
	f = f * dvec2(m_zoom, m_zoom * l_size->width() / l_size->height());
	return f;
}

glm::dvec2 MapCanvas::viewToPlane(const glm::dvec2& pos) const
{
	glm::dvec2 f = pos / dvec2(m_zoom, m_zoom * l_size->width() / l_size->height());
	f = glm::rotate(f, -m_rotation);
	f = f + position;
	return f;
}

glm::dvec2 MapCanvas::planeToPosition(const glm::dvec2& pos) const
{
	return planeToSphere(
		pos, getCenter());
}

glm::dvec2 MapCanvas::positionToPlane(const glm::dvec2& pos) const
{
	return sphereToPlane(
		pos, getCenter());
}

glm::dvec2 MapCanvas::windowToPosition(glm::ivec2 vec) const
{
	dvec2 view = windowToView(vec);
	dvec2 plane = viewToPlane(view);
	dvec2 pos = planeToPosition(plane);
	return pos;
}

glm::ivec2 MapCanvas::positionToWindow(glm::dvec2 vec) const
{
	dvec2 plane = positionToPlane(vec);
	dvec2 view = planeToView(plane);
	ivec2 window = viewToWindow(view);
	return window;
}

/*
Matrix3f MapCanvas::transformPlaneToView3D() const
{
	glm::mat3 matrix(1.0f);
	glm::translate(matrix, vec2(-position));
	glm::rotate(matrix, (float)m_rotation);
	glm::scale(matrix, glm::vec2(m_zoom, m_zoom * l_size->width() / l_size->height()));
	return toView(matrix);
}

Matrix4f MapCanvas::transformPlaneToView4D() const {
	Matrix4f translate = Matrix4f::translate(
		Vector3f(-position.x, -position.y, 0.0f));
	Matrix4f rotation = Matrix4f::rotate(Vector3f(0.0f, 0.0f, 1.0f), m_rotation);
	Matrix4f scale = Matrix4f::scale(
		Vector3f(m_zoom, m_zoom * l_size->width() / l_size->height(), 1.0f));
	return scale * rotation * translate;
}
*/

glm::mat3 MapCanvas::transformPlaneToView3DGLM() const {
	glm::mat3 mat(1.0f);
	mat = glm::scale(mat, glm::vec2(m_zoom, m_zoom * l_size->width() / l_size->height()));
	mat = glm::rotate(mat, (float)m_rotation);
	mat = glm::translate(mat, glm::vec2(-position.x, -position.y));
	return mat;
}

glm::mat4 MapCanvas::transformPlaneToView4DGLM() const {
	glm::mat4 mat(1.0f);
    mat = glm::scale(mat, glm::vec3(m_zoom, m_zoom * l_size->width() / l_size->height(), 1.0f));
	mat = glm::rotate(mat, (float)m_rotation, {0.0F, 0.0F, 1.0F});
	mat = glm::translate(mat, glm::vec3(-position.x, -position.y, 0.0f));
	return mat;
}

void MapCanvas::render()
{
	if (m_active && m_success && hasMap()) {
		auto transform = transformPlaneToView4DGLM();
		// Chunk rendering
		if (m_render_chunk)
		{

		}
		
		
		entities->clear();
		for (const auto& t : l_mesh_routes) {
			t->setTransform4D(transform);
			entities->add(t);
		}
		l_mesh_map->setTransform4D(transform);
		l_mesh_highway->setTransform4D(transform);
		entities->add(l_mesh_highway);
		entities->add(l_mesh_map);
		l_pipeline.render();
	}
}
