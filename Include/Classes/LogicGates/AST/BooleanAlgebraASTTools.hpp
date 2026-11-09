#pragma once
#include <iostream>
#include <stdexcept>
#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"

namespace BooleanAlgebraASTTools {
    static bool IsValidNodeType(BooleanAlgebraNodeType Type) { return (int)Type >= 0 && (int)Type <= 8; }
    static bool IsValidNodeType(BooleanAlgebraNodeType Type) { return (int)Type >= 0 && (int)Type <= 8; }
    static bool IsUnaryOperator(BooleanAlgebraNodeType Type) { return Type == BooleanAlgebraNodeType::NOT; }
    static bool IsBinaryOperator(BooleanAlgebraNodeType Type) { return (int)Type >= 3 && (int)Type <= 8; }
    
    static int GetOperandCount(const ASTNode& Node) { if (!IsValidNodeType(Node->Type)) throw std::invalid_argument("Passed in node is an invalid node."); return IsUnaryOperator(Node->Type) ? 1 : (IsBinaryOperator(Node->Type) ? 2 : 0); }
    static int GetOperationCount(const ASTNode& Node) {
        int Total = GetOperandCount(Node);
        
    static int GetOperationCount(const ASTNode& Node) {
        int Total = GetOperandCount(Node);
        
        if (IsUnaryOperator(Node->Type)) {
            auto* Unary = dynamic_cast<const UnaryExpressionNode*>(Node.get());
            if (Unary) Total += GetOperationCount(Unary->Node);
        } else if (IsBinaryOperator) {
            if (Unary) Total += GetOperationCount(Unary->Node);
        } else if (IsBinaryOperator) {
            auto* Binary = dynamic_cast<const BinaryExpressionNode*>(Node.get());
            if (Binary) Total += GetOperationCount(Binary->Left) + GetOperationCount(Binary->Right);
        }
        
        return GetOperandCount(Node);
    }
    static int GetCost(const ASTNode& Node) {
        // Temp
        return GetOperationCount(Node);
    }
    
    static BooleanAlgebraNodeType StringToNodeType(string& String) {
        if (String == "¬") return BooleanAlgebraNodeType::NOT;
        else if (String == "∧") return BooleanAlgebraNodeType::AND;
        else if (String == "∨") return BooleanAlgebraNodeType::OR;
        else if (String == "↓") return BooleanAlgebraNodeType::NOR;
        else if (String == "⊕") return BooleanAlgebraNodeType::XOR;
        else if (String == "⊙") return BooleanAlgebraNodeType::XNOR;
        else if (String == "↑") return BooleanAlgebraNodeType::NAND;
        else if (String == "Constant") return BooleanAlgebraNodeType::Constant;
        else if (String == "Variable") return BooleanAlgebraNodeType::Variable;
        else return BooleanAlgebraNodeType::Unknown;
    }
    static ASTNode ParseEquation(const string& Equation) {
        string Left, Right, Operator;
        
        if (Equation.starts_with("(¬")) return std::make_unique<UnaryExpressionNode>(
            BooleanAlgebraNodeType::NOT, 
            "¬", 
            ParseEquation(Equation.substr(3, BooleanAlgebraHelpers::FindClosed(Equation, 0) - 3))
        );
        
        if (BooleanAlgebraHelpers::SplitTopLevel(Equation, Left, Right, Operator)) return std::make_unique<BinaryExpressionNode>(
            StringToNodeType(Operator),
            Operator,
            ParseEquation(Left ),
            ParseEquation(Right)
        );
        
        if (Equation == "0" || Equation == "1") return std::make_unique<ConstantNode>(Equation == "0" ? 0 : 1);
        
        return std::make_unique<VariableNode>(Equation);
    }
    static string SerializeNode(const ASTNode& Node) {
        if (!Node) return "";
        
        if (Node->Type == BooleanAlgebraNodeType::Variable) {
            auto* Variable = dynamic_cast<const VariableNode*>(Node.get());
            if (Variable) return Variable->Value;
        }
        
        if (Node->Type == BooleanAlgebraNodeType::Constant) {
            auto* Const = dynamic_cast<const ConstantNode*>(Node.get());
            if (Const) return Const->Value == 0 ? "0" : "1";
        }
        
        if (IsUnaryOperator(Node->Type)) {
            auto* Unary = dynamic_cast<const UnaryExpressionNode*>(Node.get());
            if (Unary) return "(" + Unary->Operator + SerializeNode(Unary->Node) + ")";
        }
        
        auto* Binary = dynamic_cast<const BinaryExpressionNode*>(Node.get());
        if (Binary) return "(" + SerializeNode(Binary->Left) + Binary->Operator + SerializeNode(Binary->Right) + ")";
        
        return "";
    }
    
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
        string EquationA = SerializeNode(NodeA);
        string EquationB = SerializeNode(NodeB);
        
        return false;
    }
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
        return true;
    }
}