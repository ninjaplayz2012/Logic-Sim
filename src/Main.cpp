#include "Configs.hpp"

const glm::ivec2 WindowSize{400, 400};

int main() {
    if (!glfwInit()) return -1;
    
    // --{ Window Creations }--
    GraphicsEngine Engine{};
    OpenGLWindow Window{&Engine, (glm::ivec2)WindowSize};
    Window.Display();
    Window.SetBackgroundColorHex("#1e1e2eFF");
    
    // --{ Shaders }--
    Shader ShaderObject = Shader::FromSource(DefaultVertexShader_vert, DefaultFragmentShader_frag);
    
    vector<Shader*> Shaders = {&ShaderObject};
    Window.Update(Shaders);
    
    // --{ Objects }--
    const float Size = 100.0f;

    vector<BaseVector> SquareVertices = {
        BaseVector(glm::vec3(0.0f, Size, 0.0f), glm::vec2(0.0f, 0.0f), std::nullopt, std::vector<float>{1.0f, 0.2f, 0.2f, 1.0f}),
        BaseVector(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), std::nullopt, std::vector<float>{1.0f, 0.2f, 0.2f, 1.0f}),
        BaseVector(glm::vec3(Size, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), std::nullopt, std::vector<float>{1.0f, 0.2f, 0.2f, 1.0f}),
        BaseVector(glm::vec3(Size, Size, 0.0f), glm::vec2(0.0f, 0.0f), std::nullopt, std::vector<float>{1.0f, 0.2f, 0.2f, 1.0f})
    };

    vector<GLuint> SquareIndices = {
        0, 1, 2,
        0, 2, 3
    };

    auto Square = std::make_unique<Entity>(ShaderObject, SquareVertices, SquareIndices);
    Square->AnchorPoint = glm::vec2(0.5f, 0.5f);
    Square->PositionScale = glm::vec2(0.5f, 0.5f);
    Square->Position = glm::vec3(0.0f, 0.0f, 0.0f);
    Square->Generate();
    Square->SetDefaultUniforms();
    Square->GetSize(glm::vec2(Window.GetSize()));
    
    int SquareId = Engine.AddEntity(std::move(Square));
    
    // --{ Render Loop }--
    Window.StartRenderLoop();
    
    // --{ Clean Up }--
    ShaderObject.Delete();
    Window.Delete();
    glfwTerminate();
    
    return 0;
}