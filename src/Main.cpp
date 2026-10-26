#include "Configs.hpp"

const glm::ivec2 WindowSize{400, 400};

// Functions
void HandleGraphics();
void HandleLogic(vector<string>& Equations);

int wmain(int ArgumentCount, wchar_t** Arguments) {
    SetConsoleOutputCP(CP_UTF8);
    
    vector<string> Equations;
    for (int i = 1; i < ArgumentCount; i++) {
        int Length = WideCharToMultiByte(CP_UTF8, 0, Arguments[i], -1, nullptr, 0, nullptr, nullptr);
        if (Length <= 0) continue;
        
        string Equation(Length, '\0');
        WideCharToMultiByte(CP_UTF8, 0, Arguments[i], -1, Equation.data(), Length, nullptr, nullptr);
        
        Equation.pop_back();
        Equations.push_back(Equation);
    }
    
    HandleLogic(Equations);
    // HandleGraphics();
    
    return 0;
}

void HandleGraphics() {
    if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW.");
    
    // --{ Window Creations }--
    GraphicsEngine Engine{};
    OpenGLWindow Window{&Engine, (glm::ivec2)WindowSize};
    Window.Display();
    Window.Maximize();
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
    Engine.RemoveEntity(SquareId);
    ShaderObject.Delete();
    Window.Delete();
    glfwTerminate();
}

void HandleLogic(vector<string>& Equations) {
    unordered_set<LogicGate*> VisitedGates;
    LogicEvaluator Evaluator;
    
    for (auto& Equation : Equations) {
        vector<string> Terms;
        string Operator = "";
        
        BooleanAlgebraHelpers::FlattenEquation(Equation, Terms, Operator, false);
        std::cout << Equation + " → " + "{\n"; //Evaluator.SimplifyEquation(Equation) + "\n";
        for (auto& Term : Terms) std::cout << "\t" + Term + "\n";
        std::cout << "}\n\n";
    }
    
    // BooleanSubCircuit<5, 1> Circuit{ "TestCircuit" };
    // std::cout << Circuit.GetEquation(VisitedGates);
}

/* 
./build/LogicSim.exe "((¬(¬A))∨B)" "((¬(¬A))∧(¬(¬B)))" "(A∧0)" "(A∨1)" "(A∧1)" "(A∨0)" "(A∧(¬A))" "(A∨(¬A))" "(¬(¬(¬A)))" "(¬(¬(A∧0)))" "(¬(¬(A∨1)))" "((A∧0)∨B)" "((A∨1)∧B)" "((A∧1)∨(B∧0))" "((A∨0)∧(B∧1))" "(¬((A∧0)∨B))" "(¬((A∨1)∧B))" "((¬(A∧B))∧A)" "((¬(A∨B))∨A)" "(¬((A∧1)∨0))"

((¬(¬A))∨B) → (A∨B)
((¬(¬A))∧(¬(¬B))) → (A∧B)
(A∧0) → 0
(A∨1) → 1
(A∧1) → A
(A∨0) → A
(A∧(¬A)) → 0
(A∨(¬A)) → 1
(¬(¬(¬A))) → (¬A)
(¬(¬(A∧0))) → 0
(¬(¬(A∨1))) → 1
((A∧0)∨B) → B
((A∨1)∧B) → B
((A∧1)∨(B∧0)) → A
((A∨0)∧(B∧1)) → (A∧B)
(¬((A∧0)∨B)) → (¬B)
(¬((A∨1)∧B)) → 0
((¬(A∧B))∧A) → ((¬(A∧B))∧A)
((¬(A∨B))∨A) → ((¬(A∨B))∨A)
(¬((A∧1)∨0)) → (¬A)
*/