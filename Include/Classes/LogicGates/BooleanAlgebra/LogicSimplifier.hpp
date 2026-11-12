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
            
            ReplaceOperators(ParsedEquation);
            for (int SimplifiedCount = 1; SimplifyOnce(ParsedEquation); SimplifiedCount++) if (Debug) { std::cout << "Simplified Equation #" << SimplifiedCount << ": "; ParsedEquation->print(); std::cout << "\n"; }
            RestoreOperators(ParsedEquation);
            
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
        
        static void ReplaceOperators(ASTNode& RootNode) {
            ReplaceXOR (RootNode);
            ReplaceXNOR(RootNode);
            ReplaceNAND(RootNode);
            ReplaceNOR (RootNode);
        }
        static void RestoreOperators(ASTNode& RootNode) {
            RestoreXOR (RootNode);
            RestoreXNOR(RootNode);
            RestoreNAND(RootNode);
            RestoreNOR (RootNode);
        }
        
        static void ReplaceXOR(ASTNode& Node) {
            if (!Node) return;
        
            if (IsUnaryOperator(Node->Type)) {
                auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
                ReplaceXOR(Unary->Node);
            } else if (IsBinaryOperator(Node->Type)) {
                auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
                
                ReplaceXOR(Binary->Left);
                ReplaceXOR(Binary->Right);
                
                if (Node->Type != BooleanAlgebraNodeType::XOR) return;
                
                ASTNode Replacement = std::make_unique<BinaryExpressionNode>( BooleanAlgebraNodeType::AND, "∧", std::make_unique<UnaryExpressionNode>(BooleanAlgebraNodeType::NOT, "¬", std::make_unique<BinaryExpressionNode>(BooleanAlgebraNodeType::AND, "∧", Binary->Left->Clone(), Binary->Right->Clone())), std::make_unique<BinaryExpressionNode>(BooleanAlgebraNodeType::OR, "∨", Binary->Left->Clone(), Binary->Right->Clone()));
                Node = std::move(Replacement);
            }
        }
        static void ReplaceXNOR(ASTNode& Node) {
            if (!Node) return;
            
            if (IsUnaryOperator(Node->Type)) {
                auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
                ReplaceXNOR(Unary->Node);
            } else if (IsBinaryOperator(Node->Type)) {
                auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
                
                ReplaceXNOR(Binary->Left);
                ReplaceXNOR(Binary->Right);
                
                if (Node->Type != BooleanAlgebraNodeType::XNOR) return;
                
                ASTNode Inside = std::make_unique<BinaryExpressionNode>(BooleanAlgebraNodeType::AND, "∧", std::make_unique<UnaryExpressionNode>(BooleanAlgebraNodeType::NOT, "¬", std::make_unique<BinaryExpressionNode>(BooleanAlgebraNodeType::AND, "∧", Binary->Left->Clone(), Binary->Right->Clone())), std::make_unique<BinaryExpressionNode>(BooleanAlgebraNodeType::OR, "∨", Binary->Left->Clone(), Binary->Right->Clone()));
                Node = std::make_unique<UnaryExpressionNode>(BooleanAlgebraNodeType::NOT, "¬", std::move(Inside));
            }
        }
        static void ReplaceNAND(ASTNode& Node) {
            if (!Node) return;
            
            if (IsUnaryOperator(Node->Type)) {
                auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
                ReplaceNAND(Unary->Node);
            } else if (IsBinaryOperator(Node->Type)) {
                auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
                
                ReplaceNAND(Binary->Left);
                ReplaceNAND(Binary->Right);
                
                if (Node->Type != BooleanAlgebraNodeType::NAND) return;
                
                ASTNode Inside = std::make_unique<BinaryExpressionNode>(BooleanAlgebraNodeType::AND, "∧", std::move(Binary->Left), std::move(Binary->Right));
                Node = std::make_unique<UnaryExpressionNode>(BooleanAlgebraNodeType::NOT, "¬", std::move(Inside));
            }
        }
        static void ReplaceNOR(ASTNode& Node) {
            if (!Node) return;
            
            if (IsUnaryOperator(Node->Type)) {
                auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
                ReplaceNOR(Unary->Node);
            } else if (IsBinaryOperator(Node->Type)) {
                auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
                
                ReplaceNOR(Binary->Left);
                ReplaceNOR(Binary->Right);
                
                if (Node->Type != BooleanAlgebraNodeType::NOR) return;
                
                ASTNode Inside = std::make_unique<BinaryExpressionNode>(BooleanAlgebraNodeType::OR, "∨", std::move(Binary->Left), std::move(Binary->Right));
                Node = std::make_unique<UnaryExpressionNode>(BooleanAlgebraNodeType::NOT, "¬", std::move(Inside));
            }
        }
        
        static void RestoreXOR(ASTNode& Node ) { RestoreOperator(Node, BooleanAlgebraNodeType::XOR , "⊕"); }
        static void RestoreXNOR(ASTNode& Node) { RestoreOperator(Node, BooleanAlgebraNodeType::XNOR, "⊙"); }
        static void RestoreNAND(ASTNode& Node) { RestoreOperator(Node, BooleanAlgebraNodeType::NAND, "↑"); }
        static void RestoreNOR(ASTNode& Node ) { RestoreOperator(Node, BooleanAlgebraNodeType::NOR , "↓"); }
        
        static void CollectSubtrees(BooleanAlgebraNode* Node, std::vector<BooleanAlgebraNode*>& Nodes) {
            if (!Node) return;
            
            Nodes.push_back(Node);
            
            if (IsUnaryOperator(Node->Type)) {
                auto* Unary = static_cast<UnaryExpressionNode*>(Node);
                CollectSubtrees(Unary->Node.get(), Nodes);
            } else if (IsBinaryOperator(Node->Type)) {
                auto* Binary = static_cast<BinaryExpressionNode*>(Node);
                CollectSubtrees(Binary->Left.get(), Nodes);
                CollectSubtrees(Binary->Right.get(), Nodes);
            }
        }
        static bool ContainsNode(BooleanAlgebraNode* Root, BooleanAlgebraNode* Target) {
            if (!Root) return false;
            if (Root == Target) return true;
            
            if (IsUnaryOperator(Root->Type)) {
                auto* Unary = static_cast<UnaryExpressionNode*>(Root);
                return ContainsNode(Unary->Node.get(), Target);
            }
            
            if (IsBinaryOperator(Root->Type)) {
                auto* Binary = static_cast<BinaryExpressionNode*>(Root);
                return ContainsNode(Binary->Left.get(), Target) ||
                       ContainsNode(Binary->Right.get(), Target);
            }
            
            return false;
        }
        static void RestoreOperator(ASTNode& Node, BooleanAlgebraNodeType Type, const std::string& Operator) {
            if (!Node) return;
        
            if (IsUnaryOperator(Node->Type)) {
                auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
                RestoreOperator(Unary->Node, Type, Operator);
            } else if (IsBinaryOperator(Node->Type)) {
                auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
                RestoreOperator(Binary->Left, Type, Operator);
                RestoreOperator(Binary->Right, Type, Operator);
            }
            
            std::vector<BooleanAlgebraNode*> Candidates;
            CollectSubtrees(Node.get(), Candidates);
            
            std::stable_sort(
                Candidates.begin(),
                Candidates.end(),
                [](BooleanAlgebraNode* A, BooleanAlgebraNode* B) { return GetCost(ASTNode(A->Clone())) < GetCost(ASTNode(B->Clone())); }
            );
            
            for (size_t i = 0; i < Candidates.size(); i++) {
                for (size_t j = i + 1; j < Candidates.size(); j++) {
                    if (ContainsNode(Candidates[i], Candidates[j]) ||
                        ContainsNode(Candidates[j], Candidates[i])) continue;
                    
                    ASTNode Test = std::make_unique<BinaryExpressionNode>(
                        Type,
                        Operator,
                        Candidates[i]->Clone(),
                        Candidates[j]->Clone()
                    );
                    
                    if (!BooleanEquivalent(Test, Node)) continue;
                    
                    Node = std::move(Test);
                    return;
                }
            }
        }
};