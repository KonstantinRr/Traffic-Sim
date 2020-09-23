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

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

namespace lt {
namespace render {

/// <summary>
/// A three dimensional camera that can be used to manipulate the view position, angle, fov, aspect ratio
/// as well as the near and far plane. The camera settings can be exported as a 4x4 matrix giving the transformation.
/// </summary>
class Camera {
protected:
    float nearPlane, farPlane, fov, aspectRatio;
    glm::vec3 position, rotation;

public:
    /// Creates a camera using the neccessary render settings.
    /// Position and rotation is set to the default value 0.
    /// nearPlane:      The camera's near plane
    /// farPlane:       The camera's far plane
    /// fov:            The field of view angle in radians
    /// aspectRation:   The viewports's aspect ratio
    Camera(
        float nearPlane, float farPlane,
        float fov, float aspectRatio);

    /// Creates a camera using the neccessary render settings
    /// with a custom position and rotation.
    /// nearPlane:      The camera's near plane
    /// farPlane:       The camera's far plane
    /// fov:            The field of view angle in radians
    /// aspectRation:   The viewport's aspect ratio
    /// position:       The camera's position
    /// rotation:       The camera's rotation
    Camera(
        float nearPlane, float farPlane,
        float fov, float aspectRatio,
        const glm::vec3 &position,
        const glm::vec3 &rotation);
    
    /// Creates a camera using the neccessary render settings
    /// with a custom position and rotation.
    /// nearPlane:      The camera's near plane
    /// farPlane:       The camera's far plane
    /// fov:            The field of view angle in radians
    /// aspectRation:   The viewport's aspect ratio
    /// roll:           The camera's roll angle. Same as rotation[0]
    /// pitch:          The camera's pitch angle. Same as rotation[1]
    /// yaw:            The camera's yaw angle. Same as rotation[2]
    Camera(
        float nearPlane, float farPlane,
        float fov, float aspectRatio,
        const glm::vec3 &position,
        float roll, float pitch, float yaw);
    
    /// Superclasses should always make their destructor virtual
    virtual ~Camera() = default;

    /* Render Parameters */
    /// Allows manipulating the basic render parameters
    virtual Camera& setNearPlane(float nearPlane);
    virtual Camera& setFarPlane(float farPlane);
    virtual Camera& setFOV(float fov);
    virtual Camera& setAspectRatio(float aspect);
    virtual Camera& setAspectRatio(int width, int height);

    /// Requests the basic render parameters
    float getNearPlane() const;
    float getFarPlane() const;
    float getFOV() const;
    float getAspectRatio() const;

    /* Angle Parameters */
    // Allows manipulating the rotation parameters
    virtual Camera& setRoll(float roll);
    virtual Camera& setPitch(float pitch);
    virtual Camera& setYaw(float yaw);
    virtual Camera& changeRoll(float roll);
    virtual Camera& changePitch(float pitch);
    virtual Camera& changeYaw(float yaw);
    virtual Camera& setRotation(const glm::vec3 &rotation);
    virtual Camera& rotate(const glm::vec3 &model);

    // Requests the camera angles
    float getRoll() const;
    float getPitch() const;
    float getYaw() const;
    glm::vec3 getRotation() const;

    /* Position */
    float getX() const;
    float getY() const;
    float getZ() const;
    glm::vec3 getPosition() const;

    // Allows manipulating the position parameters
    virtual Camera& setX(float x);
    virtual Camera& setY(float y);
    virtual Camera& setZ(float z);
    virtual Camera& setPosition(const glm::vec3 &model);
    virtual Camera& move(const glm::vec3 &model);

    /* Functions */

    glm::vec3 getViewDirection() const;
    glm::vec3 getViewCrossDirection() const;

    virtual glm::mat4x4 getViewMatrix() const;
    virtual glm::mat4x4 getProjectionMatrix() const;

    glm::mat4x4 calculateViewMatrix() const;
    glm::mat4x4 calculateProjectionMatrix() const;
};

/// Subclass inheriting from Camera.
/// The camera stores its own transformation matrix
/// that is updated whenever a parameter is changed.
class MatrixBufferedCamera : public Camera {
protected:
    glm::mat4x4 viewMatrix;
    glm::mat4x4 projectionMatrix;
    bool hasViewChange = true;
    bool hasProjectionChange = true;

public:
    /// Creates a buffered camera using the neccessary render settings.
    /// Position and rotation is set to the default value 0.
    /// nearPlane:      The camera's near plane
    /// farPlane:       The camera's far plane
    /// fov:            The field of view angle in radians
    /// aspectRation:   The viewports's aspect ratio
    MatrixBufferedCamera(
        float nearPlane, float farPlane,
        float fov, float aspectRatio);

    /// Creates a buffered camera using the neccessary render settings
    /// with a custom position and rotation.
    /// nearPlane:      The camera's near plane
    /// farPlane:       The camera's far plane
    /// fov:            The field of view angle in radians
    /// aspectRation:   The viewport's aspect ratio
    /// position:       The camera's position
    /// rotation:       The camera's rotation
    MatrixBufferedCamera(
        float nearPlane, float farPlane,
        float fov, float aspectRatio,
        const glm::vec3 &position,
        const glm::vec3 &rotation);

    /// Creates a camera using the neccessary render settings
    /// with a custom position and rotation.
    /// nearPlane:      The camera's near plane
    /// farPlane:       The camera's far plane
    /// fov:            The field of view angle in radians
    /// aspectRation:   The viewport's aspect ratio
    /// roll:           The camera's roll angle. Same as rotation[0]
    /// pitch:          The camera's pitch angle. Same as rotation[1]
    /// yaw:            The camera's yaw angle. Same as rotation[2]
    MatrixBufferedCamera(
        float nearPlane, float farPlane,
        float fov, float aspectRatio,
        const glm::vec3 &position,
        float roll, float pitch, float yaw);

    virtual Camera& setNearPlane(float nearPlane);
    virtual Camera& setFarPlane(float farPlane);
    virtual Camera& setFOV(float fov);
    virtual Camera& setAspectRatio(float aspect);
    virtual Camera& setAspectRatio(int width, int height);

    // Camera angles
    virtual Camera& setRoll(float roll);
    virtual Camera& setPitch(float pitch);
    virtual Camera& setYaw(float yaw);
    virtual Camera& changeRoll(float roll);
    virtual Camera& changePitch(float pitch);
    virtual Camera& changeYaw(float yaw);
    virtual Camera& rotate(const glm::vec3 &model);
    virtual Camera& setRotation(const glm::vec3 &rotation);

    virtual Camera& setX(float x);
    virtual Camera& setY(float y);
    virtual Camera& setZ(float z);
    virtual Camera& move(const glm::vec3 &model);
    virtual Camera& setPosition(const glm::vec3 &position);

    virtual glm::mat4x4 getViewMatrix() const;
    virtual glm::mat4x4 getProjectionMatrix() const;

    void updateBuffers();
    void rebuildProjection();
    void rebuildView();
    void markRebuildProjection(bool value=true);
    void markRebuildView(bool value=true);

    bool didViewChange() const;
    bool didProjectionChange() const;
};

} // namespace render
} // namespace lt

#endif // CAMERA_H
