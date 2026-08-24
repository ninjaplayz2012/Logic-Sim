#ifndef WINDOW_CLASS_HPP
#define WINDOW_CLASS_HPP

#include "C:/GLFW/glfw3.h"
#include "C:/glad/glad.h"
#include <vector>
#include <string>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include "Classes/GraphicsEngine/GraphicsEngine.hpp"
#include "Classes/GraphicsEngine/ShaderClass.hpp"

using std::string;
using std::vector;

class OpenGLWindow {
    public:
        
        // Variables
        GLFWwindow *Window;
        GLFWmonitor *Monitor;
        GLFWvidmode Mode;
        bool Rendering = false;
        double DeltaTime = 0;
        GraphicsEngine *GraphicsEngineReference;
        vector<Shader*> Shaders;
        
        // Initializers
        OpenGLWindow(GraphicsEngine *GraphicsEngineReference, glm::ivec2 Size, GLFWmonitor *FullscreenMonitor = nullptr) : GraphicsEngineReference(GraphicsEngineReference), Size(Size) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            
            Monitor = FullscreenMonitor != nullptr ? FullscreenMonitor : glfwGetPrimaryMonitor();
            Mode = *glfwGetVideoMode(Monitor);
            
            glfwWindowHint(GLFW_RED_BITS, Mode.redBits);
            glfwWindowHint(GLFW_GREEN_BITS, Mode.greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, Mode.blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, Mode.refreshRate);
            
            Window = glfwCreateWindow(Size.x, Size.y, "Life Simulation", FullscreenMonitor, nullptr);
            if (!Window) {
                glfwTerminate();
                throw std::runtime_error("Failed to create the glfw window.");
            }
        }
        
        // Functions
        void Display() {
            glfwMakeContextCurrent(Window);
            glfwSwapInterval(1);
            
            glViewport(0, 0, Size.x, Size.y);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        
        void Maximize() {
            glfwMaximizeWindow(Window);
            SetSize(glm::ivec2(Mode.width, Mode.height));
        }
        
        void SetBackgroundColor(float Red, float Green, float Blue, float Alpha) {
            glClearColor(Red, Green, Blue, Alpha);
        }
        
        void SetBackgroundColorHex(string HexCode) {
            HexCode = HexCode.substr(1); // Remove the "#" from the hex code
            int Red   = std::stoi(HexCode.substr(0, 2), nullptr, 16);
            int Green = std::stoi(HexCode.substr(2, 2), nullptr, 16);
            int Blue  = std::stoi(HexCode.substr(4, 2), nullptr, 16);
            int Alpha = std::stoi(HexCode.substr(6, 2), nullptr, 16);
            glClearColor(Red / 255.0f, Green / 255.0f, Blue / 255.0f, Alpha / 255.0f);
        }
        
        void Update(vector<Shader*> NewShaders) {
            NewShaders.insert(NewShaders.end(), Shaders.begin(), Shaders.end());
            Shaders = NewShaders;
            
            for (Shader *ShaderPtr : Shaders) {
                ShaderPtr->Compile();
                ShaderPtr->SetGlobalUniform("WindowSize", [this](GLint Location){glUniform2f(Location, (float)Size.x, (float)Size.y);});
            }
            
            glfwSetWindowUserPointer(Window, this);
            glfwSetFramebufferSizeCallback(Window, [](GLFWwindow *Win, int Wid, int Hei) {
                OpenGLWindow *Self = static_cast<OpenGLWindow*>(glfwGetWindowUserPointer(Win));
                Self->ChangeSize(glm::ivec2(Wid, Hei));
            });
        }
        
        void ChangeSize(glm::ivec2 Size) {
            SetSize(Size);
            
            for (auto &Entity : GraphicsEngineReference->Entities)
                Entity.GetSize(Size);
        }
        
        void StartRenderLoop(std::function<void(double)> Callback = nullptr) {
            Rendering = true;
            
            double CurrentTime = glfwGetTime();
            double PreviousTime = glfwGetTime();
            while (!glfwWindowShouldClose(Window)) {
                CurrentTime = glfwGetTime();
                DeltaTime = CurrentTime - PreviousTime;
                PreviousTime = CurrentTime;
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                GraphicsEngineReference->Render(GL_TRIANGLES);
                if (Callback)
                    Callback(DeltaTime);
                
                glfwSwapBuffers(Window);
                glfwPollEvents();
            }
            
            Rendering = false;
        }
        
        void Delete() {
            glfwDestroyWindow(Window);
        }
        
        glm::ivec2 GetSize() {
            return Size;
        }
        
        void SetSize(glm::ivec2 Size) {
            glViewport(0, 0, Size.x, Size.y);
            Size = Size;
        }
    
        private:
            glm::ivec2 Size;
};

#endif