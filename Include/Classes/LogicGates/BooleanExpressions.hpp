#pragma once

#include "Classes/LogicGates/LogicGates.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <cctype>
#include <unordered_set>

extern "C" bool ANDOP(bool A, bool B);

using std::string, std::vector, std::unordered_map, std::stack, std::unordered_set;

class BooleanInterpreter {
    public:
        static bool EvaluateEquation(string& Equation) {
            return !Equation.empty() && Equation == "1";
        }
    private:
        
};

template <int InputCountTemplate, int OutputCountTemplate>
class BooleanSubCircuit : public BaseGate<InputCountTemplate> {
    public:
        array<string, OutputCountTemplate> Equations;
        
        BooleanSubCircuit(string ClassName, array<string, OutputCountTemplate>& Equations) : BaseGate<InputCountTemplate>(ClassName), Equations(Equations) {}
        BooleanSubCircuit(string ClassName) : BaseGate<InputCountTemplate>(ClassName) {}
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if constexpr (OutputCountTemplate > 0) return Equations[0];
            return "0";
        }
        
        void ComputeNextState() override {}
    private:
        
};