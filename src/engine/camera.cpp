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
#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace lt::render;

Camera::Camera(float pNearPlane, float pFarPlane, float pFOV, float pAspectRatio) {
    this->nearPlane = pNearPlane;
    this->farPlane = pFarPlane;
    this->fov = pFOV;
    this->aspectRatio = pAspectRatio;
    this->position = glm::vec3(0.0f, 0.0f, 0.0f);
    this->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
}

Camera::Camera(
    float pNearPlane, float pFarPlane, float pFOV, float pAspectRatio,
    const glm::vec3 &pPosition,
    const glm::vec3 &pRotation
) {
    this->position = pPosition;
    this->rotation = pRotation;
    this->nearPlane = pNearPlane;
    this->farPlane = pFarPlane;
    this->fov = pFOV;
    this->aspectRatio = pAspectRatio;
}

Camera::Camera(
    float pNearPlane, float pFarPlane, float pFOV, float pAspectRatio,
    const glm::vec3 &pPosition,
    float pRoll, float pPitch, float pYaw) {
    this->position = pPosition;
    this->rotation = glm::vec3(pRoll, pPitch, pYaw);
    this->nearPlane = pNearPlane;
    this->farPlane = pFarPlane;
    this->fov = pFOV;
    this->aspectRatio = pAspectRatio;
}

glm::vec3 Camera::getViewDirection() const {
    return glm::vec3(
        -std::cos(getYaw()),
        std::sin(getPitch()),
        -std::sin(getPitch()));
}

glm::vec3 Camera::getViewCrossDirection() const {
    glm::mat4x4 matrix(1.0f);
    matrix = glm::rotate(matrix, getRoll(), getViewDirection());
    glm::vec4 vector(
        -std::cos(getYaw()),
         std::sin(getPitch()),
        -std::sin(getPitch()),
        0.0);
    return glm::vec3(matrix * vector);
}

Camera& Camera::setNearPlane(float pNearPlane) {
    this->nearPlane = pNearPlane;
    return *this;
}
Camera& Camera::setFarPlane(float pFarPlane) {
    this->farPlane = pFarPlane;
    return *this;
}
Camera& Camera::setFOV(float pFOV) {
    this->fov = pFOV;
    return *this;
}
Camera& Camera::setAspectRatio(float pAspect) {
    this->aspectRatio = pAspect;
    return *this;
}
Camera& Camera::setAspectRatio(int width, int height) {
    this->aspectRatio = static_cast<float>(width)
            / static_cast<float>(height);
    return *this;
}
Camera& Camera::setRoll(float roll) { rotation[0] = roll; return *this; }
Camera& Camera::setPitch(float pitch) { rotation[1] = pitch; return *this; }
Camera& Camera::setYaw(float yaw) { rotation[2] = yaw; return *this; }
Camera& Camera::changeRoll(float roll) { rotation[0] += roll; return *this; }
Camera& Camera::changePitch(float pitch) { rotation[1] += pitch; return *this; }
Camera& Camera::changeYaw(float yaw) { rotation[2] += yaw; return *this; }

float Camera::getNearPlane() const { return nearPlane; }
float Camera::getFarPlane() const { return farPlane; }
float Camera::getFOV() const { return fov; }
float Camera::getAspectRatio() const { return aspectRatio; }

float Camera::getRoll() const { return rotation[0]; }
float Camera::getPitch() const { return rotation[1]; }
float Camera::getYaw() const { return rotation[2]; }
glm::vec3 Camera::getRotation() const { return rotation; }

float Camera::getX() const { return position[0]; }
float Camera::getY() const { return position[1]; }
float Camera::getZ() const { return position[2]; }
glm::vec3 Camera::getPosition() const { return position; }

Camera& Camera::rotate(const glm::vec3 &protation) { rotation += protation; return *this;}
Camera& Camera::move(const glm::vec3 &pposition) { position += pposition; return *this; }

Camera& Camera::setRotation(const glm::vec3 &protation) { rotation = protation; return *this; }
Camera& Camera::setPosition(const glm::vec3 &pposition) { position = pposition; return *this; }

Camera& Camera::setX(float x) { position[0] = x; return *this; }
Camera& Camera::setY(float y) { position[1] = y; return *this; }
Camera& Camera::setZ(float z) { position[2] = z; return *this; }

glm::mat4x4 Camera::getViewMatrix() const { return calculateViewMatrix(); }
glm::mat4x4 Camera::getProjectionMatrix() const { return calculateProjectionMatrix(); }

glm::mat4x4 Camera::calculateViewMatrix() const{
    //roll can be removed from here. because is not actually used in FPS camera
    glm::mat4x4 matRoll(1.0f);
    glm::mat4x4 matPitch(1.0f);
    glm::mat4x4 matYaw(1.0f);

    //roll, pitch and yaw are used to store our angles in our class
    matRoll = glm::rotate(matRoll, getRoll(), glm::vec3(0.0f, 0.0f, 1.0f));
    matPitch = glm::rotate(matPitch, getPitch(), glm::vec3(1.0f, 0.0f, 0.0f));
    matYaw = glm::rotate(matYaw, getYaw(), glm::vec3(0.0f, 1.0f, 0.0f));

    //order matters
    glm::mat4x4 rotate = matRoll * matPitch * matYaw;

    glm::mat4x4 translate(1.0f);
    translate = glm::translate(translate, -position);
    return rotate * translate;
}

glm::mat4x4 Camera::calculateProjectionMatrix() const{
    return glm::perspective(fov, aspectRatio, nearPlane, farPlane);
}

//// ---- MatrixBufferedCamera ---- ////

MatrixBufferedCamera::MatrixBufferedCamera(float pNearPlane, float pFarPlane, float pFOV, float pAspectRatio)
    : Camera(pNearPlane, pFarPlane, pFOV, pAspectRatio) { }
MatrixBufferedCamera::MatrixBufferedCamera(float pNearPlane, float pFarPlane, float pFOV, float pAspectRatio,
    const glm::vec3 &pPosition, const glm::vec3 &pRotation)
    : Camera(pNearPlane, pFarPlane, pFOV, pAspectRatio, pPosition, pRotation) { }
MatrixBufferedCamera::MatrixBufferedCamera(float pNearPlane, float pFarPlane, float pFOV, float pAspectRatio,
    const glm::vec3 &pPosition, float pRoll, float pPitch, float pYaw)
    : Camera(pNearPlane, pFarPlane, pFOV, pAspectRatio, pPosition, pRoll, pPitch, pYaw) { }

Camera& MatrixBufferedCamera::setNearPlane(float pNearPlane) { hasProjectionChange = true; return Camera::setNearPlane(pNearPlane);}
Camera& MatrixBufferedCamera::setFarPlane(float pFarPlane) { hasProjectionChange = true; return Camera::setFarPlane(pFarPlane); }
Camera& MatrixBufferedCamera::setFOV(float pFOV) { hasProjectionChange = true; return Camera::setFOV(pFOV); }
Camera& MatrixBufferedCamera::setAspectRatio(float pAspect) { hasProjectionChange = true; return Camera::setAspectRatio(pAspect); }
Camera& MatrixBufferedCamera::setAspectRatio(int pWidth, int pHeight) { hasProjectionChange = true; return Camera::setAspectRatio(pWidth, pHeight); }

Camera& MatrixBufferedCamera::setRoll(float pRoll) { hasViewChange = true; return Camera::setRoll(pRoll); }
Camera& MatrixBufferedCamera::setPitch(float pPitch) { hasViewChange = true; return Camera::setPitch(pPitch); }
Camera& MatrixBufferedCamera::setYaw(float pYaw) { hasViewChange = true; return Camera::setYaw(pYaw); }

Camera& MatrixBufferedCamera::changeRoll(float pRoll) { hasViewChange = true; return Camera::changeRoll(pRoll); }
Camera& MatrixBufferedCamera::changePitch(float pPitch) { hasViewChange = true; return Camera::changePitch(pPitch); }
Camera& MatrixBufferedCamera::changeYaw(float pYaw) { hasViewChange = true; return Camera::changeYaw(pYaw); }

Camera& MatrixBufferedCamera::rotate(const glm::vec3 &pRotation) { hasViewChange = true; return Camera::rotate(pRotation); }
Camera& MatrixBufferedCamera::move(const glm::vec3 &pPosition) { hasViewChange = true; return Camera::move(pPosition); }
Camera& MatrixBufferedCamera::setRotation(const glm::vec3 &pRotation) { hasViewChange = true; return Camera::setRotation(pRotation); }
Camera& MatrixBufferedCamera::setX(float pX) { hasViewChange = true; return Camera::setX(pX); }
Camera& MatrixBufferedCamera::setY(float pY) { hasViewChange = true; return Camera::setY(pY); }
Camera& MatrixBufferedCamera::setZ(float pZ) { hasViewChange = true; return Camera::setZ(pZ); }
Camera& MatrixBufferedCamera::setPosition(const glm::vec3 &pPosition) { hasViewChange = true; return Camera::setPosition(pPosition); }

bool MatrixBufferedCamera::didViewChange() const { return hasViewChange;}
bool MatrixBufferedCamera::didProjectionChange() const { return hasProjectionChange; }

void MatrixBufferedCamera::updateBuffers() {
    if (didProjectionChange()) rebuildProjection();
    if (didViewChange()) rebuildView();
}

void MatrixBufferedCamera::rebuildProjection() {
    projectionMatrix = calculateProjectionMatrix();
    hasProjectionChange = false;
}
void MatrixBufferedCamera::rebuildView() {
    viewMatrix = calculateViewMatrix();
    hasViewChange = false;
}

void MatrixBufferedCamera::markRebuildProjection(bool value) { hasProjectionChange = value; }
void MatrixBufferedCamera::markRebuildView(bool value) { hasViewChange = value; }

glm::mat4x4 MatrixBufferedCamera::getViewMatrix() const { return viewMatrix; }
glm::mat4x4 MatrixBufferedCamera::getProjectionMatrix() const { return projectionMatrix; }