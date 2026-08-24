#ifndef SHADER_CLASS_HPP
#define SHADER_CLASS_HPP

#include "C:/glad/glad.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <functional>
#include <stdexcept>
#include <any>

using std::string;
using std::vector;

struct Uniform {
    GLint Location;
    std::function<void(GLint)> Setter;
    
    Uniform(GLint Location, std::function<void(GLint)> Setter) : Location(Location), Setter(Setter) {}
    Uniform() = default;
    
    void Set() const { Setter(Location); }
};

class Shader {
    public:
        // Variables
        GLuint ShaderProgram;
        
        // Initializers
        Shader(string VertexShaderPath, string FragmentShaderPath) {
            std::ifstream VertexFile(VertexShaderPath);
            if (!VertexFile.is_open())
                throw std::runtime_error("Failed to open vertex shader file: " + VertexShaderPath);
            std::stringstream VertexStream;
            VertexStream << VertexFile.rdbuf();
            VertexShaderSource = VertexStream.str();
            
            std::ifstream FragmentFile(FragmentShaderPath);
            if (!FragmentFile.is_open())
                throw std::runtime_error("Failed to open fragment shader file: " + FragmentShaderPath);
            std::stringstream FragmentStream;
            FragmentStream << FragmentFile.rdbuf();
            FragmentShaderSource = FragmentStream.str();
        }
        
        // Functions
        void Compile() {
            const char *VertexSourcePtr = VertexShaderSource.c_str();
            GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(VertexShader, 1, &VertexSourcePtr, nullptr);
            glCompileShader(VertexShader);
            
            const char *FragmentSourcePtr = FragmentShaderSource.c_str();
            GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(FragmentShader, 1, &FragmentSourcePtr, nullptr);
            glCompileShader(FragmentShader);
            
            GLint Good;
            glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Good);
            if (!Good) {
                char Log[1024];
                glGetShaderInfoLog(VertexShader, 1024, nullptr, Log);
                throw std::runtime_error("Vertex Shader compile error: \n" + string(Log));
            }
            
            glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Good);
            if (!Good) {
                char Log[1024];
                glGetShaderInfoLog(FragmentShader, 1024, nullptr, Log);
                throw std::runtime_error("Fragment Shader compile error: \n" + string(Log));
            }
            
            ShaderProgram = glCreateProgram();
            
            glAttachShader(ShaderProgram, VertexShader);
            glAttachShader(ShaderProgram, FragmentShader);
            
            glLinkProgram(ShaderProgram);
            
            glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Good);
            if (!Good) {
                char Log[1024];
                glGetProgramInfoLog(ShaderProgram, 1024, nullptr, Log);
                throw std::runtime_error("Shader link error: \n" + string(Log));
            }
            
            glDeleteShader(VertexShader);
            glDeleteShader(FragmentShader);
        }
        
        void SetGlobalUniform(string Name, std::function<void(GLint)> Setter) {
            GLint UniformId = glGetUniformLocation(ShaderProgram, Name.c_str());
            _Uniforms.push_back(Uniform(UniformId, Setter));
        }
        
        void Activate() {
            glUseProgram(ShaderProgram);
            
            for (Uniform &UniformRef : _Uniforms)
                UniformRef.Set();
        }
        
        void Delete() {
            glDeleteProgram(ShaderProgram);
        }
    
        private:
            string VertexShaderSource;
            string FragmentShaderSource;
            vector<Uniform> _Uniforms;
            
};

#endif