/// BEGIN LICENSE
///
/// Copyright 2020 Konstantin Rolf <konstantin.rolf@gmail.com>
/// Permission is hereby granted, free of charge, to any person obtaining a copy of
/// this software and associated documentation files (the "Software"), to deal in
/// the Software without restriction, including without limitation the rights to
/// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/// of the Software, and to permit persons to whom the Software is furnished to do
/// so, subject to the following conditions:
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
/// END LICENSE

#include <iostream>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

#include "resource.hpp"

using namespace lt;
using namespace lt::render;
using namespace lt::render::shader;
using namespace lt::resource;

Shader::Shader(bool hasVertexShader, bool hasFragmentShader)
    : hasVertexShader(hasVertexShader), hasFragmentShader(hasFragmentShader) { }

void showShaderLog(GLuint shader) {
    spdlog::info("Could not link Shader!");
    GLint logSize = 0;
    CGL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize));
    std::vector<GLchar> errorLog(static_cast<size_t>(logSize + 1));
    errorLog[static_cast<size_t>(logSize)] = '\0';
    CGL(glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]));
    spdlog::error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n{}", errorLog.data());
}

void showInfoLog(GLuint program) {
    spdlog::info("Could not link Program");
    GLint logSize = 0;
    CGL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize));
    std::vector<GLchar> errorLog(static_cast<size_t>(logSize + 1));
    errorLog[static_cast<size_t>(logSize)] = '\0';
    CGL(glGetProgramInfoLog(program, logSize, &logSize, &errorLog[0]));
    spdlog::error("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}", errorLog.data());
}

void Shader::cleanup(GLuint vertex_shader, GLuint fragment_shader) {
    if (hasVertexShader) CGL(glDeleteShader(vertex_shader));
    if (hasFragmentShader) CGL(glDeleteShader(fragment_shader));
}

void Shader::create() {
    int success;

    program = glCreateProgram();
    GLuint vertex_shader = 0, fragment_shader = 0;

    if (hasVertexShader) {
        spdlog::info("Creating vertex shader");
        auto src = retrieveVertexShader();
        spdlog::info("Retrieved shader source \n'{}'", src.data());

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar *srcPointer = (const GLchar *)src.data();
        CGL(glShaderSource(vertex_shader, 1, &(srcPointer), NULL));
        CGL(glCompileShader(vertex_shader));
        CGL(glAttachShader(program, vertex_shader));

        CGL(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success));
        if(!success) {
            showShaderLog(vertex_shader);
            cleanup(vertex_shader, fragment_shader);
            throw std::runtime_error("Could not load vertex shader");
        }
    }
    
    if (hasFragmentShader) {
        spdlog::info("Creating Fragment Shader");
        auto src = retrieveFragmentShader();
        spdlog::info("Source {}", src.data());

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar *srcPointer = (const GLchar *)src.data();
        CGL(glShaderSource(fragment_shader, 1, &srcPointer, NULL));
        CGL(glCompileShader(fragment_shader));
        CGL(glAttachShader(program, fragment_shader));

        CGL(glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success));
        if(!success) {
            showShaderLog(fragment_shader);
            cleanup(vertex_shader, fragment_shader);
            throw std::runtime_error("Could not load fragment shader");
        };
    }

    CGL(glLinkProgram(program));
    CGL(glGetProgramiv(program, GL_LINK_STATUS, &success));
    if(!success) {
        showInfoLog(program);
        cleanup(vertex_shader, fragment_shader);
        throw std::runtime_error("Could not link shader");
    }

    cleanup(vertex_shader, fragment_shader);
    spdlog::info("Shaders successfully linked");
    initializeUniforms();
    spdlog::info("Uniforms successfully loaded");
}

void Shader::bind() {
    CGL(glUseProgram(program));
}

void Shader::release() {
    CGL(glUseProgram(0));
}

void Shader::loadFloat(GLint location, float value) { CGL(glUniform1f(location, value)); }
void Shader::loadBool(GLint location, bool value) { CGL(glUniform1i(location, value)); }
void Shader::loadInt(GLint location, int value) { CGL(glUniform1i(location, value)); }

void Shader::loadVec1(GLint location, const glm::vec1 &vec) { CGL(glUniform1f(location, vec.x)); }
void Shader::loadVec2(GLint location, const glm::vec2 &vec) { CGL(glUniform2f(location, vec.x, vec.y)); }
void Shader::loadVec3(GLint location, const glm::vec3 &vec) { CGL(glUniform3f(location, vec.x, vec.y, vec.y)); }
void Shader::loadVec4(GLint location, const glm::vec4 &vec) { CGL(glUniform4f(location, vec.x, vec.y, vec.z, vec.w)); }

void Shader::loadMat2x2(GLint location, const glm::mat2x2 &mat) { CGL(glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }
void Shader::loadMat2x3(GLint location, const glm::mat2x3 &mat) { CGL(glUniformMatrix2x3fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }
void Shader::loadMat2x4(GLint location, const glm::mat2x4 &mat) { CGL(glUniformMatrix2x4fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }

void Shader::loadMat3x2(GLint location, const glm::mat3x2 &mat) { CGL(glUniformMatrix3x2fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }
void Shader::loadMat3x3(GLint location, const glm::mat3x3 &mat) { CGL(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }
void Shader::loadMat3x4(GLint location, const glm::mat3x4 &mat) { CGL(glUniformMatrix3x4fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }

void Shader::loadMat4x2(GLint location, const glm::mat4x2 &mat) { CGL(glUniformMatrix4x2fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }
void Shader::loadMat4x3(GLint location, const glm::mat4x3 &mat) { CGL(glUniformMatrix4x3fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }
void Shader::loadMat4x4(GLint location, const glm::mat4x4 &mat) { CGL(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat))); }


GLint Shader::uniformLocation(const std::string &name) {
    GLint v = glGetUniformLocation(program, name.c_str());
    //if (v == -1) throw std::runtime_error("Could not load uniform " + name);
    return v;
}

GLuint Shader::getShaderID() {
    return program;
}

// ---- Resource Shader ----

ResourceShader::ResourceShader(bool hasVertexShader, bool hasFragmentShader)
    : Shader(hasVertexShader, hasFragmentShader) { }

std::vector<char> ResourceShader::retrieveVertexShader() {
    return readFile(getVertexSource());
}

std::vector<char> ResourceShader::retrieveFragmentShader() {
    return readFile(getFragmentSource());
}


void RenderPipeline::addStage(const std::shared_ptr<Renderable> &component) {
    renders.push_back(component);
}
void RenderPipeline::clear() {
    renders.clear();
}

void RenderPipeline::render() {
    for (auto v : renders) {
        v->render();
    }
}


template<typename EntityType>
void RenderList<EntityType>::add(const std::shared_ptr<EntityType>& entity) {
    entities.push_back(entity);
}

template<typename EntityType>
void RenderList<EntityType>::remove(const std::shared_ptr<EntityType>& entity) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if ((*it)->getID() == entity->getID()) {
            entities.erase(it);
        }
    }
}

template<typename EntityType>
void RenderList<EntityType>::clear() {
    entities.clear();
}

// ---- RenderBatch ---- //
template<typename EntityType>
void RenderBatch<EntityType>::add(const std::shared_ptr<EntityType>& entity) {
    map[entity->getTexture()->getTexture()].add(entity);
}

template<typename EntityType>
void RenderBatch<EntityType>::remove(const std::shared_ptr<EntityType>& entity) {
    map[entity->getTexture()->getTexture()].remove(entity);
}

// ---- TickerList ---- //

void TickerList::add(const std::shared_ptr<Tickable>& ticker) {
    tickables.push_back(ticker);
}
void TickerList::clear() {
    tickables.clear();
}
float TickerList::getCurrentTime() const {
    return time;
}
void TickerList::updateAll(float dt) {
    for (const auto& ticker : tickables) {
        ticker->update(time, dt);
    }
    time += dt;
}

RectStageBuffer::RectStageBuffer(
    const std::shared_ptr<RenderList<TransformableEntity2D>>& renderList)
    : renderList(renderList) { }

// ---- RectShader ---- //
RectShader::RectShader()
    : Shader(true, true) { } 

void RectShader::render(const RenderList<TransformableEntity2D>& renderList) {
    bind();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    loadTexture(0);

    for (auto& entity : renderList) {
        loadTransform(entity->calculateTransformationMatrix());
        entity->getModel()->bind(); // binds the model
        entity->getTexture()->bind(); // binds texture
        glDrawArrays(GL_TRIANGLES, 0, entity->getModel()->getSize());
    }

    release();
}

void RectShader::render(const RectStageBuffer& renderList) {
    render(*renderList.renderList);
}

void RectShader::loadTransform(const glm::mat3x3& matrix) { loadMat3x3(uniformTransform, matrix); }
void RectShader::loadTexture(GLint texture) { loadInt(uniformTexture, texture); }

void RectShader::initializeUniforms() {
    uniformTexture = uniformLocation("textureSampler");
    uniformTransform = uniformLocation("transform");
}

// ---- SimpleShader ---- //

SimpleShader::SimpleShader()
    : Shader(true, true) { }

void SimpleShader::render()
{
    // TODO
}

void SimpleShader::initializeUniforms() {}

// ---- SimpleMVPShader ---- //

MVPBatchStageBuffer::MVPBatchStageBuffer(
    const std::shared_ptr<Camera>& pCamera,
    const std::shared_ptr<RenderBatch<Entity>>& pBatch)
    : camera(pCamera), batch(pBatch) { }

MVPListStageBuffer::MVPListStageBuffer(
    const std::shared_ptr<Camera>& pCamera,
    const std::shared_ptr<RenderList<Entity>>& pList)
    : camera(pCamera), list(pList) { }


SimpleMVPShader::SimpleMVPShader()
    : Shader(true, true) { }

void SimpleMVPShader::initializeUniforms() {
    location_mvp = uniformLocation("mvp");
}

// TODO
void SimpleMVPShader::render(const Camera& camera, const RenderList<Entity>& list) {
}
void SimpleMVPShader::render(const Camera& camera, const RenderBatch<Entity>& batch) {
}
void SimpleMVPShader::render(const MVPBatchStageBuffer& stageBuffer) {
}
void SimpleMVPShader::render(const MVPListStageBuffer& stageBuffer) {
}


void SimpleMVPShader::loadMVPMatrix(const glm::mat4& mat) {
    loadMat4x4(location_mvp, mat);
}

// ---- PhongShader ---- //

PhongListStageBuffer::PhongListStageBuffer(
    const std::shared_ptr<Camera>& pCamera,
    const std::shared_ptr<RenderList<Entity>>& pRenderList,
    const glm::vec3& pLightPosition, const glm::vec3& pLightColor)
    : camera(pCamera), renderList(pRenderList),
    lightPosition(pLightPosition), lightColor(pLightColor) { }

PhongBatchStageBuffer::PhongBatchStageBuffer(
    const std::shared_ptr<Camera>& pCamera,
    const std::shared_ptr<RenderBatch<Entity>>& pRenderList,
    const glm::vec3& pLightPosition, const glm::vec3& pLightColor)
    : camera(pCamera), renderList(pRenderList),
    lightPosition(pLightPosition), lightColor(pLightColor) { }

PhongShader::PhongShader()
    : Shader(true, true) { }

void PhongShader::initializeUniforms() {
    uniformModelViewTransformPhong = uniformLocation("modelViewTransform");
    uniformProjectionTransformPhong = uniformLocation("projectionTransform");
    uniformNormalTransformPhong = uniformLocation("normalTransform");
    uniformMaterialPhong = uniformLocation("material");
    uniformLightPositionPhong = uniformLocation("lightPosition");
    uniformLightColorPhong = uniformLocation("lightColor");
    uniformTextureSamplerPhong = uniformLocation("textureSampler");
}


void PhongShader::render(const Camera& camera, const RenderList<Entity>& list,
    const glm::vec3& lightPosition, const glm::vec3& lightColor)
{
    bind();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    // loads the uniforms that stay the same
    loadProjection(camera.getProjectionMatrix());
    loadLightPosition(lightPosition);
    loadLightColor(lightColor);
    loadTexture(0);
    for (auto& entity : list) {
        loadModelView(camera.getViewMatrix() * entity->calculateTransformationMatrix());
        loadNormalMatrix(entity->calculateNormalMatrix());
        loadMaterial(entity->getTexture()->getMaterial());


        entity->getTexture()->bind(); // binds model and texture
        glDrawArrays(GL_TRIANGLES, 0, entity->getTexture()->getModel()->getSize());
    }
    release();
}

void PhongShader::render(const Camera& camera, const RenderBatch<Entity>& batch,
    const glm::vec3& lightPosition, const glm::vec3& lightColor) {
    bind();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE0);
    // loads the uniforms that stay the same
    loadProjection(camera.calculateProjectionMatrix());
    loadLightPosition(lightPosition);
    loadLightColor(lightColor);
    loadTexture(0);

    for (auto& renderListIt : batch) {
        glBindTexture(GL_TEXTURE_2D, renderListIt.first);

        for (auto& entity : renderListIt.second) {
            loadModelView(camera.calculateViewMatrix() * entity->calculateTransformationMatrix());
            loadNormalMatrix(entity->calculateNormalMatrix());
            loadMaterial(entity->getTexture()->getMaterial());

            entity->getTexture()->bindModel(); // binds the model only
            glDrawArrays(GL_TRIANGLES, 0, entity->getTexture()->getModel()->getSize());
        }
    }
    release();
}

void PhongShader::render(const PhongListStageBuffer& stageBuffer) {
    render(*stageBuffer.camera, *stageBuffer.renderList, stageBuffer.lightPosition, stageBuffer.lightColor);
}
void PhongShader::render(const PhongBatchStageBuffer& stageBuffer) {
    render(*stageBuffer.camera, *stageBuffer.renderList, stageBuffer.lightPosition, stageBuffer.lightColor);
}

void PhongShader::loadModelView(const glm::mat4x4& matrix) { loadMat4x4(uniformModelViewTransformPhong, matrix); }
void PhongShader::loadProjection(const glm::mat4x4& matrix) { loadMat4x4(uniformProjectionTransformPhong, matrix); }
void PhongShader::loadNormalMatrix(const glm::mat3x3& matrix) { loadMat3x3(uniformNormalTransformPhong, matrix); }

void PhongShader::loadMaterial(const glm::vec4& vector) { loadVec4(uniformMaterialPhong, vector); }
void PhongShader::loadLightPosition(const glm::vec3& vector) { loadVec3(uniformLightPositionPhong, vector); }
void PhongShader::loadLightColor(const glm::vec3& vector) { loadVec3(uniformLightColorPhong, vector); }
void PhongShader::loadTexture(GLint unit) { loadInt(uniformTextureSamplerPhong, unit); }

std::vector<char> toArray(const char* raw) {
    std::string str(raw);
    std::vector<char> v(str.length() + 1);
    std::copy(str.begin(), str.end(), v.begin());
    v.back() = '\0';
    return v;
}

std::vector<char> PhongMemoryShader::retrieveVertexShader()
{
    const char * frag = R"(
    #version 330 core

    // Specify the input locations of attributes.
    layout (location = 0) in vec3 vertCoordinates_in;
    layout (location = 1) in vec3 vertNormals_in;
    layout (location = 2) in vec2 texCoords_in;

    // Specify the uniforms of the vertex shader.
    uniform mat4 modelViewTransform;
    uniform mat4 projectionTransform;
    uniform vec3 lightPosition;
    uniform mat3 normalTransform;

    // Specify the output of the vertex stage.
    out vec3 vertNormal;
    out vec3 vertPosition;
    out vec3 relativeLightPosition;
    out vec2 texCoords;

    void main()
    {
        gl_Position  = projectionTransform * modelViewTransform * vec4(vertCoordinates_in, 1.0F);

        // Pass the required information to the fragment shader stage.
        relativeLightPosition = vec3(modelViewTransform * vec4(lightPosition, 1.0F));
        vertPosition = vec3(modelViewTransform * vec4(vertCoordinates_in, 1.0F));
        vertNormal   = normalize(normalTransform * vertNormals_in);
        texCoords    = texCoords_in;
    }
    )";
    return toArray(frag);
}

std::vector<char> PhongMemoryShader::retrieveFragmentShader()
{
    const char * frag = R"(
    #version 330 core

    // The input from the vertex shader.
    in vec3 vertNormal;
    in vec3 vertPosition;
    in vec3 relativeLightPosition;
    in vec2 texCoords;

    // Illumination model constants.
    uniform vec4 material;
    uniform vec3 lightColor;

    // Texture sampler.
    uniform sampler2D textureSampler;

    // Specify the output of the fragment shader.
    out vec4 vertColor;

    void main()
    {
        // Ambient color does not depend on any vectors.
        vec3 texColor = texture(textureSampler, texCoords).xyz;
        //vec3 texColor = vec3(0.5, 0.3, 0.5);
        vec3 color = material.x * texColor;

        // Calculate light direction vectors in the Phong illumination model.
        vec3 lightDirection = normalize(relativeLightPosition - vertPosition);
        vec3 normal = normalize(vertNormal);

        // Diffuse color.
        float diffuseIntensity = max(dot(normal, lightDirection), 0.0F);
        color += texColor * material.y * diffuseIntensity;

        // Specular color.
        vec3 viewDirection = normalize(-vertPosition); // The camera is always at (0, 0, 0).
        vec3 reflectDirection = reflect(-lightDirection, normal);
        float specularIntensity = max(dot(reflectDirection, viewDirection), 0.0F);
        color += lightColor * material.z * pow(specularIntensity, material.w);

        vertColor = vec4(color, 1.0F);
    }
    )";
    return toArray(frag);
}

std::vector<char> MemoryRectShader::retrieveFragmentShader()
{
    const char * frag = R"(
    #version 330 core

    in vec2 texturePosition;
    out vec3 color;
    uniform sampler2D textureSampler;

    void main(){
        color = texture(textureSampler, texturePosition).xyz;
    }
    )";
    return toArray(frag);
}

std::vector<char> MemoryRectShader::retrieveVertexShader()
{
    const char * vert = R"(
    #version 330 core

    layout(location = 0) in vec2 vertexPosition;
    layout(location = 1) in vec2 vertexTexturePosition;

    out vec2 texturePosition;

    // Values that stay constant for the whole mesh.
    uniform mat3 transform;
  
    void main(){
      gl_Position = vec4(transform * vec3(vertexPosition, -1.0), 1.0);
      texturePosition = vertexTexturePosition;
    }
    )";
    return toArray(vert);
}

std::vector<char> SimpleMemoryShader::retrieveVertexShader()
{
    return std::vector<char>();
}

std::vector<char> SimpleMemoryShader::retrieveFragmentShader()
{
    return std::vector<char>();
}

TriangleShader::TriangleShader() { }

void TriangleShader::initializeUniforms()
{
}

std::vector<char> TriangleMemoryShader::retrieveVertexShader()
{
    const char * vert = R"(
    #version 330 core

    layout(location = 0) in vec3 vertexPosition_modelspace;

    void main(){
        gl_Position.xyz = vertexPosition_modelspace;
        gl_Position.w = 1.0;
    }
    )";
    return toArray(vert);
}

std::vector<char> TriangleMemoryShader::retrieveFragmentShader()
{
    const char * frag = R"(
    #version 330 core
    out vec3 color;

    void main(){
        color = vec3(1,0,0);
    }
    )";
    return toArray(frag);
}

std::vector<char> lt::render::shader::SimpleMVPMemoryShader::retrieveVertexShader()
{
    const char * vert = R"(
    #version 330 core

    layout(location = 0) in vec3 vertexPosition_modelspace;
  
    // Values that stay constant for the whole mesh.
    uniform mat4 mvp;
  
    void main(){
      // Output position of the vertex, in clip space : MVP * position
      gl_Position =  mvp * vec4(vertexPosition_modelspace, 1.0);
    }
    )";
    return toArray(vert);
}

std::vector<char> lt::render::shader::SimpleMVPMemoryShader::retrieveFragmentShader()
{
    const char * frag = R"(
    #version 330 core

    out vec3 color;

    void main(){
        color = vec3(1,0,0);
    }
    )";
    return toArray(frag);
}

LineShader::LineShader()
    : Shader(true, true) { }

void LineShader::initializeUniforms()
{
    uniformMVP = uniformLocation("mvp");
}

void LineShader::loadMVP(const glm::mat4x4& mat) { loadMat4x4(uniformMVP, mat); }

void LineShader::render(const LineStageBuffer& stageBuffer)
{
    bind();

    for (const auto& entity : *(stageBuffer.renderList)) {
        loadMVP(entity->getTransform4D());

        entity->getModel()->bind();
        CGL(glDrawArrays(GL_LINES, 0, entity->getModel()->getSize()));
    }
    release();
}

std::vector<char> LineMemoryShader::retrieveVertexShader()
{
    const char * vert = R"(
    #version 330

    uniform mat4 mvp;

    in vec2 vVertex;
    in vec3 color;
    out vec3 mixedColor;

    void main(void) {
	    gl_Position = mvp * vec4(vVertex, 0.0, 1.0);
	    mixedColor = color;
    })";
    return toArray(vert);
}

std::vector<char> LineMemoryShader::retrieveFragmentShader()
{
    const char * frag = R"(
    #version 330
    in vec3 mixedColor;

    out vec4 color;

    void main() {
        color = vec4(mixedColor, 1.0);
    })";
    return toArray(frag);
}

LineStageBuffer::LineStageBuffer(
    const std::shared_ptr<RenderList<Entity2D>>& list)
    : renderList(list) { }

// Explicit instantiations
template class lt::render::shader::RenderList<Entity>;
template class lt::render::shader::RenderBatch<Entity>;
template class lt::render::shader::RenderList<Entity2D>;
template class lt::render::shader::RenderBatch<Entity2D>;