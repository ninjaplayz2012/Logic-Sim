#include "Configs.hpp"

const glm::ivec2 Size{400, 400};

int main() {
    if (!glfwInit()) return -1;
    
    // --{ Window Creations }--
    GraphicsEngine Engine{};
    OpenGLWindow Window{&Engine, (glm::ivec2)Size};
    Window.Display();
    Window.SetBackgroundColorHex("#1e1e2eFF");
    
    // --{ Shaders }--
    Shader ShaderObject{"./Shaders/DefaultVertexShader.vert", "./Shaders/DefaultFragmentShader.frag"};
    
    vector<Shader*> Shaders = {&ShaderObject};
    Window.Update(Shaders);
    
    // --{ Objects }--
    
    // --{ Render Loop }--
    Window.StartRenderLoop();
    
    // --{ Clean Up }--
    ShaderObject.Delete();
    Window.Delete();
    glfwTerminate();
    
    return 0;
}