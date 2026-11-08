#pragma once
#include <iostream>
#include <stdexcept>
#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"

namespace BooleanAlgebraASTTools {
    static bool IsValidNodeType(BooleanAlgebraNodeType Type) { return Type != BooleanAlgebraNodeType::Unknown; }
    static bool IsUnaryOperator(BooleanAlgebraNodeType Type) { return Type == BooleanAlgebraNodeType::NOT; }
    static bool IsBinaryOperator(BooleanAlgebraNodeType Type) { return (int)Type >= 3 && (int)Type <= 8; }
    
    static int GetOperandCount(const ASTNode& Node) { if (!IsValidNodeType(Node->Type)) throw std::invalid_argument("Passed in node is an invalid node."); return IsUnaryOperator(Node->Type) ? 1 : (IsBinaryOperator(Node->Type) ? 2 : 0); }
    static bool ValidateNode(const ASTNode& Node) {
        if (!Node) return false;
        if (!IsValidNodeType(Node->Type)) return false;
        if (IsUnaryOperator(Node->Type)) {
            auto* Unary = dynamic_cast<const UnaryExpressionNode*>(Node.get());
            if (Unary) return ValidateNode(Unary->Node);
            return false;
        }
        if (IsBinaryOperator(Node->Type)) {
            auto* Binary = dynamic_cast<const BinaryExpressionNode*>(Node.get());
            if (Binary) return ValidateNode(Binary->Left) && ValidateNode(Binary->Right);
            return false;
        }
    }
    static bool ValidateTree(const ASTNode& Node) {}
    
    static ASTNode ExtractNode(ASTNode& Target) { return std::move(Target); }
    static void ReplaceNode(ASTNode& Target, ASTNode Replacement) { Target = std::move(Replacement); }
    static bool StructurallyEqual(const ASTNode& NodeA, const ASTNode& NodeB) {
        if (!NodeA && !NodeB) return true;
        if (!NodeA || !NodeB) return false;
        
        if (NodeA->Type != NodeB->Type) return false;
        
        if (NodeA->Type == BooleanAlgebraNodeType::Variable) {
            auto* VariableA = dynamic_cast<const VariableNode*>(NodeA.get());
            auto* VariableB = dynamic_cast<const VariableNode*>(NodeB.get());
            if (VariableA && VariableB) {
                return VariableA->Value == VariableB->Value;
            }
            return false;
        }
        
        if (NodeA->Type == BooleanAlgebraNodeType::Constant) {
            auto* ConstA = dynamic_cast<const ConstantNode*>(NodeA.get());
            auto* ConstB = dynamic_cast<const ConstantNode*>(NodeB.get());
            if (ConstA && ConstB) {
                return ConstA->Value == ConstB->Value;
            }
            return false;
        }
        
        if (IsUnaryOperator(NodeA->Type)) {
            auto* UnaryA = dynamic_cast<const UnaryExpressionNode*>(NodeA.get());
            auto* UnaryB = dynamic_cast<const UnaryExpressionNode*>(NodeB.get());
            if (UnaryA && UnaryB) return StructurallyEqual(UnaryA->Node, UnaryB->Node);
            return false;
        }
        
        auto* BinaryA = dynamic_cast<const BinaryExpressionNode*>(NodeA.get());
        auto* BinaryB = dynamic_cast<const BinaryExpressionNode*>(NodeB.get());
        if (BinaryA && BinaryB) {
            return StructurallyEqual(BinaryA->Left, BinaryB->Left) && StructurallyEqual(BinaryA->Right, BinaryB->Right);
        }
        
        return false;
    }
    static bool BooleanEquivalent(const ASTNode& NodeA, const ASTNode& NodeB) {
        if (!NodeA || !NodeB) return false;
        
    }
}