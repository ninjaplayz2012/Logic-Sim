#pragma once
#include <vector>
#include <string>
#include <stdexcept>

#include "Classes/LogicGates/LogicGates.hpp"
#include "Classes/LogicGates/BooleanAlgebra/BooleanAlgebraHelpers.hpp"
#include "Classes/LogicGates/BooleanAlgebra/BooleanAlgebraLaws.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraASTTools.hpp"

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
            
            ASTNode ParsedEquation = BooleanAlgebraASTTools::ParseEquation(*Equation);
            if (Debug) { 
                std::cout << "Simlifying Equation:\n";
                std::cout << "Original Equation: " + *Equation + "\n";
                std::cout << "Parsed Equation: "; ParsedEquation->print(); std::cout << "\n"; 
            }
            
            for (int SimplifiedCount = 1; SimplifyOnce(ParsedEquation); SimplifiedCount++) if (Debug) { std::cout << "Simplified Equation #" << SimplifiedCount << ": "; ParsedEquation->print(); std::cout << "\n"; }
            
            *Equation = BooleanAlgebraASTTools::SerializeNode(ParsedEquation);
            if (Debug) { std::cout << "Final Equation: " + *Equation + "\n"; }
        }
        
    private:
        static bool SimplifyOnce(ASTNode& RootNode) {
            bool Changed = false;
            
            if (Debug) std::cout << "Pre-DoubleNegation: " + BooleanAlgebraASTTools::SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::DoubleNegation(RootNode, Changed);
            if (Debug) std::cout << "Post-DoubleNegation: " + BooleanAlgebraASTTools::SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-Annihlation: " + BooleanAlgebraASTTools::SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::Annihlation(RootNode, Changed);
            if (Debug) std::cout << "Post-Annihlation: " + BooleanAlgebraASTTools::SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-Identity: " + BooleanAlgebraASTTools::SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::Identity(RootNode, Changed);
            if (Debug) std::cout << "Post-Identity: " + BooleanAlgebraASTTools::SerializeNode(RootNode) + "\n";
            
            return Changed;
        }
};