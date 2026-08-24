#pragma once
#include <vector>
#include <optional>
#include <functional>
#include <limits>
#include <algorithm>

using std::vector, std::optional, std::function;

#include "Classes/GraphicsEngine/VertexArrayObject.hpp"
#include "Classes/GraphicsEngine/VertexBufferObject.hpp"
#include "Classes/GraphicsEngine/ElementArrayBufferObject.hpp"
#include "Classes/GraphicsEngine/TextureClass.hpp"
#include "Classes/GraphicsEngine/ShaderClass.hpp"
#include "Classes/Vectors.hpp"

#include "C:/glm/gtc/type_ptr.hpp"
#include "C:/glm/glm.hpp"
#include "Math.hpp"

#ifndef ENTITIES_HPP
#define ENTITIES_HPP
class BaseEntity {
    public:
        vector<BaseVector> Vertices{};
        optional<vector<int>> Indices{};
        vector<Uniform> UniformIds{};
        double ZIndex = 0.0;
        
        VertexArrayObject VAO;
        VertexBufferObject VBO;
        ElementArrayBufferObject EBO;
        
        Shader ShaderProgram;
        
        virtual void Render(GLuint DrawType);
        virtual void SetDefaultUniforms();
        virtual void SetUniform(const char *Name, std::function<void(GLint)> setter);
        virtual void GetSize(glm::vec2 WindowSize);
        virtual void Generate();
        
        virtual double Rotation() const {
            return _Rotation;
        }
        
        virtual void Rotation(double Value) {
            if (Value == _Rotation) return;
            
            _Rotation = Value;
            RotationMatrix = MathUtils::RotationMatrix(Value);
        }
        
        inline virtual bool operator<(BaseEntity Entity) {
            return ZIndex < Entity.ZIndex;
        }
        
        BaseEntity(Shader ShaderProgram, vector<BaseVector> Vertices = {}, optional<vector<int>> Indices = std::nullopt) : ShaderProgram(ShaderProgram), Vertices(Vertices), Indices(Indices) {}
        virtual ~BaseEntity() = default;
    protected:
        double _Rotation = 0.0;
        glm::mat2 RotationMatrix = MathUtils::RotationMatrix(0.0);
};

class Entity : public BaseEntity {
    public:
        glm::vec4 Color{1, 1, 1, 1};
        glm::vec3 Position{0, 0, 0};
        glm::vec2 PositionScale{0, 0};
        glm::vec2 AnchorPoint{0.5, 0.5};
        glm::vec2 Size{};
        double Scale = 1.0;
        
        void Render(GLuint DrawType) override {
            ShaderProgram.Activate();
            VAO.Bind();
            VBO.Bind();
            if (EBO.ID) EBO.Bind();
            
            for (auto& Uniform : UniformIds) Uniform.Set();
            
            if (EBO.ID) glDrawElements(DrawType, Indices.value().size() , GL_UNSIGNED_INT, nullptr);
            else glDrawArrays(DrawType, 0, Vertices.size());
            
            VAO.Unbind();
        };
        
        void SetDefaultUniforms() override {
            SetUniform("Position", [this](GLint location){glUniform3f(location, Position[0], Position[1], Position[2]);});
            SetUniform("PositionScale", [this](GLint location){glUniform2f(location, PositionScale[0], PositionScale[1]);});
            SetUniform("Scale", [this](GLint location){glUniform1f(location, Scale);});
            SetUniform("Rotation", [this](GLint location){glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(RotationMatrix));});
            SetUniform("Anchor", [this](GLint location){glUniform2f(location, AnchorPoint[0], AnchorPoint[1]);});
            SetUniform("Size", [this](GLint location){glUniform2f(location, Size[0], Size[1]);});
            SetUniform("Color", [this](GLint location){glUniform4f(location, Color[0], Color[1], Color[2], Color[3]);});
        };
        
        void SetUniform(const char *Name, std::function<void(GLint)> Setter) override {
            GLint UniformId = glGetUniformLocation(ShaderProgram.ShaderProgram, Name);
            UniformIds.emplace_back(UniformId, Setter);
        };
        
        void GetSize(glm::vec2 WindowSize) override {
            glm::vec2 MinBound( std::numeric_limits<float>::max());
            glm::vec2 MaxBound(-std::numeric_limits<float>::max());
            
            for (const auto& Vertex : Vertices) {
                glm::vec2 Position = glm::vec2(Vertex.Position.x, Vertex.Position.y) + Vertex.PositionScale * WindowSize;
                MinBound = glm::min(MinBound, Position);
                MaxBound = glm::max(MaxBound, Position);
            }
            Size = MaxBound - MinBound;
        };
        
        void Generate() override {
            VAO.Bind();
            VBO.Bind();
            if (EBO.ID) EBO.Bind();
            
            int Stride = 9 * 4;
            VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, Stride, (void*)0);
            VAO.LinkAttrib(VBO, 1, 2, GL_FLOAT, Stride, (void*)(3 * 4));
            VAO.LinkAttrib(VBO, 2, 4, GL_FLOAT, Stride, (void*)(5 * 4));
            
            VAO.Unbind();
        };
        
        Entity(Shader ShaderProgram, vector<BaseVector> Vertices = {}, optional<vector<int>> Indices = std::nullopt) : BaseEntity(ShaderProgram, Vertices, Indices) {}
};

class TextureEntity : public BaseEntity {
    
};

#endif