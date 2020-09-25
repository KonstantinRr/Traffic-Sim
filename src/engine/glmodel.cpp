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

#include "module.hpp"
#include "glmodel.hpp"

using namespace lt;
using namespace lt::resource;

GLModel::GLModel(GLsizei modelSize, GLuint vao, GLuint vbo) {
	this->modelSize = modelSize;
	this->vao = vao;
	this->vbo = vbo;
}

GLModel::GLModel(const lt::resource::MeshBuilder2D::ExportFile2D& file)
{
    generateVAO();
    CGL(glGenBuffers(1, &vbo));
    CGL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    CGL(glBufferData(GL_ARRAY_BUFFER, file.data.size() * sizeof(float),
        file.data.data(), GL_STATIC_DRAW));

    CGL(glBindBuffer(GL_ARRAY_BUFFER, vbo));

    // Creates the needed glVertexAttribPointers
    size_t strideElem = 0;
    for (size_t i = 0; i < file.exp.size(); i++)
        strideElem += file.exp[i].size;

    size_t offset = 0;
    for (size_t i = 0; i < file.exp.size(); i++) {
        CGL(glVertexAttribPointer(
            i, file.exp[i].size, GL_FLOAT, GL_FALSE,
            strideElem * sizeof(float), // stride
            (void*)(sizeof(float) * offset) // initial 
        ));
        offset += file.exp[i].size;
    }
    modelSize = static_cast<GLsizei>(file.data.size() / strideElem);
    indexSize = 0;

    // Enables all used VertexAttribPointers
    for (size_t i = 0; i < file.exp.size(); i++)
        CGL(glEnableVertexAttribArray(i));

    CGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CGL(glBindVertexArray(0));
}

GLModel::GLModel(const std::vector<lt::resource::Vertex2D> &vertices) {
    modelSize = static_cast<GLsizei>(vertices.size());
    indexSize = 0;
    type = ModelType::VERTEX2D;
    generateVAO();
    generateVBOVertexArray2D(vertices);
}

GLModel::GLModel(const std::vector<Vertex> &vertices) {
	modelSize = static_cast<GLsizei>(vertices.size());
    indexSize = 0;
    type = ModelType::VERTEX;
    generateVAO();
    generateVBOVertexArray(vertices);
}

GLModel::GLModel(const std::vector<PointVertex> &vertices) {
	modelSize = static_cast<GLsizei>(vertices.size());
    indexSize = 0;
    type = ModelType::POINT_VERTEX;
    generateVAO();
    generateVBOPointVertexArray(vertices);
}

GLModel::GLModel(const std::vector<NormalVertex> &vertices) {
	modelSize = static_cast<GLsizei>(vertices.size());
    indexSize = 0;
    type = ModelType::NORMAL_VERTEX;
	generateVAO();
    generateVBONormalVertexArray(vertices);
}

GLModel::GLModel(const std::vector<lt::resource::Vertex> &vertices, const std::vector<size_t> &index) {
    modelSize = static_cast<GLsizei>(vertices.size());
    indexSize = static_cast<GLsizei>(index.size());
    type = ModelType::VERTEX_INDEXED;
    generateVAO();
    generateVIO(index);
    generateVBOVertexArray(vertices);
}

GLModel::GLModel(const std::vector<lt::resource::PointVertex> &vertices, const std::vector<size_t> &index) {
    modelSize = static_cast<GLsizei>(vertices.size());
    indexSize = static_cast<GLsizei>(index.size());
    type = ModelType::POINT_VERTEX_INDEXED;
    generateVAO();
    generateVIO(index);
    generateVBOPointVertexArray(vertices);
}
GLModel::GLModel(const std::vector<lt::resource::NormalVertex> &vertices, const std::vector<size_t> &index) {
    modelSize = static_cast<GLsizei>(vertices.size());
    indexSize = static_cast<GLsizei>(index.size());
    type = ModelType::NORMAL_VERTEX_INDEXED;
    generateVAO();
    generateVIO(index);
    generateVBONormalVertexArray(vertices);
}


void GLModel::generateVAO() {
    CGL(glGenVertexArrays(1, &vao));
    CGL(glBindVertexArray(vao));
}

void GLModel::generateVIO(const std::vector<size_t> &index) {
    glGenBuffers(1, &vio);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * 2, index.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLModel::generateVBOVertexArray2D(const std::vector<lt::resource::Vertex2D> &vertices) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*) (sizeof(float) * 2));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLModel::generateVBOVertexArray(const std::vector<lt::resource::Vertex> &vertices) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 6));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLModel::generateVBOPointVertexArray(const std::vector<lt::resource::PointVertex> &vertices) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PointVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLModel::generateVBONormalVertexArray(const std::vector<lt::resource::NormalVertex> &vertices) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NormalVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLModel::bind() {
    CGL(glBindVertexArray(vao));
    CGL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
}

void GLModel::unbind() {
    CGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CGL(glBindVertexArray(0));
}

void GLModel::cleanUp() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
    vbo = std::numeric_limits<GLuint>::max();
    vao = std::numeric_limits<GLuint>::max();
}

GLsizei GLModel::getSize() const { return modelSize; }
GLuint GLModel::getVAO() const { return vao; }
GLuint GLModel::getVBO() const { return vbo; }

// ---- GLTexture2D ---- //
void GLTexture2D::applyFilters() {
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void GLTexture2D::genTexture() {
    hasTexture = true;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
}

GLTexture2D::GLTexture2D() {}
/*
GLTexture2D::GLTexture2D(
    const std::shared_ptr<GLModel> &model,
    const std::string &textureFile
) {
    this->model = model;
    genTexture();

    // Push image data to texture.
    LTImage image = loadImage(textureFile);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        static_cast<GLsizei>(image.getWidth()),
        static_cast<GLsizei>(image.getHeight()),
        0, GL_RGBA, GL_UNSIGNED_BYTE, image.getData());

    applyFilters();
}
*/

GLTexture2D::GLTexture2D(const lt::ImageRGB8 &image) {
    genTexture();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
        static_cast<GLsizei>(image.getXExtent()),
        static_cast<GLsizei>(image.getYExtent()),
        0, GL_RGB, GL_UNSIGNED_BYTE, image.getData());

    applyFilters();
}


GLTexture2D::GLTexture2D(const lt::ImageBGR8 &image) {
    genTexture();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
        static_cast<GLsizei>(image.getXExtent()),
        static_cast<GLsizei>(image.getYExtent()),
        0, GL_BGR, GL_UNSIGNED_BYTE, image.getData());

    applyFilters();
}


GLTexture2D::GLTexture2D(const lt::ImageRGBA8 &image) {
    genTexture();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        static_cast<GLsizei>(image.getXExtent()),
        static_cast<GLsizei>(image.getYExtent()),
        0, GL_RGBA, GL_UNSIGNED_BYTE, image.getData());

    applyFilters();
} 


GLTexture2D::~GLTexture2D() {
    cleanup();
}

void GLTexture2D::cleanup() {
    if (hasTexture) {
        glDeleteTextures(1, &texture);
    }
}

void GLTexture2D::bind() {
    if (hasTexture) {
        glBindTexture(GL_TEXTURE_2D, texture);
    }
}

GLuint GLTexture2D::getTexture() const { return texture; }

// ---- GLTexturedModel ---- //

/*
GLTexturedModel::GLTexturedModel(
        const std::shared_ptr<GLModel> &model,
        const std::string &textureFile,
        const glm::vec4 &material) {
    this->model = model;
    this->material = material;
    this->hasTexture = true;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // TODO
    // Push image data to texture.
    LTImage image = loadImage(textureFile);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        static_cast<GLsizei>(image.getWidth()),
        static_cast<GLsizei>(image.getHeight()),
        0, GL_RGBA, GL_UNSIGNED_BYTE, image.getData());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
*/

GLTexturedModel::GLTexturedModel(const std::shared_ptr<GLModel> &model,
                const glm::vec4 &material) {
    this->model = model;
    this->material = material;
    this->hasTexture = false;
}

GLTexturedModel::~GLTexturedModel() {
    cleanUp();
}

void GLTexturedModel::cleanUp() {
    if (hasTexture) {
        glDeleteTextures(1, &texture);
    }
}
void GLTexturedModel::bind() {
    bindModel();
    bindTexture();
}
    
void GLTexturedModel::bindTexture() {
    if (hasTexture) {
        glBindTexture(GL_TEXTURE_2D, texture);
    }
}
void GLTexturedModel::bindModel() {
    model->bind();
}

const glm::vec4& GLTexturedModel::getMaterial() const { return material; }
std::shared_ptr<GLModel> GLTexturedModel::getModel() { return model; }
GLuint GLTexturedModel::getTexture() const { return texture; }
