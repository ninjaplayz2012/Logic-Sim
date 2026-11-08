#pragma once
#include "Classes/LogicGates/LogicGates.hpp"
#include "Classes/LogicGates/BooleanAlgebraHelpers.hpp"
#include <vector>
#include <string>

using std::string;
using std::vector;
using std::regex;
using std::unordered_set;

class LogicEvaluator {
    public:
        inline static bool Debug = false;
        inline static bool DeepDebug = false;
        inline static bool RecursiveDebug = false;
        inline static int RecursiveDebugDepth = 3;
        
        static string SimplifyEquation(LogicGate* Gate) { return SimplifyEquation(Gate->GetEquation()); }
        static string SimplifyEquation(string Equation) { SimplifyEquation(&Equation); return Equation; }
        static void SimplifyEquation(string* Equation) {
            if ()
        }
        
    private:
    
};