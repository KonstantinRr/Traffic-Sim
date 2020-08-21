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

#include "listener.h"

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

class MapContextDialog : public nanogui::Window {
public:
	MapContextDialog(nanogui::Widget *parent, MapCanvas *canvas);

	void addSeparator();
	void addContextButton(const std::string &title, std::function<void()> callback);

	Listener<void()>& openListener();
	Listener<void()>& closeListener();

protected:
	Listener<void()> k_listener_open;
	Listener<void()> k_listener_close;
	MapCanvas *k_canvas;
};

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

class MapDialogPath : public nanogui::FormHelper {
public:
	MapDialogPath(nanogui::Screen *parent, Vector2i pos,
		MapCanvas *canvas, MapContextDialog *contextMenu);

	void clear();
	void setStart(Vector2d start);
	void setStop(Vector2d stop);

protected:
	MapCanvas* m_canvas = nullptr;
	nanogui::ref<nanogui::Window> m_window = nullptr;

	Vector2d m_start;
	Vector2d m_stop;
};


/// <summary>
/// A canvas that is used to render a map to the screen. This canvas uses its own
/// OpenGL shaders to render a mesh of the map dynamically on the screen. It offers
/// some functions to manipulate the view matrix (zoom, rotation, translation).
/// </summary>
class MapCanvas : public nanogui::Canvas {
public:
	MapCanvas(Widget* parent, std::shared_ptr<traffic::OSMSegment> map);

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
	Vector2d getPosition() const;
	Vector2d getCursor() const;
	Vector2d getPositionPlane() const;
	Vector2d getCursorPlane() const;

	double getZoom() const;
	double getRotation() const;
	double getMinZoom() const;
	double getMaxZoom() const;

	Vector2d getCenter() const;

	void loadMap(std::shared_ptr<traffic::OSMSegment> map);

	bool hasMap() const;

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

	void applyTranslation(Vector2d rel);
	void applyZoom(double iterations);
	void applyRotation(double radians);
	void resetView();

	// ---- Transformations ---- //

	// -- Window <-> View -- //
	Vector2d windowToView(Vector2i vec);
	Vector2i viewToWindow(Vector2d vec);
	// -- View <-> Plane -- //
	Vector2d viewToPlane(const Vector2d &pos) const;
	Vector2d planeToView(const Vector2d &pos) const;
	// -- Plane <-> Position -- //
	Vector2d planeToPosition(const Vector2d &pos) const;
	Vector2d positionToPlane(const Vector2d &pos) const;


	Matrix3f transformPlaneToView3D() const;
	Matrix4f transformPlaneToView4D() const;

	void setActive(bool active);
	
	// ---- Callbacks ---- //
	template<typename Type> CallbackReturn<void(Vector2d)>
	addCallbackLeftClick(const Type& function) { return addCallback(function, m_cb_leftclick); }

	template<typename Type> CallbackReturn<void(Vector2d)>
	addCallbackRightClick(const Type& function) { return addCallback(function, m_cb_rightclick); }

	template<typename Type> CallbackReturn<void(Vector2d)>
	addCallbackMapMoved(const Type& function) { return addCallback<Type, void(Vector2d)>(function, m_cb_map_moved); }

	template<typename Type> CallbackReturn<void(Vector2d)>
	addCallbackCursorMoved(const Type& function) { return addCallback(function, m_cb_cursor_moved); }

	template<typename Type> CallbackReturn<void(double)>
	addCallbackZoomChanged(const Type& function) { return addCallback(function, m_cb_zoom_changed); }

	template<typename Type> CallbackReturn<void(double)>
	addCallbackRotationChanged(const Type& function) { return addCallback(function, m_cb_rotation_changed); }

	void triggerCallbackClickLeft() const;
	void triggerCallbackClickRight() const;
	void triggerCallbackMapMoved() const;
	void triggerCallbackCursorMoved() const;
	void triggerCallbackZoomChanged() const;
	void triggerCallbackRotationChanged() const;
	void triggerCallbackViewChanged() const;

	void clearCallbacks();
	void clearCallbacksLeftClick();
	void clearCallbacksRightClick();
	void clearCallbacksMapMoved();
	void clearCallbacksCursorMoved();
	void clearCallbacksZoomChanged();
	void clearCallbacksRotationChanged();
	void clearCallbacksViewChanged();
protected:
	// ---- Mesh access ---- //
	void genMesh();
	void clearMesh();

	void setMesh(
		const std::vector<glm::vec3>& colors,
		const std::vector<glm::vec2>& points);

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

	std::vector<CallbackForm<void(Vector2d)>> m_cb_leftclick;
	std::vector<CallbackForm<void(Vector2d)>> m_cb_rightclick;
	std::vector<CallbackForm<void(Vector2d)>> m_cb_map_moved;
	std::vector<CallbackForm<void(Vector2d)>> m_cb_cursor_moved;

	std::vector<CallbackForm<void(traffic::Rect)>> m_cb_view_changed;
	std::vector<CallbackForm<void(double)>> m_cb_zoom_changed;
	std::vector<CallbackForm<void(double)>> m_cb_rotation_changed;

	nanogui::ref<nanogui::Shader> m_shader;
	nanogui::ref<nanogui::Shader> m_chunk_shader;

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

