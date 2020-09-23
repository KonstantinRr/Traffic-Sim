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

#include "entity.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtc/matrix_inverse.hpp>

// ---- Entity ---- //

Entity::Entity(int id,
    const std::shared_ptr<GLTexturedModel> &model,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale) {
    this->id = id;
    this->model = model;
    this->entityPosition = position;
    this->entityRotation = rotation;
    this->entityScale = scale;
}

Entity& Entity::move(const glm::vec3 &operation) {
    entityPosition += operation;
    return *this;
}
Entity& Entity::scale(const glm::vec3 &operation) {
    entityScale *= operation;
    return *this;
}
Entity& Entity::scale(float scale) {
    entityScale *= scale;
    return *this;
}
Entity& Entity::rotate(const glm::vec3 &operation) {
    entityRotation += operation;
    return *this;
}

Entity& Entity::rotateX(float angle) { entityRotation[0] += angle; return *this; }
Entity& Entity::rotateY(float angle) { entityRotation[1] += angle; return *this; }
Entity& Entity::rotateZ(float angle) { entityRotation[2] += angle; return *this; }

Entity& Entity::setID(int pID) {
    this->id = pID;
    return *this;
}
Entity& Entity::setPosition(const glm::vec3 &position) {
    entityPosition = position;
    return *this;
}
Entity& Entity::setRotation(const glm::vec3 &rotation) {
    entityRotation = rotation;
    return *this;
}
Entity& Entity::setScale(const glm::vec3 &scale) {
    entityScale = scale;
    return *this;
}
Entity& Entity::setScale(float scale) {
    entityScale = {scale, scale, scale};
    return *this;
}

int Entity::getID() const { return id; }
const glm::vec3& Entity::getPosition() const { return entityPosition; }
const glm::vec3& Entity::getRotation() const { return entityRotation; }
const glm::vec3& Entity::getScale() const { return entityScale; }
const std::shared_ptr<GLTexturedModel>& Entity::getTexture() const { return model; }

glm::mat4x4 Entity::calculateTransformationMatrix() const {
    glm::mat4x4 meshTransform(1.0f);
    meshTransform = glm::translate(meshTransform, entityPosition);

    meshTransform = glm::rotate(meshTransform, entityRotation.x, {1.0F, 0.0F, 0.0F});
    meshTransform = glm::rotate(meshTransform, entityRotation.y, {0.0F, 1.0F, 0.0F});
    meshTransform = glm::rotate(meshTransform, entityRotation.z, {0.0F, 0.0F, 1.0F});

    meshTransform = glm::scale(meshTransform, entityScale);
    return meshTransform;
}

glm::mat3x3 Entity::calculateNormalMatrix() const {
    glm::mat3x3 mat(calculateTransformationMatrix());
    return glm::inverseTranspose(mat);
}

glm::mat4x4 Entity::getTransformationMatrix() const { return calculateTransformationMatrix(); }
glm::mat3x3 Entity::getNormalMatrix() const { return calculateNormalMatrix(); }

//// ---- MatrixBufferedEntity ---- ////
MatrixBufferedEntity::MatrixBufferedEntity(int id,
    const std::shared_ptr<GLTexturedModel> &model,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale)
    : Entity(id, model, position, rotation, scale) { }

Entity& MatrixBufferedEntity::move(const glm::vec3 &operation) { hasTransformChange = true; return Entity::move(operation);}

Entity& MatrixBufferedEntity::scale(const glm::vec3 &scale) { hasTransformChange = true; return Entity::scale(scale); }
Entity& MatrixBufferedEntity::scale(float scale) { hasTransformChange = true; return Entity::scale(scale); }

Entity& MatrixBufferedEntity::rotate(const glm::vec3 &rotation) { hasTransformChange = true; return Entity::rotate(rotation); }
Entity& MatrixBufferedEntity::rotateX(float angle) { hasTransformChange = true; return Entity::rotateX(angle); }
Entity& MatrixBufferedEntity::rotateY(float angle) { hasTransformChange = true; return Entity::rotateY(angle); }
Entity& MatrixBufferedEntity::rotateZ(float angle) { hasTransformChange = true; return Entity::rotateZ(angle); }

Entity& MatrixBufferedEntity::setPosition(const glm::vec3 &position) { hasTransformChange = true; return Entity::setPosition(position); }
Entity& MatrixBufferedEntity::setRotation(const glm::vec3 &rotation) { hasTransformChange = true; return Entity::setRotation(rotation); }
Entity& MatrixBufferedEntity::setScale(const glm::vec3 &scale) { hasTransformChange = true; return Entity::setScale(scale); }
Entity& MatrixBufferedEntity::setScale(float scale) { hasTransformChange = true; return Entity::setScale(scale); }

glm::mat4x4 MatrixBufferedEntity::getTransformationMatrix() const { return transformationMatrix; }
glm::mat3x3 MatrixBufferedEntity::getNormalMatrix() const { return normalMatrix; }

bool MatrixBufferedEntity::didChangeTransform() const { return hasTransformChange; }
void MatrixBufferedEntity::updateBuffers() {
    if (didChangeTransform()) rebuildTransform();
}
void MatrixBufferedEntity::rebuildTransform() {
    if (hasTransformChange) {
        transformationMatrix = calculateTransformationMatrix();
        normalMatrix = calculateNormalMatrix();
        hasTransformChange = false;
    }
}
void MatrixBufferedEntity::markChangeTransform(bool value) {
    hasTransformChange = value;
}

Entity2D::Entity2D() : id(-1) { }

Entity2D::Entity2D(int id,
    const std::shared_ptr<GLModel>& model,
    const std::shared_ptr<GLTexture2D>& texture)
    : id(id), model(model), texture(texture) { }


const std::shared_ptr<GLTexture2D> Entity2D::getTexture() const { return texture; }
const std::shared_ptr<GLModel> Entity2D::getModel() const { return model; }

int Entity2D::getID() const { return id; }
void Entity2D::setID(int pID) { this->id = pID; }

// ---- TransformedEntity2D ---- //
TransformedEntity2D::TransformedEntity2D(int id,
    const std::shared_ptr<GLModel>& model,
    const std::shared_ptr<GLTexture2D>& texture,
    const glm::mat3x3 &transform)
    : Entity2D(id, model, texture), transform(transform) { }

void TransformedEntity2D::setTransformationMatrix(const glm::mat3x3& mat) { transform = mat; }

glm::mat4x4 TransformedEntity2D::getTransform4D() const { return glm::mat4(1.0f); }
glm::mat3x3 TransformedEntity2D::getTransformationMatrix() const { return transform; }

// ---- Transformed4DEntity2D ---- //
Transformed4DEntity2D::Transformed4DEntity2D(int id,
    const std::shared_ptr<GLModel>& model,
    const std::shared_ptr<GLTexture2D>& texture,
    const glm::mat4x4& transform)
    : Entity2D(id, model, texture), transform(transform) { }

void Transformed4DEntity2D::setTransform4D(const glm::mat4x4& mat) { transform = mat; }

glm::mat4x4 Transformed4DEntity2D::getTransform4D() const { return transform; }
glm::mat3x3 Transformed4DEntity2D::getTransformationMatrix() const { return glm::mat3(1.0f); }


// ---- TransformableEntity2D ---- //
TransformableEntity2D::TransformableEntity2D() {}
TransformableEntity2D::TransformableEntity2D(
    int id,
    const std::shared_ptr<GLModel> &model,
    const std::shared_ptr<GLTexture2D> &texture,
    const glm::vec2 &position, const glm::vec2 &scale,
    float rotation
) : Entity2D(id, model, texture) {
    this->entityPosition = position;
    this->entityScale = scale;
    this->entityRotation = rotation;
}

TransformableEntity2D& TransformableEntity2D::setPosition(const glm::vec2 &position) { entityPosition = position; return *this; }
TransformableEntity2D& TransformableEntity2D::setScale(const glm::vec2 &scale) { entityScale = scale; return *this; }
TransformableEntity2D& TransformableEntity2D::setRotation(float rotation) { entityRotation = rotation; return *this; }

TransformableEntity2D& TransformableEntity2D::move(const glm::vec2 &argument) { entityPosition += argument; return *this; }
TransformableEntity2D& TransformableEntity2D::scale(const glm::vec2 &scale) { entityScale *= scale; return *this; }
TransformableEntity2D& TransformableEntity2D::scale(float scale) { entityScale *= scale; return *this; }
TransformableEntity2D& TransformableEntity2D::rotate(float rotation) { entityRotation += rotation; return *this; }

const glm::vec2& TransformableEntity2D::getPosition() const { return entityPosition; }
const glm::vec2& TransformableEntity2D::getScale() const { return entityScale; }
float TransformableEntity2D::getRotation() const { return entityRotation; }

glm::mat3x3 TransformableEntity2D::calculateTransformationMatrix() const {
    glm::mat3x3 transform(1.0f);
    transform = glm::translate(transform, entityPosition);
    transform = glm::rotate(transform, entityRotation);
    transform = glm::scale(transform, entityScale);
    return transform;
}

glm::mat3x3 TransformableEntity2D::calculateTransformationMatrix4D() const
{
    glm::mat4x4 transform(1.0f);
    transform = glm::translate(transform, glm::vec3(entityPosition, 1.0f));
    transform = glm::rotate(transform, entityRotation, { 0.0F, 0.0F, 1.0F });
    transform = glm::scale(transform, glm::vec3(entityScale, 1.0f));
    return transform;
}

glm::mat4x4 TransformableEntity2D::getTransform4D() const {
    return calculateTransformationMatrix4D();
}
glm::mat3x3 TransformableEntity2D::getTransformationMatrix() const {
    return calculateTransformationMatrix();
}

// ---- MatrixBufferedEntity2D ---- //
MatrixBufferedEntity2D::MatrixBufferedEntity2D(
    int id,
    const std::shared_ptr<GLModel> &model,
    const std::shared_ptr<GLTexture2D> &texture,
    const glm::vec2 &position,
    const glm::vec2 &scale,
    float rotation) : TransformableEntity2D(
        id, model, texture, position, scale, rotation) { }

TransformableEntity2D& MatrixBufferedEntity2D::setPosition(const glm::vec2 &position) { hasTransformChange = true; return TransformableEntity2D::setPosition(position); }
TransformableEntity2D& MatrixBufferedEntity2D::setScale(const glm::vec2 &scale) { hasTransformChange = true; return TransformableEntity2D::setScale(scale); }
TransformableEntity2D& MatrixBufferedEntity2D::setRotation(float rotation) { hasTransformChange = true; return TransformableEntity2D::setRotation(rotation); }

TransformableEntity2D& MatrixBufferedEntity2D::move(const glm::vec2 &argument) { hasTransformChange = true; return TransformableEntity2D::move(argument); }
TransformableEntity2D& MatrixBufferedEntity2D::scale(const glm::vec2 &scale) { hasTransformChange = true; return TransformableEntity2D::scale(scale); }
TransformableEntity2D& MatrixBufferedEntity2D::scale(float scale) { hasTransformChange = true; return TransformableEntity2D::scale(scale); }
TransformableEntity2D& MatrixBufferedEntity2D::rotate(float rotation) { hasTransformChange = true; return TransformableEntity2D::rotate(rotation); };

bool MatrixBufferedEntity2D::didChangeTransform() const { return hasTransformChange; }
void MatrixBufferedEntity2D::updateBuffers() {
    if (didChangeTransform()) rebuildTransform();
}
void MatrixBufferedEntity2D::rebuildTransform() {
    transform = calculateTransformationMatrix();
    hasTransformChange = false;
}
void MatrixBufferedEntity2D::markChangeTransform(bool value) {
    hasTransformChange = value;
}
glm::mat3x3 MatrixBufferedEntity2D::getTransformationMatrix() const {
    return transform;
}
// ---- TickableLambdaEntity ---- //

void TickableLambdaEntity::update(float t, float dt) {
    updateFunction(t, dt, *this);
}
