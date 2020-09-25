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

#ifndef GLMODEL_H
#define GLMODEL_H

#include "module.hpp"

#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "graphics.hpp"
#include "resource.hpp"

namespace lt
{

    enum ModelType {
        VERTEX, POINT_VERTEX, NORMAL_VERTEX,
        VERTEX2D, VERTEX_INDEXED, POINT_VERTEX_INDEXED,
        NORMAL_VERTEX_INDEXED
    };

    class GLModel {
    protected:
        GLuint vao, vbo, vio;
        GLsizei modelSize, indexSize;
        ModelType type;
    
    public:
        GLModel(GLsizei modelSize, GLuint vao, GLuint vbo);
        GLModel(const lt::resource::MeshBuilder2D::ExportFile2D &file);

        GLModel(const std::vector<lt::resource::Vertex2D> &vertices);
        GLModel(const std::vector<lt::resource::Vertex> &vertices);
        GLModel(const std::vector<lt::resource::PointVertex> &vertices);
        GLModel(const std::vector<lt::resource::NormalVertex> &vertices);

        GLModel(const std::vector<lt::resource::Vertex> &vertices, const std::vector<size_t> &index);
        GLModel(const std::vector<lt::resource::PointVertex> &vertices, const std::vector<size_t> &index);
        GLModel(const std::vector<lt::resource::NormalVertex> &vertices, const std::vector<size_t> &index);

        void generateVAO();
        void generateVIO(const std::vector<size_t> &index);
        void generateVBOVertexArray2D(const std::vector<lt::resource::Vertex2D> &vertices);
        void generateVBOVertexArray(const std::vector<lt::resource::Vertex> &vertices);
        void generateVBOPointVertexArray(const std::vector<lt::resource::PointVertex> &vertices);
        void generateVBONormalVertexArray(const std::vector<lt::resource::NormalVertex> &vertices);

        void cleanUp();
        void bind();
        void unbind();
    
        GLsizei getSize() const;
        GLuint getVAO() const;
        GLuint getVBO() const;
    };


    class GLTexture2D {
    protected:
        GLuint texture; 
        bool hasTexture;

    protected:
        void applyFilters();
        void genTexture();
    public:
        GLTexture2D();

        GLTexture2D(const lt::ImageRGB8 &image);
        GLTexture2D(const lt::ImageBGR8 &image);
        GLTexture2D(const lt::ImageRGBA8 &image);
    
        ~GLTexture2D();

        void cleanup();
        void bind();

        GLuint getTexture() const;
    };

    /**
     * @brief The GLTexturedModel class
     *
     * A GLModel with a texture and material specs
     */
    class GLTexturedModel {
    protected:
        std::shared_ptr<GLModel> model;
        glm::vec4 material;
        GLuint texture;
        bool hasTexture;

    public:
        GLTexturedModel() = default;
        GLTexturedModel(const std::shared_ptr<GLModel> &model,
                        const std::string &texture,
                        const glm::vec4 &material = {0.5F, 0.5F, 0.5F, 5.0F});
        GLTexturedModel(const std::shared_ptr<GLModel> &model,
                        const glm::vec4 &material = {0.5F, 0.5F, 0.5F, 5.0F});
        ~GLTexturedModel();

        //QVector<quint8> imageToBytes(QImage image);

        void cleanUp();
        void bind();
        void bindTexture();
        void bindModel();

        std::shared_ptr<GLModel> getModel();
        const glm::vec4& getMaterial() const;
        GLuint getTexture() const;
    };
}

#endif // GLMODEL_H
