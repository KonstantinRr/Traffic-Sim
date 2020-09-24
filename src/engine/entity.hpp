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

#ifndef ENTITY_H
#define ENTITY_H

#include "com.h"

#include <glm/glm.hpp>
#include <memory>
#include <functional>

#include "glmodel.hpp"

namespace lt
{
    class Entity {
    public:
        Entity() = default;
    
    
        Entity(int id,
            const std::shared_ptr<GLTexturedModel> &model,
            const glm::vec3 &position = glm::vec3(0.0, 0.0, 0.0),
            const glm::vec3 &rotation = glm::vec3(0.0, 0.0, 0.0),
            const glm::vec3 &scale = glm::vec3(1.0, 1.0, 1.0));
        virtual ~Entity() = default;

        virtual Entity& move(const glm::vec3 &operation);
    
        virtual Entity& scale(const glm::vec3 &scale);
        virtual Entity& scale(float scale);

        virtual Entity& rotate(const glm::vec3 &rotation);
        virtual Entity& rotateX(float angle);
        virtual Entity& rotateY(float angle);
        virtual Entity& rotateZ(float angle);

        virtual Entity& setID(int id);
        virtual Entity& setPosition(const glm::vec3 &position);
        virtual Entity& setRotation(const glm::vec3 &rotation);
        virtual Entity& setScale(const glm::vec3 &scale);
        virtual Entity& setScale(float scale);

        int getID() const;
        const glm::vec3& getPosition() const;
        const glm::vec3& getRotation() const;
        const glm::vec3& getScale() const;

        const std::shared_ptr<GLTexturedModel>& getTexture() const;

        glm::mat4x4 calculateTransformationMatrix() const;
        glm::mat3x3 calculateNormalMatrix() const;

        virtual glm::mat4x4 getTransformationMatrix() const;
        virtual glm::mat3x3 getNormalMatrix() const;

    protected:
        int id;
        glm::vec3 entityPosition;
        glm::vec3 entityRotation, entityScale;
        std::shared_ptr<GLTexturedModel> model;
    };

    //// ---- MatrixBufferedEntity ---- ////

    class MatrixBufferedEntity : public Entity {
    protected:
        glm::mat4x4 transformationMatrix;
        glm::mat3x3 normalMatrix;
        bool hasTransformChange;

    public:
        MatrixBufferedEntity() = default;
        MatrixBufferedEntity(int id,
            const std::shared_ptr<GLTexturedModel> &model,
            const glm::vec3 &position = glm::vec3(0.0, 0.0, 0.0),
            const glm::vec3 &rotation = glm::vec3(0.0, 0.0, 0.0),
            const glm::vec3 &scale = glm::vec3(1.0, 1.0, 1.0));
        virtual ~MatrixBufferedEntity() = default;

        virtual Entity& move(const glm::vec3 &operation);
    
        virtual Entity& scale(const glm::vec3 &scale);
        virtual Entity& scale(float scale);

        virtual Entity& rotate(const glm::vec3 &rotation);
        virtual Entity& rotateX(float angle);
        virtual Entity& rotateY(float angle);
        virtual Entity& rotateZ(float angle);

        virtual Entity& setPosition(const glm::vec3 &position);
        virtual Entity& setRotation(const glm::vec3 &rotation);
        virtual Entity& setScale(const glm::vec3 &scale);
        virtual Entity& setScale(float scale);

        bool didChangeTransform() const;
        void updateBuffers();
        void rebuildTransform();
        void markChangeTransform(bool value=true);

        virtual glm::mat4x4 getTransformationMatrix() const;
        virtual glm::mat3x3 getNormalMatrix() const;
    };

    //// ---- TransformableEntity2D ---- ////

    class Entity2D {
    public:
        /// <summary>
        /// Creates an Entity2D with an invalid ID and without any model or texture.
        /// This entity will be skipped in rendering.
        /// </summary>
        Entity2D();

        /// <summary>
        /// Creates an Entity2D with an ID as well as an optional model and/or texture.
        /// </summary>
        /// <param name="id">The entitie's unqiue ID</param>
        /// <param name="model">The entitie's model</param>
        /// <param name="texture">The entitie's texture</param>
        Entity2D(int id,
            const std::shared_ptr<GLModel>& model = nullptr,
            const std::shared_ptr<GLTexture2D>& texture = nullptr
        );

        int getID() const;
        void setID(int id);

        const std::shared_ptr<GLTexture2D> getTexture() const;
        const std::shared_ptr<GLModel> getModel() const;
    
        virtual glm::mat4x4 getTransform4D() const = 0;
        virtual glm::mat3x3 getTransformationMatrix() const = 0;

    protected:
        int id;
        std::shared_ptr<GLModel> model;
        std::shared_ptr<GLTexture2D> texture;
    };

    class TransformedEntity2D : public Entity2D {
    public:
        TransformedEntity2D(int id,
            const std::shared_ptr<GLModel>& model = nullptr,
            const std::shared_ptr<GLTexture2D>& texture = nullptr,
            const glm::mat3x3 &transformation = glm::mat3(1.0f));

        void setTransformationMatrix(const glm::mat3x3 &mat);

        virtual glm::mat4x4 getTransform4D() const;
        virtual glm::mat3x3 getTransformationMatrix() const;
    protected:
        glm::mat3x3 transform;
    };

    class Transformed4DEntity2D : public Entity2D {
    protected:
        glm::mat4x4 transform;
    public:
        Transformed4DEntity2D(int id,
            const std::shared_ptr<GLModel>& model = nullptr,
            const std::shared_ptr<GLTexture2D>& texture = nullptr,
            const glm::mat4x4 &transformation = glm::mat3(1.0f));

        void setTransform4D(const glm::mat4x4 &mat);

        virtual glm::mat4x4 getTransform4D() const;
        virtual glm::mat3x3 getTransformationMatrix() const;
    };

    class TransformableEntity2D : public Entity2D {
    protected:
        glm::vec2 entityPosition;
        glm::vec2 entityScale;
        float entityRotation;

    public:
        TransformableEntity2D();
        TransformableEntity2D(
            int id,
            const std::shared_ptr<GLModel> &model = nullptr,
            const std::shared_ptr<GLTexture2D> &texture = nullptr,
            const glm::vec2 &position=glm::vec2(0.0f, 0.0f),
            const glm::vec2 &scale=glm::vec2(0.0f, 0.0f),
            float rotation=0.0f);
        virtual ~TransformableEntity2D() = default;

        virtual TransformableEntity2D& setPosition(const glm::vec2 &position);
        virtual TransformableEntity2D& setScale(const glm::vec2 &scale);
        virtual TransformableEntity2D& setRotation(float rotation);

        virtual TransformableEntity2D& move(const glm::vec2 &argument);
        virtual TransformableEntity2D& scale(const glm::vec2 &scale);
        virtual TransformableEntity2D& scale(float scale);
        virtual TransformableEntity2D& rotate(float rotation);
    
        const glm::vec2& getPosition() const;
        const glm::vec2& getScale() const;
        float getRotation() const;

        glm::mat3x3 calculateTransformationMatrix() const;
        glm::mat3x3 calculateTransformationMatrix4D() const;

        virtual glm::mat4x4 getTransform4D() const;
        virtual glm::mat3x3 getTransformationMatrix() const;
    };

    //// ---- MatrixBufferedEntity2D ---- ////
    class MatrixBufferedEntity2D : public TransformableEntity2D {
    protected:
        bool hasTransformChange;
        glm::mat3x3 transform;
    
    public:
        MatrixBufferedEntity2D() = default;
        MatrixBufferedEntity2D(int id,
            const std::shared_ptr<GLModel> &model = nullptr,
            const std::shared_ptr<GLTexture2D> &texture = nullptr,
            const glm::vec2 &position = glm::vec2(0.0f, 0.0f),
            const glm::vec2 &scale = glm::vec2(1.0f, 1.0f),
            float rotation = 0.0f);
    
        virtual TransformableEntity2D& setPosition(const glm::vec2 &position);
        virtual TransformableEntity2D& setScale(const glm::vec2 &scale);
        virtual TransformableEntity2D& setRotation(float rotation);

        virtual TransformableEntity2D& move(const glm::vec2 &argument);
        virtual TransformableEntity2D& scale(const glm::vec2 &scale);
        virtual TransformableEntity2D& scale(float scale);
        virtual TransformableEntity2D& rotate(float rotation);

        bool didChangeTransform() const;
        void updateBuffers();
        void rebuildTransform();
        void markChangeTransform(bool value=true);

        virtual glm::mat3x3 getTransformationMatrix() const;
    };

    class Tickable {
    public:
        virtual ~Tickable() = default;
        virtual void update(float t, float dt) = 0;
    };

    class TickableLambdaEntity : public Entity, public Tickable {
    protected:
        std::function<void(float, float, Entity&)> updateFunction;
    public:
        TickableLambdaEntity() = default;

        template<typename ftype>
        TickableLambdaEntity(
                int id,
                const std::shared_ptr<GLTexturedModel> &model,
                const ftype& updateFunction,
                const glm::vec3 &position = glm::vec3(0.0, 0.0, 0.0),
                const glm::vec3 &rotation = glm::vec3(0.0, 0.0, 0.0),
                const glm::vec3 &scale = glm::vec3(1.0, 1.0, 1.0))
            : Entity(id, model, position, rotation, scale) {
            // wraps the lambda call in a std::function
            this->updateFunction = std::function<
                    void(float, float, Entity&)>(updateFunction);
        }

        virtual void update(float t, float dt);
    };

}
#endif // ENTITY_H
