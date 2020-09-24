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

#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "com.h"
#include "tiny_obj_loader.hpp"

#include <stdlib.h>
#include <stdio.h>

#include <vector>
#include <string>
#include <glm/glm.hpp>


namespace lt {
namespace resource {
    /// -- class PointVertex2D --
    /// This data struct stores 2 dimensional vertex
    /// data in the following layout:
    /// float32 x, float32 y
    struct PointVertex2D {
        float data[2];

        PointVertex2D();
        PointVertex2D(float x, float y);
    };

    /// -- class Vertex2D --
    /// This data struct stores 2 dimensional vertex
    /// and texture data in the following layout:
    /// float32 x, float32 y
    /// float32 tx, float32 ty 
    struct Vertex2D {
        float data[4];

        Vertex2D();
        Vertex2D(float x, float y, float tx, float ty);
    };

    /// -- class Point Vertex --
    /// This data struct stores 3 dimensional vertex
    /// data in the following layout:
    /// float32 x, float32 y, float32 z
    struct PointVertex {
        float data[3];

        PointVertex();
        PointVertex(float x, float y, float z);
    };

    /// -- class NormalVertex --
    /// This data struct stores 3 dimensional vertex,
    /// and normal data in the following layout:
    /// float32 x, float32 y, float32 z
    /// float32 nx, float32 ny, float32 nz
    struct NormalVertex {
        float data[6];

        NormalVertex();
        NormalVertex(
            float x, float y, float z,
            float nx, float ny, float nz);
    };

    /// This data struct stores 3 dimensional vertex,
    /// normal and texture data in the following layout:
    /// float32 x, float32 y, float32 z
    /// float32 nx, float32 ny, float32 nz
    /// float32 tx, float32 ty
    struct Vertex {
        float data[8];
        
        // Constructors
        Vertex();
        Vertex(float x, float y, float z,
            float nx, float ny, float nz,
            float tx1, float ty1);
    };

    struct Indice {
        int v, t, n;
        
        Indice(int v, int t, int n);
    };

    class HeightMap {
    protected:
        std::vector<std::vector<float>> heightMap;

    public:
        HeightMap(size_t size);
        HeightMap(const std::string &filename);

        void fillRandom();

        void scaleHeight(float scale);

        inline size_t getSize() const { return heightMap.size(); }
        inline auto& getHeightMap() { return heightMap; }
        inline const auto& getHeightMap() const { return heightMap; }
        inline auto& operator[](size_t index) { return heightMap[index]; }
        inline const auto& operator[](size_t index) const { return heightMap[index]; }
    };

    class MeshBuilder2D {
    public:
        std::vector<glm::vec2> vertices;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> colors;
        std::vector<int> v_indices;
        std::vector<int> vt_indices;

    public:
        enum ExportType { VERTEX, TEXTURE, COLOR };

        struct ExportMacro {
            ExportType type;
            size_t size;
        };

        struct ExportFile2D {
            std::vector<float> data;
            std::vector<ExportMacro> exp;
        };

        class Exporter2D {
        protected:
            MeshBuilder2D* builder;
            std::vector<ExportType> exp;

            void addVec2(ExportFile2D& exp, glm::vec2 vec) const;
            void addVec3(ExportFile2D& exp, glm::vec3 vec) const;

        public: // Can only created by parent class
            Exporter2D(MeshBuilder2D* builder);

        public:
            inline Exporter2D& addVertex() { exp.push_back(VERTEX); return *this; }
            inline Exporter2D& addTexture() { exp.push_back(TEXTURE); return *this; }
            inline Exporter2D& addColor() { exp.push_back(COLOR); return *this; }

            ExportFile2D exportData() const;
        };


        MeshBuilder2D();
        MeshBuilder2D(
            const std::vector<glm::vec2> &vertices,
            const std::vector<glm::vec2> &texCoords,
            const std::vector<glm::vec3> &colors,
            const std::vector<int> &v_indices,
            const std::vector<int> &vt_indices);

        void clear();

        Exporter2D exporter();

        float maxExtent() const;
        float minExtent() const;

        void scale(float scale);
        void unitize(float unitScale=1.0f);

        void addVertex(glm::vec2 vertex);
        void addTextureCoord(glm::vec2 texture);
        void addColor(glm::vec3 color);

        void setVertices(const std::vector<glm::vec2> &vertices);
        void setTextureCoords(const std::vector<glm::vec2> &textureCoords);
        void setColors(const std::vector<glm::vec3> &colors);
        void setVertices(std::vector<glm::vec2>&& vertices);
        void setTextureCoords(std::vector<glm::vec2>&& textureCoords);
        void setColors(std::vector<glm::vec3>&& colors);


        void setVIndices(const std::vector<int> &v_indices);
        void setVTIndices(const std::vector<int> &vt_indices);
        void setVIndices(std::vector<int>&& v_indices);
        void setVTIndices(std::vector<int>&& vt_indices);

        const std::vector<glm::vec2>& getVertices() const;
        const std::vector<glm::vec2>& getTextureCoords() const;
        const std::vector<glm::vec3>& getColors() const;

        const std::vector<int>& getV_indices() const;
        const std::vector<int>& getVt_indices() const;

        std::vector<Vertex2D> toVertexArray(float scaleModif=1.0f);
        std::vector<Vertex2D> toVertexArrayIndexed(float scaleModif=1.0f);
    };


    class MeshBuilder {
    public:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texcoords;
        std::vector<int> v_indices;
        std::vector<int> vn_indices;
        std::vector<int> vt_indices;

        std::vector<tinyobj::material_t> materials;

    public:
        MeshBuilder();
        MeshBuilder(
            const std::vector<glm::vec3> &vertices,
            const std::vector<glm::vec3> &normals,
            const std::vector<glm::vec2> &texcoords,
            const std::vector<int>& v_indices,
            const std::vector<int>& vn_indices,
            const std::vector<int>& vt_indices);
        static MeshBuilder fromOBJ(const std::string &file, const std::string &material);

        void clear();

        float maxExtent() const;
        float minExtent() const;

        void scale(float scale);
        void unitize(float unitScale=1.0f);

        void setVertices(const std::vector<glm::vec3> &vertices);
        void setNormals(const std::vector<glm::vec3> &normals);
        void setTexCoords(const std::vector<glm::vec2> &texCoords);
        void setVIndices(const std::vector<int> &indices);
        void setVNIndices(const std::vector<int> &indices);
        void setVTIndices(const std::vector<int> &indices);

        const std::vector<glm::vec3>& getVertices() const;
        const std::vector<glm::vec3>& getNormals() const; 
        const std::vector<glm::vec2>& getTextureCoords() const;
        const std::vector<int>& getVIndices() const;
        const std::vector<int>& getVNIndices() const;
        const std::vector<int>& getVTIndices() const;

        std::vector<Vertex> toVertexArray();
        std::vector<PointVertex> toPointVertexArray();
        std::vector<NormalVertex> toNormalVertexArray();

        std::vector<Vertex> toVertexArrayIndexed();
        std::vector<PointVertex> toPointVertexArrayIndexed();
        std::vector<NormalVertex> toNormalVertexArrayIndexed();
    };

    class LTImage {
    protected:
        std::vector<char> data;
        size_t width, height;

    public:
        LTImage(size_t width, size_t height, std::vector<char> &&data);

        size_t getWidth() const;
        size_t getHeight() const;
        size_t getSize() const;

        char* getData();
        const char* getData() const;
    };

    MeshBuilder loadMesh(const std::string &file, const std::string &material);
    MeshBuilder loadCube();
    MeshBuilder loadTriangle();
    MeshBuilder2D loadTriangle2D();
    MeshBuilder2D loadRect2D();

    std::vector<char> readFile(const std::string &file);

    //LTImage loadImage(const std::string &file);
}
}

#endif
