#pragma once
#include <stdexcept>
#include <iostream>

#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraASTTools.hpp"

namespace BooleanAlgebraLaws {
    static void DoubleNegation(ASTNode& Node, bool& Changed) {
        if (BooleanAlgebraASTTools::IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (!BooleanAlgebraASTTools::IsUnaryOperator(Unary->Node->Type)) { DoubleNegation(Unary->Node, Changed); return; }
            auto* NestedUnary = static_cast<UnaryExpressionNode*>(Unary->Node.get());
            ASTNode Grandchild = std::move(NestedUnary->Node);
            BooleanAlgebraASTTools::ReplaceNode(Node, std::move(Grandchild));
            DoubleNegation(Node, Changed);
            Changed = true;
        } else if (BooleanAlgebraASTTools::IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            DoubleNegation(Binary->Left, Changed);
            DoubleNegation(Binary->Right, Changed);
        }
    }
    static void Annihlation(ASTNode& Node, bool& Changed) {
        if (BooleanAlgebraASTTools::IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            ConstantNode* Constant;
            bool LeftIsConstant;
            
            if (Binary->Left->Type == BooleanAlgebraNodeType::Constant) { Constant = static_cast<ConstantNode*>(Binary->Left.get()); LeftIsConstant = true; }
            else if (Binary->Right->Type == BooleanAlgebraNodeType::Constant) { Constant = static_cast<ConstantNode*>(Binary->Right.get()); LeftIsConstant = false; }
            else { Annihlation(Binary->Left, Changed); Annihlation(Binary->Right, Changed); return; }
            
            bool AndAnnihlation = Binary->Type == BooleanAlgebraNodeType::AND && Constant->Value == 0;
            bool OrAnnihlation = Binary->Type == BooleanAlgebraNodeType::OR && Constant->Value == 1;
            if (!(AndAnnihlation || OrAnnihlation)) { Annihlation(Binary->Left, Changed); Annihlation(Binary->Right, Changed); return; }
            
            BooleanAlgebraASTTools::ReplaceNode(Node, std::move(LeftIsConstant ? Binary->Left : Binary->Right));
            Changed = true;
        } else if (BooleanAlgebraASTTools::IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            Annihlation(Unary->Node, Changed);
        }
    }
    static void Identity(ASTNode& Node, bool& Changed) {
        if (BooleanAlgebraASTTools::IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            ConstantNode* Constant;
            bool LeftIsConstant;
            
            if (Binary->Left->Type == BooleanAlgebraNodeType::Constant) { Constant = static_cast<ConstantNode*>(Binary->Left.get()); LeftIsConstant = true; }
            else if (Binary->Right->Type == BooleanAlgebraNodeType::Constant) { Constant = static_cast<ConstantNode*>(Binary->Right.get()); LeftIsConstant = false; }
            else { Identity(Binary->Left, Changed); Identity(Binary->Right, Changed); return; }
            
            bool AndIdentity = Binary->Type == BooleanAlgebraNodeType::AND && Constant->Value == 1;
            bool OrIdentity = Binary->Type == BooleanAlgebraNodeType::OR && Constant->Value == 0;
            if (!(AndIdentity || OrIdentity)) { Identity(Binary->Left, Changed); Identity(Binary->Right, Changed); return; }
            
            BooleanAlgebraASTTools::ReplaceNode(Node, std::move(LeftIsConstant ? Binary->Right : Binary->Left));
            Identity(Node, Changed);
            Changed = true;
        } else if (BooleanAlgebraASTTools::IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            Identity(Unary->Node, Changed);
        }
    }
    static void Idempotence(ASTNode& Node, bool& Changed);
    static void Complementation(ASTNode& Node, bool& Changed) {
        if (BooleanAlgebraASTTools::IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            UnaryExpressionNode* NegatedNode;
            bool LeftIsNegated;
            
            if (!(Binary->Type == BooleanAlgebraNodeType::AND || Binary->Type == BooleanAlgebraNodeType::OR)) { Complementation(Binary->Left, Changed); Complementation(Binary->Right, Changed); return; }
            else if (Binary->Left->Type  == BooleanAlgebraNodeType::NOT) { NegatedNode = static_cast<UnaryExpressionNode*>(Binary->Left.get()); LeftIsNegated = true; }
            else if (Binary->Right->Type == BooleanAlgebraNodeType::NOT) { NegatedNode = static_cast<UnaryExpressionNode*>(Binary->Right.get()); LeftIsNegated = false; }
            else { Complementation(Binary->Left, Changed); Complementation(Binary->Right, Changed); return; }
            
            bool Complements = BooleanAlgebraASTTools::BooleanEquivalent(NegatedNode->Node, LeftIsNegated ? Binary->Right : Binary->Left);
            if (Complements) { Complementation(Binary->Left, Changed); Complementation(Binary->Right, Changed); return; }
            
            ASTNode Constant = std::make_unique<ConstantNode>(Binary->Type == BooleanAlgebraNodeType::AND ? 0 : 1);
            BooleanAlgebraASTTools::ReplaceNode(Node, std::move(Constant));
            Complementation(Node, Changed);
            Changed = true;
        } else if (BooleanAlgebraASTTools::IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            Complementation(Unary->Node, Changed);
        }
    }
    static void Absorption(ASTNode& Node, bool& Changed);
    static void DeMorgan(ASTNode& Node, bool& Changed);
    static void Distributivity(ASTNode& Node, bool& Changed);
}