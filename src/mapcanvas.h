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

#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanogui/button.h>
#include <nanogui/canvas.h>
#include <nanogui/shader.h>
#include <nanogui/renderpass.h>
#include <nanogui/formhelper.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "traffic/agent.h"
#include "traffic/osm.h"

using nanogui::Vector2i;
using nanogui::Shader;
using nanogui::Canvas;
using nanogui::ref;

// ---- Floating point types ---- //
using nanogui::Vector4f;
using nanogui::Vector3f;
using nanogui::Vector2f;

using nanogui::Matrix2f;
using nanogui::Matrix3f;
using nanogui::Matrix4f;
// ---- Double types ---- //
using Vector2d = nanogui::Array<double, 2>;
using Vector3d = nanogui::Array<double, 3>;
using Vector4d = nanogui::Array<double, 4>;

class MapForm;
class MapInfo;
class MapCanvas;

// ---- View to GLM transform ---- //

inline glm::mat4 toGLM(const Matrix4f &value) { return glm::make_mat4((float*)value.m); }
inline glm::mat3 toGLM(const Matrix3f &value) { return glm::make_mat3((float*)value.m); }
inline glm::mat2 toGLM(const Matrix2f &value) { return glm::make_mat2((float*)value.m); }

inline glm::vec4 toGLM(const Vector4f &value) { return glm::make_vec4<float>(value.v); }
inline glm::vec3 toGLM(const Vector3f &value) { return glm::make_vec3<float>(value.v); }
inline glm::vec2 toGLM(const Vector2f &value) { return glm::make_vec2<float>(value.v); }
inline glm::dvec4 toGLM(const Vector4d& value) { return glm::make_vec4<double>(value.v); }
inline glm::dvec3 toGLM(const Vector3d& value) { return glm::make_vec3<double>(value.v); }
inline glm::dvec2 toGLM(const Vector2d& value) { return glm::make_vec2<double>(value.v); }

// ---- GLM to View transform ---- //

Matrix4f toView(const glm::mat4& value);
Matrix3f toView(const glm::mat3& value);
Matrix2f toView(const glm::mat2& value);

inline Vector4f toView(const glm::vec4& value) { return Vector4f(value.x, value.y, value.z, value.w); }
inline Vector3f toView(const glm::vec3& value) { return Vector3f(value.x, value.y, value.z); }
inline Vector2f toView(const glm::vec2& value) { return Vector2f(value.x, value.y); }

inline Vector4d toView(const glm::dvec4& value) { return Vector4d(value.x, value.y, value.z, value.w); }
inline Vector3d toView(const glm::dvec3& value) { return Vector3d(value.x, value.y, value.z); }
inline Vector2d toView(const glm::dvec2& value) { return Vector2d(value.x, value.y); }

/// <summary>
/// A window that gives information about the current latitude and longitude
/// position of the map and the cursor. It can be used to manipulate the map
/// settings. This includes the zoom and rotation of the map.
/// </summary>
class MapForm : public nanogui::FormHelper {
public:
	MapForm(nanogui::Screen* parent, Vector2i pos, MapCanvas* canvas = nullptr);

	MapCanvas* getCanvas() const noexcept;
	void setCanvas(MapCanvas* canvas) noexcept;

protected:
	MapCanvas* m_canvas = nullptr;
	nanogui::ref<nanogui::Window> m_window = nullptr;
};

/// <summary>
/// A window that gives information about the currently loaded map. This includes
/// memory statistics about the currently loaded map and the option to load a new
/// map into memory.
/// </summary>
class MapInfo : public nanogui::FormHelper {
public:
	MapInfo(nanogui::Screen* parent, Vector2i pos,
		traffic::World* world = nullptr,
		MapCanvas *canvas = nullptr);

	traffic::World* getWorld() const noexcept;
	void setWorld(traffic::World *world) noexcept;

	MapCanvas* getCanvas() const noexcept;
	void setCanvas(MapCanvas *canvas);

protected:
	traffic::World* m_world = nullptr;
	MapCanvas* m_canvas = nullptr;
	nanogui::ref<nanogui::Window> m_window = nullptr;
};

/// <summary>
/// A canvas that is used to render a map to the screen. This canvas uses its own
/// OpenGL shaders to render a mesh of the map dynamically on the screen. It offers
/// some functions to manipulate the view matrix (zoom, rotation, translation).
/// </summary>
class MapCanvas : public nanogui::Canvas {
public:
	MapCanvas(Widget* parent, std::shared_ptr<traffic::OSMSegment> map,
		MapForm* form = nullptr);

	// ---- Position updates ---- //
	void setLatitude(double lat);
	void setLongitude(double lon);
	void setLatLon(double lat, double lon);
	void setPosition(Vector2d pos);
	void setZoom(double zoom);
	void setRotation(double rotation);
	
	// ---- Positional reads ---- //
	double getLatitude() const;
	double getLongitude() const;
	double getCursorLatitude() const;
	double getCursorLongitude() const;
	double getZoom() const;
	double getRotation() const;
	double getMinZoom() const;
	double getMaxZoom() const;

	Vector2d getCenter() const;

	void refreshView();
	void loadMap(std::shared_ptr<traffic::OSMSegment> map);

	bool hasMap() const;

	MapForm* getForm() const;
	void setForm(MapForm* form);

	// ---- Events ---- //

	virtual bool mouse_button_event(
		const Vector2i& p, int button, bool down, int modifiers) override;
	virtual bool mouse_drag_event(
		const Vector2i& p, const Vector2i& rel, int button, int modifiers) override;
	virtual bool mouse_motion_event(
		const Vector2i& p, const Vector2i& rel, int button, int modifiers) override;

	virtual bool scroll_event(
		const Vector2i& p, const Vector2f& rel) override;

	virtual bool keyboard_event(
		int key, int scancode, int action, int modifiers) override;

	// ---- Ticks ---- //
	virtual void draw_contents() override;

	void updateKeys(double dt);
	void update(double dt);

	Vector2d scaleWindowDistance(Vector2i vec);
	Vector2d transformWindow(Vector2i vec);

	void applyTranslation(Vector2d rel);
	void applyZoom(double iterations);
	void applyRotation(double radians);
	void resetView();

	Vector2d forwardTransform(const Vector2d &pos) const;
	Vector2d inverseTransform(const Vector2d &pos) const;
	Matrix3f createTransform3D() const;
	Matrix4f createTransform4D() const;

	void setActive(bool active);

protected:
	// ---- Mesh access ---- //
	void genMesh();
	void clearMesh();

	void setMesh(
		const std::vector<glm::vec3>& colors,
		const std::vector<glm::vec2>& points);

	void setChunkMesh(
		const std::vector<glm::vec2>& points);

	// ---- Member variables ---- //
	nanogui::ref<nanogui::Shader> m_shader;
	nanogui::ref<nanogui::Shader> m_chunk_shader;
	MapForm* m_form;

	std::shared_ptr<traffic::OSMSegment> m_map;
	size_t pointsSize, chunksSize;
	bool m_active;
	bool m_success;
	bool m_render_chunk;
	bool m_mark_update;
	bool m_update_view;

	Vector2d position;
	Vector2d cursor;
	double m_zoom;
	double m_rotation;

	double m_max_zoom;
	double m_min_zoom;

};