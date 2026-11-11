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
using namespace BooleanAlgebraASTTools;

class LogicSimplifier {
    public:
        inline static bool Debug = false;
        
        static string SimplifyEquation(LogicGate* Gate) { return SimplifyEquation(Gate->GetEquation()); }
        static string SimplifyEquation(string Equation) { SimplifyEquation(&Equation); return Equation; }
        static void SimplifyEquation(string* Equation) {
            if (Equation == nullptr) throw std::invalid_argument("Equation must not be null.");
            
            ASTNode ParsedEquation = ParseEquation(*Equation);
            if (Debug) { 
                std::cout << "Simlifying Equation:\n";
                std::cout << "Original Equation: " + *Equation + "\n";
                std::cout << "Parsed Equation: "; ParsedEquation->print(); std::cout << "\n"; 
            }
            
            for (int SimplifiedCount = 1; SimplifyOnce(ParsedEquation); SimplifiedCount++) if (Debug) { std::cout << "Simplified Equation #" << SimplifiedCount << ": "; ParsedEquation->print(); std::cout << "\n"; }
            
            *Equation = SerializeNode(ParsedEquation);
            if (Debug) { std::cout << "Final Equation: " + *Equation + "\n"; }
        }
        
    private:
        static bool SimplifyOnce(ASTNode& RootNode) {
            bool Changed = false;
            
            if (Debug) std::cout << "Pre-DoubleNegation: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::DoubleNegation(RootNode, Changed);
            if (Debug) std::cout << "Post-DoubleNegation: " + SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-Annihlation: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::Annihlation(RootNode, Changed);
            if (Debug) std::cout << "Post-Annihlation: " + SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-Identity: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::Identity(RootNode, Changed);
            if (Debug) std::cout << "Post-Identity: " + SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-Idempotence: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::Idempotence(RootNode, Changed);
            if (Debug) std::cout << "Post-Idempotence: " + SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-Complementation: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::Complementation(RootNode, Changed);
            if (Debug) std::cout << "Post-Complementation: " + SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-Absorption: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::Absorption(RootNode, Changed);
            if (Debug) std::cout << "Post-Absorption: " + SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-DeMorgan: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::DeMorgan(RootNode, Changed);
            if (Debug) std::cout << "Post-DeMorgan: " + SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-Distributivity: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::Distributivity(RootNode, Changed);
            if (Debug) std::cout << "Post-Distributivity: " + SerializeNode(RootNode) + "\n";
            
            if (Debug) std::cout << "Pre-SimplifyConstants: " + SerializeNode(RootNode) + "\n";
            BooleanAlgebraLaws::SimplifyConstants(RootNode, Changed);
            if (Debug) std::cout << "Post-SimplifyConstants: " + SerializeNode(RootNode) + "\n";
            
            return Changed;
        }
};