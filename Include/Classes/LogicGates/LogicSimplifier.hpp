#pragma once
#include "Classes/LogicGates/LogicGates.hpp"
#include "Classes/LogicGates/BooleanAlgebraHelpers.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraASTTools.hpp"
#include <vector>
#include <string>
#include <stdexcept>

using std::string;
using std::vector;

class LogicSimplifier {
    public:
        inline static bool Debug = false;
        inline static bool DeepDebug = false;
        inline static bool RecursiveDebug = false;
        inline static int RecursiveDebugDepth = 3;
        
        static string SimplifyEquation(LogicGate* Gate) { return SimplifyEquation(Gate->GetEquation()); }
        static string SimplifyEquation(string Equation) { SimplifyEquation(&Equation); return Equation; }
        static void SimplifyEquation(string* Equation) {
            if (Equation == nullptr) throw std::invalid_argument("Equation must not be null.");
            BooleanAlgebraASTTools::ParseEquation(*Equation)->print();
        }
        
    private:
};