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

#define _CRT_SECURE_NO_WARNINGS 1

#include "resource.hpp"
#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>


#include "resource.h"

using namespace lt;
using namespace lt::resource;

std::vector<char> lt::resource::readFile(const std::string &file) {
	spdlog::info("Opening resource {}", file);
	FILE *f = fopen(file.c_str(), "rb");
	if (!f) throw std::runtime_error("Could not open File");
	int rt;
	rt = fseek(f, 0, SEEK_END);
	if (rt != 0) throw std::runtime_error("Could not seek file");
	long fsize = ftell(f);
	rt = fseek(f, 0, SEEK_SET);
	if (rt != 0) throw std::runtime_error("Could not seek file");

	std::vector<char> data;
	data.resize(static_cast<size_t>(fsize + 1));
	fread(data.data(), 1, static_cast<size_t>(fsize), f);
	data[fsize] = 0;

	rt = fclose(f);
	if (rt != 0) throw std::runtime_error("Could not close file");
	return data;
}

// ---- PointVertex ---- //

PointVertex::PointVertex() :
	data{0.0f} { }
PointVertex::PointVertex(float px, float py, float pz) :
	data{px, py, pz} { }

// ---- NormalVertex ---- //

NormalVertex::NormalVertex() :
	data{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f} { }
NormalVertex::NormalVertex(
	float px, float py, float pz,
    float pnx, float pny, float pnz) :
	data{px, py, pz, pnx, pny, pnz} { }

// ---- Vertex ---- //

Vertex::Vertex() : data{0.0f} { }

Vertex::Vertex(float px, float py, float pz,
    float pnx, float pny, float pnz,
    float ptx, float pty) :
	data{px, py, pz, pnx, pny, pnz, ptx, pty} { }

// ---- Vertex2D ---- //

Vertex2D::Vertex2D() : data{0.0f} { }
Vertex2D::Vertex2D(float px, float py, float ptx, float pty)
	: data{px, py, ptx, pty} { }

/////////////////////////////
//// ---- HeightMap ---- ////
HeightMap::HeightMap(size_t size) {
    heightMap.resize(size, std::vector<float>(size));
}
HeightMap::HeightMap(const std::string &) {
	//LTImage image = loadImage(filename);
    ////image = image.scaled(imagePrev.width()/4, imagePrev.height()/4);
    //size_t size = static_cast<size_t>(image.size().width());
    //heightMap.resize(size, std::vector<float>(size));
    //for (size_t x = 0; x < size; x++) {
    //    for (size_t y = 0; y < size; y++) {
    //        QColor pixel = image.pixelColor(x, y); // Alpha component
    //        heightMap[x][y] = pixel.redF();
    //    }
    //}
}
void HeightMap::scaleHeight(float scale) {
    for (auto &line : heightMap) {
        for (auto &val : line) {
            val *= scale;
        }
    }
}

void HeightMap::fillRandom() {
    for (auto &line : heightMap) {
        for (auto &val : line) {
            val = static_cast<float>(rand())
                    / static_cast<float>(RAND_MAX);
        }
    }
}

/////////////////////////////////
//// ---- MeshBuilder2D ---- ////
MeshBuilder2D::MeshBuilder2D() {}
MeshBuilder2D::MeshBuilder2D(
    const std::vector<glm::vec2> &pVertices,
    const std::vector<glm::vec2> &pTexCoords,
	const std::vector<glm::vec3> &colors,
    const std::vector<int> &pV_indices,
    const std::vector<int> &pVt_indices)
	: vertices(pVertices), texCoords(pTexCoords), colors(colors),
	v_indices(pV_indices), vt_indices(pVt_indices) { }

void MeshBuilder2D::clear() {
	vertices.clear();
	texCoords.clear();
	colors.clear();
	v_indices.clear();
	vt_indices.clear();
}

MeshBuilder2D::Exporter2D MeshBuilder2D::exporter() { return Exporter2D(this); }

float MeshBuilder2D::maxExtent() const {
	float length = std::numeric_limits<float>::min();
	for (size_t i = 0; i < vertices.size(); i++) {
		float testLength = static_cast<float>(vertices[i].length());
		if (testLength > length) length = testLength;
	}
	return length;
}
float MeshBuilder2D::minExtent() const {
	float length = std::numeric_limits<float>::max();
	for (size_t i = 0; i < vertices.size(); i++) {
		float testLength = static_cast<float>(vertices[i].length());
		if (testLength < length) length = testLength;
	}
	return length;
}

void MeshBuilder2D::scale(float scale) {
	for (size_t i = 0; i < vertices.size(); i++) {
		vertices[i] *= scale;
	}
}

void MeshBuilder2D::unitize(float unitScale) {
	scale(unitScale / maxExtent());
}

void MeshBuilder2D::addVertex(glm::vec2 vertex) { vertices.push_back(vertex); }
void MeshBuilder2D::addTextureCoord(glm::vec2 vertex) { texCoords.push_back(vertex); }
void MeshBuilder2D::addColor(glm::vec3 color) { colors.push_back(color); }

void MeshBuilder2D::setVertices(const std::vector<glm::vec2> &vertices) { this->vertices = vertices; }
void MeshBuilder2D::setTextureCoords(const std::vector<glm::vec2> &texCoords) { this->texCoords = texCoords; }
void MeshBuilder2D::setColors(const std::vector<glm::vec3>& colors) { this->colors = colors;}

void MeshBuilder2D::setVertices(std::vector<glm::vec2>&& vertices) { this->vertices = std::move(vertices); }
void MeshBuilder2D::setTextureCoords(std::vector<glm::vec2>&& textureCoords) { this->texCoords = std::move(textureCoords); }
void MeshBuilder2D::setColors(std::vector<glm::vec3>&& colors) { this->colors = std::move(colors); }

void MeshBuilder2D::setVIndices(const std::vector<int> &pv_indices) { this->v_indices = pv_indices; }
void MeshBuilder2D::setVTIndices(const std::vector<int> &pvt_indices) { this->vt_indices = pvt_indices; }

void MeshBuilder2D::setVIndices(std::vector<int>&& v_indices) { this->vt_indices = std::move(v_indices); }
void MeshBuilder2D::setVTIndices(std::vector<int>&& vt_indices) { this->vt_indices = std::move(vt_indices); }

const std::vector<glm::vec2>& MeshBuilder2D::getVertices() const { return vertices; }
const std::vector<glm::vec2>& MeshBuilder2D::getTextureCoords() const { return texCoords; }
const std::vector<glm::vec3>& MeshBuilder2D::getColors() const { return colors; }
const std::vector<int>& MeshBuilder2D::getV_indices() const { return v_indices; }
const std::vector<int>& MeshBuilder2D::getVt_indices() const { return vt_indices; }

std::vector<Vertex2D> MeshBuilder2D::toVertexArray(float scaleModif) {
	std::vector<Vertex2D> vertexData(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++) {
		vertexData[i] = Vertex2D(
			vertices[i].x * scaleModif,
			vertices[i].y * scaleModif,
			texCoords[i].x, texCoords[i].y);
	}
	return vertexData;
}

// TODO what does this function do?
std::vector<Vertex2D> MeshBuilder2D::toVertexArrayIndexed(float scaleModif) {
	std::vector<Vertex2D> vertexData(vertices.size());
	scaleModif;
	//for (size_t i = 0; i < vertices.size(); i++) {
	//	vertexData[i] = vertices[i] * scaleModif;
	//}
	return vertexData;
}

///////////////////////////////
//// ---- MeshBuilder ---- ////

MeshBuilder::MeshBuilder() { }
MeshBuilder::MeshBuilder(
    const std::vector<glm::vec3> &pVertices,
    const std::vector<glm::vec3> &pNormals,
    const std::vector<glm::vec2> &pTexcoords,
    const std::vector<int>& pV_indices,
    const std::vector<int>& pVn_indices,
    const std::vector<int>& pVt_indices)
	: vertices(pVertices), normals(pNormals), texcoords(pTexcoords),
	v_indices(pV_indices), vn_indices(pVn_indices), vt_indices(pVt_indices) { }

void MeshBuilder::clear() {
	vertices.clear();
	normals.clear();
	texcoords.clear();
	v_indices.clear();
	vn_indices.clear();
	vt_indices.clear();
}

float MeshBuilder::maxExtent() const {
	float length = std::numeric_limits<float>::max();
	for (size_t i = 0; i < vertices.size(); i++) {
		float testLength = static_cast<float>(vertices[i].length());
		if (testLength > length) length = testLength;
	}
	return length;
}
float MeshBuilder::minExtent() const {
	float length = std::numeric_limits<float>::max();
	for (size_t i = 0; i < vertices.size(); i++) {
		float testLength = static_cast<float>(vertices[i].length());
		if (testLength < length) length = testLength;
	}
	return length;
}

void MeshBuilder::scale(float scale) {
	for (size_t i = 0; i < vertices.size(); i++) {
		vertices[i] *= scale;
	}
}
void MeshBuilder::unitize(float unitScale) {
	scale(unitScale / maxExtent());
}

std::vector<Vertex> MeshBuilder::toVertexArray() {
	float scaleModif = 1.0f;
	std::vector<Vertex> vertexData(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++) {
		vertexData[i] = Vertex(
			vertices[i].x * scaleModif,
			vertices[i].y * scaleModif,
			vertices[i].z * scaleModif,
			normals[i].x,
			normals[i].y,
			normals[i].z,
			texcoords[i].x,
			texcoords[i].y
		);
	}
	return vertexData;
}
std::vector<PointVertex> MeshBuilder::toPointVertexArray() {
	float scaleModif = 1.0f;
	std::vector<PointVertex> vertexData(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++) {
		vertexData[i] = PointVertex(
			vertices[i].x * scaleModif,
			vertices[i].y * scaleModif,
			vertices[i].z * scaleModif
		);
	}
	return vertexData;
}
std::vector<NormalVertex> MeshBuilder::toNormalVertexArray() {
	float scaleModif = 1.0f;
	std::vector<NormalVertex> vertexData(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++) {
		vertexData[i] = NormalVertex(
			vertices[i].x * scaleModif,
			vertices[i].y * scaleModif,
			vertices[i].z * scaleModif,
			normals[i].x,
			normals[i].y,
			normals[i].z
		);
	}
	return vertexData;
}

std::vector<Vertex> MeshBuilder::toVertexArrayIndexed() {
	float scaleModif = 1.0;
    std::vector<Vertex> vertexData(v_indices.size());
    for (size_t i = 0; i < v_indices.size(); i++) {
        vertexData[i] = Vertex(
            vertices[v_indices[i]-1].x * scaleModif,
            vertices[v_indices[i]-1].y * scaleModif,
            vertices[v_indices[i]-1].z * scaleModif,
            normals[vn_indices[i]-1].x,
			normals[vn_indices[i]-1].y,
			normals[vn_indices[i]-1].z,
            texcoords[vt_indices[i]-1].x,
			texcoords[vt_indices[i]-1].y
		);
    }
	return vertexData;
}


std::vector<PointVertex> MeshBuilder::toPointVertexArrayIndexed() {
	float scaleModif = 1.0;
	std::vector<PointVertex> vertexData(v_indices.size());
	for (size_t i = 0; i < v_indices.size(); i++) {
		vertexData[i] = PointVertex(
			vertices[v_indices[i]-1].x * scaleModif,
			vertices[v_indices[i]-1].y * scaleModif,
			vertices[v_indices[i]-1].z * scaleModif
		);
	}
	return vertexData;
}
std::vector<NormalVertex> MeshBuilder::toNormalVertexArrayIndexed() {
	float scaleModif = 1.0;
	std::vector<NormalVertex> vertexData(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++) {
		vertexData[i] = NormalVertex(
            vertices[v_indices[i]-1].x * scaleModif,
            vertices[v_indices[i]-1].y * scaleModif,
            vertices[v_indices[i]-1].z * scaleModif,
            normals[vn_indices[i]-1].x,
			normals[vn_indices[i]-1].y,
			normals[vn_indices[i]-1].z
		);
	}
	return vertexData;
}

Indice::Indice(int pv, int pt, int pn) {
	this->v = pv;
	this->n = pn;
	this->t = pt;
}

void vertex_cb(void *user_data, float x, float y, float z, float w) {
	MeshBuilder *mesh = reinterpret_cast<MeshBuilder *>(user_data);
	mesh->vertices.push_back(glm::vec3(x, y, z));
	w; // Discard W
}

void normal_cb(void *user_data, float x, float y, float z) {
	MeshBuilder *mesh = reinterpret_cast<MeshBuilder *>(user_data);
	mesh->normals.push_back(glm::vec3(x, y, z));
}

void texcoord_cb(void *user_data, float x, float y, float z) {
	MeshBuilder *mesh = reinterpret_cast<MeshBuilder *>(user_data);
	mesh->texcoords.push_back(glm::vec2(x, y));
	z; // Discard Z
}

void index_cb(void *user_data, tinyobj::index_t *indices, int num_indices) {
	// NOTE: the value of each index is raw value.
	// For example, the application must manually adjust the index with offset
	// (e.g. v_indices.size()) when the value is negative(whic means relative
	// index).
	// Also, the first index starts with 1, not 0.
	// See fixIndex() function in tiny_obj_loader.h for details.
	// Also, 0 is set for the index value which
	// does not exist in .obj
	MeshBuilder *mesh = reinterpret_cast<MeshBuilder *>(user_data);

  	for (int i = 0; i < num_indices; i++) {
		tinyobj::index_t idx = indices[i];
		if (idx.vertex_index != 0) {
		  mesh->v_indices.push_back(idx.vertex_index);
		}
		if (idx.normal_index != 0) {
		  mesh->vn_indices.push_back(idx.normal_index);
		}
		if (idx.texcoord_index != 0) {
		  mesh->vt_indices.push_back(idx.texcoord_index);
		}
	}
}

void usemtl_cb(void *user_data, const char *name, int material_idx) {
  	MeshBuilder *mesh = reinterpret_cast<MeshBuilder*>(user_data);
  	if ((material_idx > -1) && (
		  static_cast<size_t>(material_idx) < mesh->materials.size())) {
  		spdlog::info("usemtl. material id = {}(name = {})", material_idx,
			mesh->materials[static_cast<size_t>(material_idx)].name.c_str());
  	} else {
		spdlog::info("usemtl. name = %s\n", name);
  	}
}

void mtllib_cb(
	void *user_data,
	const tinyobj::material_t *materials, 
	int num_materials)
{
	MeshBuilder *mesh = reinterpret_cast<MeshBuilder *>(user_data);
	spdlog::info("mtllib. # of materials = {}", num_materials);

	for (size_t i = 0; i < static_cast<size_t>(num_materials); i++) {
	  mesh->materials.push_back(materials[i]);
	}
}

void group_cb(void *user_data, const char **names, int num_names) {
	(void)user_data;
	// MyMesh *mesh = reinterpret_cast<MyMesh*>(user_data);
	spdlog::info("group : name = ");
	for (int i = 0; i < num_names; i++) {
	  spdlog::info("  {}\n", names[i]);
	}
}

void object_cb(void *user_data, const char *name) {
	(void)user_data;
	// MyMesh *mesh = reinterpret_cast<MyMesh*>(user_data);
	spdlog::info("object : name = {}", name);
}

void MeshBuilder::setVertices(const std::vector<glm::vec3> &pVertices) { this->vertices = pVertices; }
void MeshBuilder::setNormals(const std::vector<glm::vec3> &pNormals) { this->normals = pNormals; }
void MeshBuilder::setTexCoords(const std::vector<glm::vec2> &pTexCoords) { this->texcoords = pTexCoords; }
void MeshBuilder::setVIndices(const std::vector<int> &pIndices) { this->v_indices = pIndices; }
void MeshBuilder::setVNIndices(const std::vector<int> &pIndices) { this->vn_indices = pIndices; }
void MeshBuilder::setVTIndices(const std::vector<int> &pIndices) { this->vt_indices = pIndices; }

const std::vector<glm::vec3>& MeshBuilder::getVertices() const { return vertices; }
const std::vector<glm::vec3>& MeshBuilder::getNormals() const { return normals; }
const std::vector<glm::vec2>& MeshBuilder::getTextureCoords() const { return texcoords; }
const std::vector<int>& MeshBuilder::getVIndices() const { return v_indices; }
const std::vector<int>& MeshBuilder::getVNIndices() const { return vn_indices; }
const std::vector<int>& MeshBuilder::getVTIndices() const { return vt_indices; }

MeshBuilder MeshBuilder::fromOBJ(const std::string &filename, const std::string &) {
	tinyobj::callback_t cb;
	cb.vertex_cb = vertex_cb;
	cb.normal_cb = normal_cb;
	cb.texcoord_cb = texcoord_cb;
	cb.index_cb = index_cb;
	cb.usemtl_cb = usemtl_cb;
	cb.mtllib_cb = mtllib_cb;
	cb.group_cb = group_cb;
	cb.object_cb = object_cb;

	MeshBuilder mesh;
	std::string warn;
	std::string err;
	std::ifstream ifs(filename.c_str());

	if (ifs.fail()) {
	  throw std::runtime_error("File not found");
	}

	tinyobj::MaterialFileReader mtlReader("../../models/");
	bool ret = tinyobj::LoadObjWithCallback(ifs, cb, &mesh, &mtlReader, &warn, &err);

	if (!warn.empty()) {
	  std::cout << "WARN: " << warn << std::endl;
	}
	if (!err.empty()) {
	  std::cerr << err << std::endl;
	}

	if (!ret) {
	  throw std::runtime_error("Failed to parse .obj");
	}

	spdlog::info("# of vertices         = {}", mesh.vertices.size() / 3);
	spdlog::info("# of normals          = {}", mesh.normals.size() / 3);
	spdlog::info("# of texcoords        = {}", mesh.texcoords.size() / 2);
	spdlog::info("# of vertex indices   = {}", mesh.v_indices.size());
	spdlog::info("# of normal indices   = {}", mesh.vn_indices.size());
	spdlog::info("# of texcoord indices = {}", mesh.vt_indices.size());
	spdlog::info("# of materials        = {}", mesh.materials.size());

	return mesh;
}

MeshBuilder lt::resource::loadCube() {
	MeshBuilder mesh;
	std::vector<glm::vec3> vertex_buffer = {
	    glm::vec3(-1.0f,-1.0f,-1.0f),
	    glm::vec3(-1.0f,-1.0f, 1.0f),
	    glm::vec3(-1.0f, 1.0f, 1.0f),
	    glm::vec3(1.0f, 1.0f,-1.0f),
	    glm::vec3(-1.0f,-1.0f,-1.0f),
	    glm::vec3(-1.0f, 1.0f,-1.0f),
	    glm::vec3(1.0f,-1.0f, 1.0f),
	    glm::vec3(-1.0f,-1.0f,-1.0f),
	    glm::vec3(1.0f,-1.0f,-1.0f),
	    glm::vec3(1.0f, 1.0f,-1.0f),
	    glm::vec3(1.0f,-1.0f,-1.0f),
	    glm::vec3(-1.0f,-1.0f,-1.0f),
	    glm::vec3(-1.0f,-1.0f,-1.0f),
	    glm::vec3(-1.0f, 1.0f, 1.0f),
	    glm::vec3(-1.0f, 1.0f,-1.0f),
	    glm::vec3(1.0f,-1.0f, 1.0f),
	    glm::vec3(-1.0f,-1.0f, 1.0f),
	    glm::vec3(-1.0f,-1.0f,-1.0f),
	    glm::vec3(-1.0f, 1.0f, 1.0f),
	    glm::vec3(-1.0f,-1.0f, 1.0f),
	    glm::vec3(1.0f,-1.0f, 1.0f),
	    glm::vec3(1.0f, 1.0f, 1.0f),
	    glm::vec3(1.0f,-1.0f,-1.0f),
	    glm::vec3(1.0f, 1.0f,-1.0f),
	    glm::vec3(1.0f,-1.0f,-1.0f),
	    glm::vec3(1.0f, 1.0f, 1.0f),
	    glm::vec3(1.0f,-1.0f, 1.0f),
	    glm::vec3(1.0f, 1.0f, 1.0f),
	    glm::vec3(1.0f, 1.0f,-1.0f),
	    glm::vec3(-1.0f, 1.0f,-1.0f),
	    glm::vec3(1.0f, 1.0f, 1.0f),
	    glm::vec3(-1.0f, 1.0f,-1.0f),
	    glm::vec3(-1.0f, 1.0f, 1.0f),
	    glm::vec3(1.0f, 1.0f, 1.0f),
	    glm::vec3(-1.0f, 1.0f, 1.0f),
	    glm::vec3(1.0f,-1.0f, 1.0f),
	};
	mesh.vertices = vertex_buffer;
	mesh.normals.resize(mesh.vertices.size());
	mesh.texcoords.resize(mesh.vertices.size());
	return mesh;
}

MeshBuilder lt::resource::loadTriangle() {
	MeshBuilder mesh;
	mesh.vertices = { { -1.0f, -1.0f, 0.0f },
		{ 1.0f, -1.0f, 0.0f }, { 0.0f,  1.0f, 0.0f } };
	mesh.normals = { { 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } };
	mesh.texcoords = {{0.0, 0.0}, {1.0, 0.0}, {0.5, 1.0}};
	mesh.v_indices = { 0, 1, 2 };
	mesh.vn_indices = { 0, 1, 2 };
	mesh.vt_indices = { 0, 1, 2 };
	return mesh;
}

MeshBuilder2D lt::resource::loadTriangle2D() {
	MeshBuilder2D mesh;
	mesh.vertices = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {0.0f, 1.0f}};
	mesh.texCoords = {{0.0, 0.0}, {1.0f, 0.0f}, {0.5, 1.0}};
	mesh.v_indices = { 0, 1, 2 };
	mesh.vt_indices = { 0, 1, 2 };
	return mesh;
}

MeshBuilder2D lt::resource::loadRect2D() {
	MeshBuilder2D mesh;
	mesh.vertices = {{-1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f},
		{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}};
	mesh.texCoords = { {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}};
	mesh.v_indices = { 0, 1, 2, 3, 4, 5 };
	mesh.vt_indices = { 0, 1, 2, 3, 4, 5 };
	return mesh;
}

LTImage::LTImage(size_t width, size_t height, std::vector<char> &&data) {
	this->width = width;
	this->height = height;
	this->data = std::move(data);
}

size_t LTImage::getWidth() const { return width; }
size_t LTImage::getHeight() const { return height; }
size_t LTImage::getSize() const { return width * height; }
char* LTImage::getData() { return data.data(); }
const char* LTImage::getData() const { return data.data(); }

void MeshBuilder2D::Exporter2D::addVec2(ExportFile2D& expFile, glm::vec2 vec) const
{
	expFile.data.push_back(vec.x);
	expFile.data.push_back(vec.y);
}

void MeshBuilder2D::Exporter2D::addVec3(ExportFile2D& expFile, glm::vec3 vec) const
{
	expFile.data.push_back(vec.x);
	expFile.data.push_back(vec.y);
	expFile.data.push_back(vec.z);
}

MeshBuilder2D::Exporter2D::Exporter2D(MeshBuilder2D* builder)
	: builder(builder) { }

MeshBuilder2D::ExportFile2D MeshBuilder2D::Exporter2D::exportData() const
{
	ExportFile2D expFile;
	for (ExportType type : exp) {
		ExportMacro macro;
		macro.type = type;
		switch(type) {
			case VERTEX: macro.size = 2; break;
			case TEXTURE: macro.size = 2; break;
			case COLOR: macro.size = 3; break;
		}
		expFile.exp.push_back(macro);
	}
	
	for (size_t i = 0; ; i++) {
		for (ExportType type : exp) {
			switch (type) {
				case VERTEX:
					if (i >= builder->vertices.size()) return expFile;
					addVec2(expFile, builder->vertices[i]); break;
				case TEXTURE:
					if (i >= builder->texCoords.size()) return expFile;
					addVec2(expFile, builder->texCoords[i]); break;
				case COLOR:
					if (i >= builder->colors.size()) return expFile;
					addVec3(expFile, builder->colors[i]); break;
			}
		}
	}
	return expFile;
}
