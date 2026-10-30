// --{ Headers }--
#pragma once
#include <array>
#include <vector>
#include <iostream>
#include <windows.h>
#include <chrono>
#include <fstream>
#include <iomanip>

// --[ OpenGL ]--
#include "C:/glad/glad.h"
#include "C:/GLFW/glfw3.h"
#include "C:/glm/glm.hpp"
#include "C:/stb/stb_image.h"

// --[ Classes ]--
#include "Classes/GraphicsEngine/GraphicsEngine.hpp"
#include "Classes/GraphicsEngine/Window.hpp"

#include "Classes/LogicGates/LogicGates.hpp"
#include "Classes/LogicGates/LogicEvaluater.hpp"
#include "Classes/LogicGates/BooleanExpressions.hpp"

// --[ Shaders ]--
#include "DefaultVertexShader.vert.hpp"
#include "DefaultFragmentShader.frag.hpp"