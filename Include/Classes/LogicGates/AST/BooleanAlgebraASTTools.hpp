#pragma once
// --{ Includes }--
#include <iostream>
#include <stdexcept>
#include <vector>
#include <utility>
#include <unordered_map>

#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"

// --{ Using(s) }--
using std::vector;
using std::pair;
using std::unordered_map;
using VariablesVector = vector<pair<string, vector<ASTNode*>>>;
using VariablesMap = unordered_map<string, size_t>;

// --{ Namespace }--
namespace BooleanAlgebraASTTools {
    // --[ Type Checks ]--
    static bool IsValidNodeType(BooleanAlgebraNodeType Type) { return (int)Type >= 0 && (int)Type <= 8; }
    static bool IsUnaryOperator(BooleanAlgebraNodeType Type) { return Type == BooleanAlgebraNodeType::NOT; }
    static bool IsBinaryOperator(BooleanAlgebraNodeType Type) { return (int)Type >= 3 && (int)Type <= 8; }
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
    
    // --[ Operand/Operation ]--
    static int GetOperandCount(const ASTNode& Node) { if (!IsValidNodeType(Node->Type)) throw std::invalid_argument("Passed in node is an invalid node."); return IsUnaryOperator(Node->Type) ? 1 : (IsBinaryOperator(Node->Type) ? 2 : 0); }
    static int GetOperationCount(const ASTNode& Node) {
        if (!Node) return 0;
    
        int Total = IsUnaryOperator(Node->Type) || IsBinaryOperator(Node->Type);
    
        if (IsUnaryOperator(Node->Type)) {
            auto* Unary = dynamic_cast<const UnaryExpressionNode*>(Node.get());
            if (Unary) Total += GetOperationCount(Unary->Node);
        } else if (IsBinaryOperator(Node->Type)) {
            auto* Binary = dynamic_cast<const BinaryExpressionNode*>(Node.get());
            if (Binary) Total += GetOperationCount(Binary->Left) + GetOperationCount(Binary->Right);
        }
    
        return Total;
    }
    static int GetCost(const ASTNode& Node) {
        // Temp
        return GetOperationCount(Node);
    }
    
    // --[ Parsing/Serializing ]--
    static string NodeTypeToString(BooleanAlgebraNodeType NodeType) {
        switch (NodeType) {
            case (BooleanAlgebraNodeType::Variable):
                return "Variable";
            case (BooleanAlgebraNodeType::Constant):
                return "Constant";
            case (BooleanAlgebraNodeType::NOT     ):
                return "NOT";
            case (BooleanAlgebraNodeType::AND     ):
                return "AND";
            case (BooleanAlgebraNodeType::OR      ):
                return "OR";
            case (BooleanAlgebraNodeType::XOR     ):
                return "XOR";
            case (BooleanAlgebraNodeType::XNOR    ):
                return "XNOR";
            case (BooleanAlgebraNodeType::NOR     ):
                return "NOR";
            case (BooleanAlgebraNodeType::NAND    ):
                return "NAND";
            case (BooleanAlgebraNodeType::Unknown ):
                return "Unknown";
            default:
                return "Invalid Type";
        }
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
            auto* Variable = static_cast<const VariableNode*>(Node.get());
            if (Variable) return Variable->Value;
        }
        
        if (Node->Type == BooleanAlgebraNodeType::Constant) {
            auto* Const = static_cast<const ConstantNode*>(Node.get());
            if (Const) return Const->Value == 0 ? "0" : "1";
        }
        
        if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<const UnaryExpressionNode*>(Node.get());
            if (Unary) return "(" + Unary->Operator + SerializeNode(Unary->Node) + ")";
        }
        
        if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<const BinaryExpressionNode*>(Node.get());
            if (Binary) return "(" + SerializeNode(Binary->Left) + Binary->Operator + SerializeNode(Binary->Right) + ")";
        }
        
        return "";
    }
    
    // --[ Variables ]--
    static void GetVariables(const ASTNode& Node, VariablesVector& Variables, VariablesMap& ContainedValues) {
        if (!Node) return;
        if (Node->Type == BooleanAlgebraNodeType::Variable) {
            auto* Variable = static_cast<VariableNode*>(Node.get());
            if (ContainedValues.contains(Variable->Value)) { Variables[ContainedValues[Variable->Value]].second.push_back(const_cast<ASTNode*>(&Node)); return; }
            Variables.emplace_back(Variable->Value, vector<ASTNode*>{const_cast<ASTNode*>(&Node)});
            ContainedValues[Variable->Value] = Variables.size() - 1;
        } else if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (Unary) GetVariables(Unary->Node, Variables, ContainedValues);
        }else if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (Binary) { GetVariables(Binary->Left, Variables, ContainedValues); GetVariables(Binary->Right, Variables, ContainedValues);}
        }
    }
    static VariablesVector GetVariables(const ASTNode& RootNode) {
        VariablesVector Variables;
        VariablesMap ContainedValues;
        GetVariables(RootNode, Variables, ContainedValues);
        return Variables;
    }
    
    // --[ Manipulation ]--
    static ASTNode ExtractNode(ASTNode& Target) { return std::move(Target); }
    static void ReplaceNode(ASTNode& Target, ASTNode Replacement) { Target = std::move(Replacement); }
    
    // --[ Equivalence ]--
    static bool StructurallyEqual(const ASTNode& NodeA, const ASTNode& NodeB) {
        if (!NodeA && !NodeB) return true;
        if (!NodeA || !NodeB) return false;
        if (!ValidateNode(NodeA) || !ValidateNode(NodeB)) return false;
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
        
        if (IsBinaryOperator(NodeA->Type)) {
            auto* BinaryA = dynamic_cast<const BinaryExpressionNode*>(NodeA.get());
            auto* BinaryB = dynamic_cast<const BinaryExpressionNode*>(NodeB.get());
            if (BinaryA && BinaryB) {
                return StructurallyEqual(BinaryA->Left, BinaryB->Left) && StructurallyEqual(BinaryA->Right, BinaryB->Right);
            }
        }
        
        return false;
    }
    static bool BooleanEquivalent(const ASTNode& NodeA, const ASTNode& NodeB);
}