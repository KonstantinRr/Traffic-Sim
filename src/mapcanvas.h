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

#include "traffic/osm.h"

using nanogui::Matrix4f;
using nanogui::Vector3f;
using nanogui::Vector2f;
using nanogui::Vector2i;
using nanogui::Shader;
using nanogui::Canvas;
using nanogui::ref;
using Vector2d = nanogui::Array<double, 2>;
using Vector3d = nanogui::Array<double, 3>;
using Vector4d = nanogui::Array<double, 4>;

class MapCanvas;

class MapForm : public nanogui::FormHelper {
public:
	MapForm(nanogui::Screen* parent, MapCanvas* canvas = nullptr);
	MapCanvas* getCanvas() const noexcept;
	void setCanvas(MapCanvas* canvas) noexcept;

protected:
	MapCanvas* m_canvas;
	nanogui::ref<nanogui::Window> m_window;
};

/*
inline glm::mat4 toGLM(const nanogui::Matrix4f &value) { return glm::make_mat4(value.m); }
inline glm::mat3 toGLM(const nanogui::Matrix3f &value) { return glm::make_mat3(value.m); }
inline glm::mat2 toGLM(const nanogui::Matrix2f &value) { return glm::make_mat2(value.m); }

inline glm::vec4 toGLM(const nanogui::Vector4f &value) { return glm::make_vec4(value.v); }
inline glm::vec3 toGLM(const nanogui::Vector3f &value) { return glm::make_vec3(value.v); }
inline glm::vec2 toGLM(const nanogui::Vector2f &value) { return glm::make_vec2(value.v); }


nanogui::Matrix4f toView(const glm::mat4& value);
nanogui::Matrix3f toView(const glm::mat3& value);
nanogui::Matrix2f toView(const glm::mat2& value);

nanogui::Vector4f toView(const glm::vec4& value);
nanogui::Vector3f toView(const glm::vec3& value);
nanogui::Vector2f toView(const glm::vec2& value);
*/

class MapCanvas : public nanogui::Canvas
{
protected:
	nanogui::ref<nanogui::Shader> m_shader;
	nanogui::ref<nanogui::Shader> m_chunk_shader;
	MapForm* m_form;

	std::shared_ptr<traffic::OSMSegment> map;
	size_t pointsSize, chunksSize;
	bool m_active;
	bool m_render_chunk;
	bool m_mark_update = false;

	Vector2d position;
	Vector2d cursor;
	double m_zoom;

public:
	MapCanvas(Widget* parent, std::shared_ptr<traffic::OSMSegment> map, MapForm* form = nullptr);

	// ---- Position updates ---- //
	void setLatitude(double lat);
	void setLongitude(double lon);
	void setLatLon(double lat, double lon);
	void setPosition(Vector2d pos);
	void setZoom(double zoom);
	// ---- Positional reads ---- //
	double getLatitude() const;
	double getLongitude() const;
	double getCursorLatitude() const;
	double getCursorLongitude() const;
	double getZoom() const;

	void refreshView();
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

	Vector2d forwardTransform(const Vector2d &pos) const;
	Vector2d inverseTransform(const Vector2d &pos) const;
	Matrix4f createTransform() const;
	// ---- Get & Set ---- //
	void setData(
		std::shared_ptr<std::vector<glm::vec3>> colors,
		std::shared_ptr<std::vector<glm::vec2>> points);

	void setChunkData(
		std::shared_ptr<std::vector<glm::vec2>> points);

	void setActive(bool active);
};