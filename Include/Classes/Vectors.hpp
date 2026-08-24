#ifndef BASEVECTOR_CLASS_HPP
#define BASEVECTOR_CLASS_HPP

#include "C:/glm/glm.hpp"
#include <vector>
#include <optional>
#include <cmath>

class BaseVector {
    public:
        
        // Variables
        glm::vec3 Position;
        glm::vec2 PositionScale;
        std::optional<std::vector<float>> TexCoord;
        std::optional<std::vector<float>> Color;
        std::optional<std::vector<float>> InstanceBasedData;
        
        // Initializers
        BaseVector(glm::vec3 InPosition = glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec2 InPositionScale = glm::vec2(0.0f, 0.0f),
                    std::optional<std::vector<float>> InTexCoord = std::nullopt,
                    std::optional<std::vector<float>> InColor = std::vector<float>{1.0f, 1.0f, 1.0f, 1.0f},
                    std::optional<std::vector<float>> InInstanceBasedData = std::nullopt)
            : Position(InPosition)
            , PositionScale(InPositionScale)
            , TexCoord(std::move(InTexCoord))
            , Color(std::move(InColor))
            , InstanceBasedData(std::move(InInstanceBasedData)) {}
        
        // Functions
        std::vector<float> Unpack() {
            std::vector<float> Result;
            Result.insert(Result.end(), {Position.x, Position.y, Position.z});
            Result.insert(Result.end(), {PositionScale.x, PositionScale.y});
            
            if (TexCoord.has_value())
                Result.insert(Result.end(), TexCoord->begin(), TexCoord->end());
            if (Color.has_value())
                Result.insert(Result.end(), Color->begin(), Color->end());
            
            return Result;
        }
        
        // Operators - Add / Subtract
        BaseVector operator+(const BaseVector& Other) const {
            return BaseVector(Position + Other.Position, PositionScale, TexCoord, Color);
        }
        
        BaseVector operator-(const BaseVector& Other) const {
            return BaseVector(Position - Other.Position, PositionScale, TexCoord, Color);
        }
        
        // Operators - Multiply
        float operator*(const BaseVector& Other) const {
            return glm::dot(Position, Other.Position);
        }
        
        BaseVector operator*(float Scalar) const {
            return BaseVector(Position * Scalar, PositionScale, TexCoord, Color);
        }
        
        friend BaseVector operator*(float Scalar, const BaseVector& Vec) {
            return BaseVector(Vec.Position * Scalar, Vec.PositionScale, Vec.TexCoord, Vec.Color);
        }
        
        // Operators - Divide
        BaseVector operator/(const BaseVector& Other) const {
            return BaseVector(Position / Other.Position, PositionScale, TexCoord, Color);
        }
        
        BaseVector operator/(float Scalar) const {
            return BaseVector(Position / Scalar, PositionScale, TexCoord, Color);
        }
        
        // Operators - Modulo
        BaseVector operator%(const BaseVector& Other) const {
            glm::vec3 Result(std::fmod(Position.x, Other.Position.x),
                              std::fmod(Position.y, Other.Position.y),
                              std::fmod(Position.z, Other.Position.z));
            return BaseVector(Result, PositionScale, TexCoord, Color);
        }
        
        BaseVector operator%(float Scalar) const {
            glm::vec3 Result(std::fmod(Position.x, Scalar),
                              std::fmod(Position.y, Scalar),
                              std::fmod(Position.z, Scalar));
            return BaseVector(Result, PositionScale, TexCoord, Color);
        }
        
        // Operators - Compound Assignment
        BaseVector& operator+=(const BaseVector& Other) {
            Position += Other.Position;
            return *this;
        }
        
        BaseVector& operator-=(const BaseVector& Other) {
            Position -= Other.Position;
            return *this;
        }
        
        BaseVector& operator*=(const BaseVector& Other) {
            Position = glm::vec3(glm::dot(Position, Other.Position), 0.0f, 0.0f);
            return *this;
        }
        
        BaseVector& operator*=(float Scalar) {
            Position *= Scalar;
            return *this;
        }
        
        BaseVector& operator/=(const BaseVector& Other) {
            Position /= Other.Position;
            return *this;
        }
        
        BaseVector& operator/=(float Scalar) {
            Position /= Scalar;
            return *this;
        }
        
        BaseVector& operator%=(const BaseVector& Other) {
            Position = glm::vec3(std::fmod(Position.x, Other.Position.x),
                                  std::fmod(Position.y, Other.Position.y),
                                  std::fmod(Position.z, Other.Position.z));
            return *this;
        }
        
};

#endif