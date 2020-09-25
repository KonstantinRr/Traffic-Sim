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

#pragma once

#ifndef MAPCANVAS_HPP
#define MAPCANVAS_HPP

#include "traffic/engine.h"
#include "traffic/agent.h"
#include "traffic/osm.h"

#include "engine/shader.hpp"
#include "engine/window.hpp"

#include "listener.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


/// <summary>
/// A canvas that is used to render a map to the screen. This canvas uses its own
/// OpenGL shaders to render a mesh of the map dynamically on the screen. It offers
/// some functions to manipulate the view matrix (zoom, rotation, translation).
/// </summary>
class MapCanvas : public lt::render::Renderable {
public:
	MapCanvas(std::shared_ptr<traffic::OSMSegment> map,
		lt::SizedObject *size);

	lt::render::RenderPipeline& getPipeline();

	virtual void render();

	// ---- Position updates ---- //
	void setLatitude(double lat);
	void setLongitude(double lon);
	void setLatLon(double lat, double lon);
	void setPosition(glm::dvec2 pos);
	void setZoom(double zoom);
	void setRotation(double rotation);

	// ---- Positional reads ---- //
	double getLatitude() const;
	double getLongitude() const;
	double getCursorLatitude() const;
	double getCursorLongitude() const;
	glm::dvec2 getPosition() const;
	glm::dvec2 getCursor() const;
	glm::dvec2 getPositionPlane() const;
	glm::dvec2 getCursorPlane() const;

	double getDistance(glm::dvec2 p1, glm::dvec2 p2) const;

	double getZoom() const;
	double getRotation() const;
	double getMinZoom() const;
	double getMaxZoom() const;

	glm::dvec2 getCenter() const;

	void loadMap(std::shared_ptr<traffic::OSMSegment> map);
	void loadHighwayMap(std::shared_ptr<traffic::OSMSegment> map);
	void loadRoute(const traffic::Route &route, std::shared_ptr<traffic::OSMSegment> map);
	void clearRoutes();

	bool hasMap() const;

	// ---- Events ---- //

	glm::dvec2 scaleWindowDistance(glm::ivec2 vec);

	void applyTranslation(glm::dvec2 rel);
	void applyZoom(double iterations);
	void applyRotation(double radians);
	void resetView();

	// ---- Transformations ---- //

	// -- Window <-> View -- //
	glm::dvec2 windowToView(glm::ivec2 vec) const;
	glm::ivec2 viewToWindow(glm::dvec2 vec) const;
	// -- View <-> Plane -- //
	glm::dvec2 viewToPlane(const glm::dvec2 &pos) const;
	glm::dvec2 planeToView(const glm::dvec2 &pos) const;
	// -- Plane <-> Position -- //
	glm::dvec2 planeToPosition(const glm::dvec2 &pos) const;
	glm::dvec2 positionToPlane(const glm::dvec2 &pos) const;

	glm::dvec2 windowToPosition(glm::ivec2 vec) const;
	glm::ivec2 positionToWindow(glm::dvec2 vec) const;

	//Matrix3f transformPlaneToView3D() const;
	//Matrix4f transformPlaneToView4D() const;

	glm::mat3 transformPlaneToView3DGLM() const;
	glm::mat4 transformPlaneToView4DGLM() const;
	void setActive(bool active);
	
	// ---- Callbacks ---- //
	Listener<void(glm::dvec2)>& cb_leftclick() { return m_cb_leftclick; }
	Listener<void(glm::dvec2)>& cb_rightclick() { return m_cb_rightclick; }
	Listener<void(glm::dvec2)>& cb_map_moved() { return m_cb_map_moved; }
	Listener<void(glm::dvec2)>& cb_cursor_moved() { return m_cb_cursor_moved; }
	Listener<void(traffic::Rect)>& cb_view_changed() { return m_cb_view_changed; }
	Listener<void(double)>& cb_zoom_changed() { return m_cb_zoom_changed; }
	Listener<void(double)>& cb_rotation_changed() { return m_cb_rotation_changed; }

protected:
	// ---- Mesh access ---- //
	void clearMesh();

	std::shared_ptr<lt::Transformed4DEntity2D> genMeshFromMap(
		const traffic::OSMSegment &seg, glm::vec3 color);
	std::shared_ptr<lt::Transformed4DEntity2D> genMesh(
		std::vector<glm::vec2> &&points, std::vector<glm::vec3> &&colors);

	void setChunkMesh(
		const std::vector<glm::vec2>& points);

	// ---- Callbacks ---- //
	template<typename Type, typename CBType>
	CallbackReturn<CBType> addCallback(const Type& function, std::vector<CallbackForm<CBType>> &callbacks) {
		int32_t id = callbacks.empty() ? 0 : callbacks.back().id + 1;
		callbacks.push_back(
			CallbackForm<CBType>(
				id, std::function<CBType>(function)
			)
		);
		return CallbackReturn(id, &callbacks);
	}

	// ---- Member variables ---- //

	Listener<void(glm::dvec2)> m_cb_leftclick;
	Listener<void(glm::dvec2)> m_cb_rightclick;
	Listener<void(glm::dvec2)> m_cb_map_moved;
	Listener<void(glm::dvec2)> m_cb_cursor_moved;
	Listener<void(traffic::Rect)> m_cb_view_changed;
	Listener<void(double)> m_cb_zoom_changed;
	Listener<void(double)> m_cb_rotation_changed;


	std::shared_ptr<lt::Transformed4DEntity2D> l_mesh_map, l_mesh_highway;
	std::vector<std::shared_ptr<lt::Transformed4DEntity2D>> l_mesh_routes;

	std::shared_ptr<lt::render::LineMemoryShader> l_shader;
	std::shared_ptr<lt::render::RenderList<lt::Entity2D>> entities;
	std::shared_ptr<lt::render::RenderComponent<
		lt::render::LineStageBuffer,
		lt::render::LineMemoryShader>> l_comp;
	lt::render::RenderPipeline l_pipeline;
	lt::SizedObject *l_size;

	std::shared_ptr<traffic::OSMSegment> m_map;
	std::shared_ptr<traffic::OSMSegment> m_highway_map;

	bool m_active;
	bool m_success;
	bool m_render_chunk;
	bool m_mark_update;
	bool m_update_view;

	glm::dvec2 position;
	glm::dvec2 cursor;
	double m_zoom;
	double m_rotation;

	double m_max_zoom;
	double m_min_zoom;
};

#endif
