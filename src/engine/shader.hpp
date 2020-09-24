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

#ifndef SHADER_HPP
#define SHADER_HPP

#include "com.h"
#include <glm/glm.hpp>

#include <string> // usage: std::string
#include <vector> // usage: std::vector
#include <memory> // usage: std::shared_ptr

#include "entity.hpp"
#include "camera.hpp"

namespace lt::render
{
	/// <summary>
	/// A general interface that defines the capability of rendering objects.
	/// </summary>
	class Renderable {
	public:
		virtual void render() = 0;
	};


	/// <summary>
	/// A general shader that loads its shader sources by invoking callback functions. The constructor identifies
	/// which shaders need to be build (Fragment, Vertex, Geometry). The fragment and vertex shaders are required
	/// to build a complete shader pipeline.
	/// </summary>
	class ShaderBase {
	protected:
		/// <summary>The OpenGL program ID that is generated</summary>
		GLuint program;

		/// <summary> Whether this shader contains a vertex unit. All OpenGL pipelines require a Vertex shader.
		/// This argument must therefore be true. </summary>
		bool hasVertexShader;
		/// <summary> Whether this shader contains a fragment unit. All OpenGL pipelines require a fragment shader.
		/// This argument must therefore be true. </summary>
		bool hasFragmentShader;

		/// Private cleanup function taking care of freeing all allocated OpenGL shader resources.

		/// <summary>Private cleanup function taking care of freeing all allocated OpenGL shader resources.</summary>
		/// <param name="vertex_shader">ID of the OpenGL vertex shader program</param>
		/// <param name="fragment_shader">ID of the OpenGL fragment shader program</param>
		void cleanupParts(
			GLuint vertex_shader,
			GLuint fragment_shader);

	public:
		/// Initializes a new shader unit with the given specs.
		/// This does not create the shader yet.
		ShaderBase(
			bool hasVertexShader=true,
			bool hasFragmentShader=true);
		ShaderBase(const ShaderBase&) = delete; 
		ShaderBase(ShaderBase &&sh);

		ShaderBase& operator=(const ShaderBase&) = delete;
		ShaderBase& operator=(ShaderBase &&sh);

		virtual ~ShaderBase();
		void cleanUp();

		/// Invokes the shader creation process. This is
		void create();

		virtual std::vector<char> retrieveVertexShader() { throw std::runtime_error("Not supported"); }
		virtual std::vector<char> retrieveFragmentShader() { throw std::runtime_error("Not supported"); }
		virtual std::vector<char> retrieveComputeShader() { throw std::runtime_error("Not supported"); }

		virtual void initializeUniforms() = 0;

		/// Invokes the OpenGL shader binding process.
		/// Releases the OpenGL shader process.
		void bind();
		void release();

		//---- Loading to Shaders ----//

		void loadFloat(GLint location, float value);
		void loadBool(GLint location, bool value);
		void loadInt(GLint location, int value);

		void loadVec1(GLint location, const glm::vec1& vec);
		void loadVec2(GLint location, const glm::vec2& vec);
		void loadVec3(GLint location, const glm::vec3& vec);
		void loadVec4(GLint location, const glm::vec4& vec);

		void loadMat2x2(GLint location, const glm::mat2x2& mat);
		void loadMat2x3(GLint location, const glm::mat2x3& mat);
		void loadMat2x4(GLint location, const glm::mat2x4& mat);

		void loadMat3x2(GLint location, const glm::mat3x2& mat);
		void loadMat3x3(GLint location, const glm::mat3x3& mat);
		void loadMat3x4(GLint location, const glm::mat3x4& mat);

		void loadMat4x2(GLint location, const glm::mat4x2& mat);
		void loadMat4x3(GLint location, const glm::mat4x3& mat);
		void loadMat4x4(GLint location, const glm::mat4x4& mat);

		/// Returns the uniform location that is bound
		/// by this name. Returns -1 if the name was not found
		GLint uniformLocation(const std::string& name);

		/// Returns the shader program ID
		GLuint getShaderID();
	};

	/// <summary>
	/// This class implements a general shader interface that retrieves its source code from a file in memory.
	/// It defines two additional abstract methods that need to be implemented to gain the filenames that
	/// need to be loaded by the ResourceShader.
	/// </summary>
	class ResourceShader : public ShaderBase {
	public:
		ResourceShader(bool hasVertexShader, bool hasFragmentShader);
		virtual ~ResourceShader() = default;

		// ---- Filename callbacks ---- //
		virtual std::string getVertexSource() = 0;
		virtual std::string getFragmentSource() = 0;

		// ---- ShaderBase implementation ---- //
		virtual std::vector<char> retrieveVertexShader();
		virtual std::vector<char> retrieveFragmentShader();
	};

	/// <summary>
	/// This class defines a component of a render pipeline. It consists of a ShaderBase object and a StageComponent.
	/// The StageComponent is responsible of delivering the data to the ShaderBase class. Both classes allow a clean
	/// separation between the rendering process and the data that is being rendered. The ShaderBase object must be
	/// able to accept an argument of the class StageType to its render function.
	/// </summary>
	/// <typeparam name="StageType">Template class of the StageBuffer component</typeparam>
	/// <typeparam name="ShaderType">Template class of the ShaderBase component</typeparam>
	template<typename StageType, typename ShaderType>
	class RenderComponent : public Renderable {
	protected:
		StageType vStageBuffer;
		std::shared_ptr<ShaderType> shader;

	public:
		RenderComponent() = default;

		/// <summary>Creates a new RenderComponent that uses the given ShaderBase object. </summary>
		/// <param name="pShader">ShaderBase object used by this RenderComponent</param>
		RenderComponent(const std::shared_ptr<ShaderType>& pShader)
			: shader(pShader) { }
		
		const std::shared_ptr<ShaderType>& getShader() { return shader; }
		void setShader(const std::shared_ptr<ShaderType>& sh) { shader = sh; }

		/// <summary>Accesses the StageBuffer of this component</summary>
		/// <returns>A reference to the currently active StageBuffer</returns>
		StageType& stageBuffer() { return vStageBuffer; }
		/// <summary>Accesses the StageBuffer of this component</summary>
		/// <returns>A reference to the currently active StageBuffer</returns>
		const StageType& stageBuffer() const { return vStageBuffer; }

		/// <summary>Implementation of the Renderable interface. Forwards the call to the ShaderBase using the
		/// StageBuffer object as argument. </summary>
		virtual void render() { shader->render(vStageBuffer); }
	};

	/// <summary>
	/// The RenderPipeline class combines multiple Renderable objects together in a single object allowing efficient
	/// chaining of rendering commands. Pipelines consist of stages that are executed in the order they were added.
	/// It is also possible to add another RenderPipeline as child Renderable. The pipeline holds shared pointer
	/// references to all passed stages. The child pipelines must not invalidate their state while this pipeline
	/// is active.
	/// </summary>
	class RenderPipeline : public Renderable {
	protected:
		std::vector<std::shared_ptr<Renderable>> renders;

	public:
		/// <summary>Virtual deconstructor</summary>
		virtual ~RenderPipeline() = default;

		/// <summary>Adds a new stage to the end of the RenderPipeline</summary>
		/// <param name="component">The new component</param>
		void addStage(const std::shared_ptr<Renderable>& component);
		/// <summary>Clears all stages in the RenderPipeline</summary>
		void clear();

		/// <summary>Renders the pipeline by invoking all child pipelines. </summary>
		void render();
	};

	//---- RenderUtilities ----//

	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="EntityType"></typeparam>
	template<typename EntityType>
	class RenderList {
	protected:
		std::vector<std::shared_ptr<EntityType>> entities;

	public:
		void add(const std::shared_ptr<EntityType>& entity);
		void remove(const std::shared_ptr<EntityType>& entity);

		void clear();
		inline auto operator[](size_t index) { return entities[index]; }
		inline auto operator[](size_t index) const { return entities[index]; }

		inline auto begin() { return entities.begin(); }
		inline auto end() { return entities.end(); }
		inline auto begin() const { return entities.begin(); }
		inline auto end() const { return entities.end(); }
	};

	template<typename EntityType>
	class RenderBatch {
	protected:
		std::unordered_map<GLuint, RenderList<EntityType>> map;

	public:
		void add(const std::shared_ptr<EntityType>& entity);
		void remove(const std::shared_ptr<EntityType>& entity);

		inline auto find(GLuint key) { return map.find(key); }
		inline auto operator[](GLuint key) { return map[key]; }

		inline auto begin() { return map.begin(); }
		inline auto end() { return map.end(); }
		inline auto begin() const { return map.begin(); }
		inline auto end() const { return map.end(); }
	};

	class TickerList {
	protected:
		std::vector<std::shared_ptr<Tickable>> tickables;
		float time = 0.0;
	public:
		void add(const std::shared_ptr<Tickable>& ticker);
		void clear();

		float getCurrentTime() const;
		void updateAll(float dt);
	};

	///////////////////////////////////////////////
	// ---- Specific ShaderBase Implementations ---- //
	// LineShader
	// TriangleShader
	// RectShader
	// SimpleShader
	// SimpleMVPShader
	// PhongShader

	///////////////////////////
	// ---- Line ShaderBase ---- //

	/// <summary>The StageBuffer class for the LineShader object. </summary>
	struct LineStageBuffer {
		std::shared_ptr<RenderList<Entity2D>> renderList;

		explicit LineStageBuffer() = default;
		explicit LineStageBuffer(const std::shared_ptr<RenderList<Entity2D>> & list);
	};

	/// <summary>
	/// This class renders a simple vertex model with colors.
	/// </summary>
	class LineShader : public ShaderBase {
	protected:
		GLint uniformMVP;

	public:
		explicit LineShader();
		virtual ~LineShader() = default;
		LineShader(const LineShader&) = delete;
		LineShader(LineShader &&sh);

		LineShader& operator=(const LineShader&) = delete;
		LineShader& operator=(LineShader &&sh);

		virtual void initializeUniforms();

		void loadMVP(const glm::mat4x4 &mat);

		void render(const LineStageBuffer &stageBuffer);
	};

	class LineMemoryShader : public LineShader {
	public:
		virtual std::vector<char> retrieveVertexShader();
		virtual std::vector<char> retrieveFragmentShader();
	};

	// ---- Triangle ShaderBase ---- //

	class TriangleShader : public ShaderBase {
	public:
		explicit TriangleShader();
		virtual ~TriangleShader() = default;
		TriangleShader(const TriangleShader&) = delete;
		TriangleShader(TriangleShader &&sh);

		TriangleShader& operator=(const TriangleShader&) = delete;
		TriangleShader& operator=(TriangleShader &&sh);

		virtual void initializeUniforms();
	};

	class TriangleMemoryShader : public TriangleShader {
	public:
		virtual std::vector<char> retrieveVertexShader();
		virtual std::vector<char> retrieveFragmentShader();
	};

	// ---- RectShader ---- //
	struct RectStageBuffer {
		std::shared_ptr<RenderList<TransformableEntity2D>> renderList;

		explicit RectStageBuffer() = default;
		explicit RectStageBuffer(const std::shared_ptr<RenderList<TransformableEntity2D>>& list);
	};

	class RectShader : public ShaderBase {
	protected:
		GLint uniformTexture;
		GLint uniformTransform;

	public:
		explicit RectShader();
		virtual ~RectShader() = default;
		RectShader(const RectShader &) = delete;
		RectShader(RectShader &&sh);

		RectShader& operator=(const RectShader&) = delete;
		RectShader& operator=(RectShader &&sh);

		void render(const RenderList<TransformableEntity2D>& renderList);
		void render(const RectStageBuffer& stageBuffer);

		void loadTransform(const glm::mat3x3& matrix);
		void loadTexture(GLint texture);

		virtual void initializeUniforms();
	};

	class MemoryRectShader : public RectShader {
	public:
		virtual std::vector<char> retrieveVertexShader();
		virtual std::vector<char> retrieveFragmentShader();
	};

	//// ---- SimpleMVPShader ---- ////

	struct MVPBatchStageBuffer {
		std::shared_ptr<Camera> camera;
		std::shared_ptr<RenderBatch<Entity>> batch;

		explicit MVPBatchStageBuffer() = default;
		explicit MVPBatchStageBuffer(
			const std::shared_ptr<Camera>& camera,
			const std::shared_ptr<RenderBatch<Entity>>& batch);
	};

	struct MVPListStageBuffer {
		std::shared_ptr<Camera> camera;
		std::shared_ptr<RenderList<Entity>> list;

		explicit MVPListStageBuffer() = default;
		explicit MVPListStageBuffer(
			const std::shared_ptr<Camera>& camera,
			const std::shared_ptr<RenderList<Entity>>& list);
	};

	class SimpleMVPShader : public ShaderBase {
	protected:
		GLint location_mvp;

	public:
		explicit SimpleMVPShader();
		virtual ~SimpleMVPShader() = default;
		SimpleMVPShader(const SimpleMVPShader &) = delete;
		SimpleMVPShader(SimpleMVPShader &&sh);

		SimpleMVPShader& operator=(const SimpleMVPShader&) = delete;
		SimpleMVPShader& operator=(SimpleMVPShader &&sh);

		void render(const Camera& camera, const RenderList<Entity>& list);
		void render(const Camera& camera, const RenderBatch<Entity>& batch);
		void render(const MVPBatchStageBuffer& stageBuffer);
		void render(const MVPListStageBuffer& stageBuffer);

		virtual void initializeUniforms();

		void loadMVPMatrix(const glm::mat4& mat);
	};

	class SimpleMVPMemoryShader : public SimpleMVPShader {
	public:
		virtual std::vector<char> retrieveVertexShader();
		virtual std::vector<char> retrieveFragmentShader();
	};

	//// ---- PhongShader ---- ////

	struct PhongListStageBuffer {
		std::shared_ptr<Camera> camera;
		std::shared_ptr<RenderList<Entity>> renderList;
		glm::vec3 lightPosition;
		glm::vec3 lightColor;

		explicit PhongListStageBuffer() = default;
		explicit PhongListStageBuffer(
			const std::shared_ptr<Camera>& camera,
			const std::shared_ptr<RenderList<Entity>>& renderList,
			const glm::vec3& lightPosition, const glm::vec3& lightColor);
	};

	struct PhongBatchStageBuffer {
		std::shared_ptr<Camera> camera;
		std::shared_ptr<RenderBatch<Entity>> renderList;
		glm::vec3 lightPosition;
		glm::vec3 lightColor;

		explicit PhongBatchStageBuffer() = default;
		explicit PhongBatchStageBuffer(
			const std::shared_ptr<Camera>& camera,
			const std::shared_ptr<RenderBatch<Entity>>& renderList,
			const glm::vec3& lightPosition, const glm::vec3& lightColor);
	};

	class PhongShader : public ShaderBase {
	protected:
		GLint uniformModelViewTransformPhong;
		GLint uniformProjectionTransformPhong;
		GLint uniformNormalTransformPhong;
		GLint uniformMaterialPhong;
		GLint uniformLightPositionPhong;
		GLint uniformLightColorPhong;
		GLint uniformTextureSamplerPhong;

	public:
		explicit PhongShader();
		virtual ~PhongShader() = default;
		PhongShader(const PhongShader&) = delete;
		PhongShader(PhongShader &&sh);

		PhongShader& operator=(const PhongShader&) = delete;
		PhongShader& operator=(PhongShader &&sh);

		// Inherited from ShaderBase
		virtual void initializeUniforms();

		void render(const Camera& camera, const RenderList<Entity>& list,
			const glm::vec3& lightPosition, const glm::vec3& lightColor);
		void render(const Camera& camera, const RenderBatch<Entity>& batch,
			const glm::vec3& lightPosition, const glm::vec3& lightColor);
		void render(const PhongListStageBuffer& stageBuffer);
		void render(const PhongBatchStageBuffer& stageBuffer);

		void loadModelView(const glm::mat4x4& matrix);
		void loadProjection(const glm::mat4x4& matrix);
		void loadNormalMatrix(const glm::mat3x3& matrix);

		void loadMaterial(const glm::vec4& vector);
		void loadLightPosition(const glm::vec3& vector);
		void loadLightColor(const glm::vec3& vector);
		void loadTexture(GLint unit);
	};

	class PhongMemoryShader : public PhongShader {
	public:
		virtual std::vector<char> retrieveVertexShader();
		virtual std::vector<char> retrieveFragmentShader();
	};


	using PhongListComponent = RenderComponent<PhongListStageBuffer, PhongShader>;
	using PhongBatchComponent = RenderComponent<PhongBatchStageBuffer, PhongShader>;
	using RectListComponent = RenderComponent<RectStageBuffer, RectShader>;
} // namespace lt

#endif
