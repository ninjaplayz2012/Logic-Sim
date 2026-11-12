#pragma once
#include <stdexcept>
#include <iostream>
#include <array>
#include <unordered_set>

#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraASTTools.hpp"
#include "Classes/LogicGates/BooleanAlgebra/LogicEvaluator.hpp"

// --{ Enums }--
enum class SkipLaw {
    DeMorgan,
    Distributivity
};

// --{ Using(s) }--
using namespace BooleanAlgebraASTTools;
using std::array;
using std::unordered_set;

// --{ Namespace }--

namespace BooleanAlgebraLaws {
    static void NegateAll(ASTNode& Node);
    static void Absorbs(ASTNode& Replacement, BinaryExpressionNode* Binary, ASTNode& InnerNode, ASTNode& OtherNode);
    static bool DeMorganWorks(ASTNode& Replacement, ASTNode& Node);
    static bool ReduceOnce(ASTNode& RootNode, bool& Changed, SkipLaw Law);
    static bool Reduce(ASTNode& Node, SkipLaw Law);
    static bool Distributes(ASTNode& Replacement, ASTNode& Node);
    static bool CanFactor(ASTNode& Node, ASTNode& Left, ASTNode& Right, ASTNode& Common);
    
    static void DoubleNegation(ASTNode& Node, bool& Changed) {
        if (!Node) return;
        
        if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (!Unary || !Unary->Node) return;
            if (!IsUnaryOperator(Unary->Node->Type)) { DoubleNegation(Unary->Node, Changed); return; }
            auto* NestedUnary = static_cast<UnaryExpressionNode*>(Unary->Node.get());
            if (!NestedUnary) return;
            ASTNode Grandchild = std::move(NestedUnary->Node);
            ReplaceNode(Node, std::move(Grandchild));
            Changed = true;
        } else if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (!Binary) return;
            DoubleNegation(Binary->Left, Changed);
            DoubleNegation(Binary->Right, Changed);
        }
    }
    static void Annihlation(ASTNode& Node, bool& Changed) {
        if (!Node) return;
        
        if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (!Binary || !Binary->Left || !Binary->Right) return;

            ConstantNode* Constant;
            bool LeftIsConstant;
            
            if (Binary->Left->Type == BooleanAlgebraNodeType::Constant) { Constant = static_cast<ConstantNode*>(Binary->Left.get()); LeftIsConstant = true; }
            else if (Binary->Right->Type == BooleanAlgebraNodeType::Constant) { Constant = static_cast<ConstantNode*>(Binary->Right.get()); LeftIsConstant = false; }
            else { Annihlation(Binary->Left, Changed); Annihlation(Binary->Right, Changed); return; }
            
            if (!Constant) return;
            bool AndAnnihlation = Binary->Type == BooleanAlgebraNodeType::AND && Constant->Value == 0;
            bool OrAnnihlation = Binary->Type == BooleanAlgebraNodeType::OR && Constant->Value == 1;
            if (!(AndAnnihlation || OrAnnihlation)) { Annihlation(Binary->Left, Changed); Annihlation(Binary->Right, Changed); return; }
            
            ReplaceNode(Node, std::move(LeftIsConstant ? Binary->Left : Binary->Right));
            Changed = true;
        } else if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (Unary) Annihlation(Unary->Node, Changed);
        }
    }
    static void Identity(ASTNode& Node, bool& Changed) {
        if (!Node) return;
        
        if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (!Binary || !Binary->Left || !Binary->Right) return;

            ConstantNode* Constant;
            bool LeftIsConstant;
            
            if (Binary->Left->Type == BooleanAlgebraNodeType::Constant) { Constant = static_cast<ConstantNode*>(Binary->Left.get()); LeftIsConstant = true; }
            else if (Binary->Right->Type == BooleanAlgebraNodeType::Constant) { Constant = static_cast<ConstantNode*>(Binary->Right.get()); LeftIsConstant = false; }
            else { Identity(Binary->Left, Changed); Identity(Binary->Right, Changed); return; }
            
            if (!Constant) return;
            bool AndIdentity = Binary->Type == BooleanAlgebraNodeType::AND && Constant->Value == 1;
            bool OrIdentity = Binary->Type == BooleanAlgebraNodeType::OR && Constant->Value == 0;
            if (!(AndIdentity || OrIdentity)) { Identity(Binary->Left, Changed); Identity(Binary->Right, Changed); return; }
            
            ReplaceNode(Node, std::move(LeftIsConstant ? Binary->Right : Binary->Left));
            Changed = true;
        } else if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (Unary) Identity(Unary->Node, Changed);
        }
    }
    static void Idempotence(ASTNode& Node, bool& Changed) {
        if (!Node) return;
        
        if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (!Binary || !Binary->Left || !Binary->Right) return;
            
            if (!BooleanEquivalent(Binary->Left, Binary->Right)) { Idempotence(Binary->Left, Changed); Idempotence(Binary->Right, Changed); return; }
            size_t CostA = GetCost(Binary->Left);
            size_t CostB = GetCost(Binary->Right);
            
            ReplaceNode(Node, std::move(CostA <= CostB ? Binary->Left : Binary->Right));
            Changed = true;
        } else if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (Unary) Idempotence(Unary->Node, Changed);
        }
    }
    static void Complementation(ASTNode& Node, bool& Changed) {
        if (!Node) return;
        
        if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (!Binary || !Binary->Left || !Binary->Right) return;

            if (!(Binary->Type == BooleanAlgebraNodeType::AND || Binary->Type == BooleanAlgebraNodeType::OR)) { Complementation(Binary->Left, Changed); Complementation(Binary->Right, Changed); return; }
            
            UnaryExpressionNode* NegatedNode = nullptr;
            bool LeftIsNegated = false;
            if (Binary->Left->Type == BooleanAlgebraNodeType::NOT) {
                NegatedNode = static_cast<UnaryExpressionNode*>(Binary->Left.get());
                LeftIsNegated = true;
            } else if (Binary->Right->Type == BooleanAlgebraNodeType::NOT) {
                NegatedNode = static_cast<UnaryExpressionNode*>(Binary->Right.get());
                LeftIsNegated = false;
            } else { Complementation(Binary->Left, Changed); Complementation(Binary->Right, Changed); return; }
            
            if (!NegatedNode) return;
            bool Complements = BooleanEquivalent(NegatedNode->Node, LeftIsNegated ? Binary->Right : Binary->Left);
            if (!Complements) { Complementation(Binary->Left, Changed); Complementation(Binary->Right, Changed); return; }
            ASTNode Constant = std::make_unique<ConstantNode>(Binary->Type == BooleanAlgebraNodeType::AND ? 0 : 1);
            
            ReplaceNode(Node, std::move(Constant));
            Changed = true;
        } else if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (Unary) Complementation(Unary->Node, Changed);
        }
    }
    static void Absorption(ASTNode& Node, bool& Changed) {
        if (!Node) return;
        
        if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (!Binary || !Binary->Left || !Binary->Right) return;

            ASTNode Replacement;
            
            if (IsBinaryOperator(Binary->Left->Type)) Absorbs(Replacement, Binary, Binary->Left, Binary->Right);
            if (IsBinaryOperator(Binary->Right->Type)) Absorbs(Replacement, Binary, Binary->Right, Binary->Left);
            
            if (!Replacement) {
                Absorption(Binary->Left, Changed);
                Absorption(Binary->Right, Changed);
                return;
            }
            
            ReplaceNode(Node, std::move(Replacement));
            Changed = true;
        } else if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (Unary) Absorption(Unary->Node, Changed);
        }
    }
    static void DeMorgan(ASTNode& Node, bool& Changed) {
        if (!Node) return;
    
        ASTNode Replacement;
    
        if (DeMorganWorks(Replacement, Node)) {
            ReplaceNode(Node, std::move(Replacement));
            Changed = true;
            return;
        }
    
        if (Node->Type == BooleanAlgebraNodeType::NOT) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (Unary) DeMorgan(Unary->Node, Changed);
            return;
        }
    
        if (!IsBinaryOperator(Node->Type)) return;
    
        auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
        if (!Binary || !Binary->Left || !Binary->Right) return;
    
        if (Node->Type != BooleanAlgebraNodeType::AND &&
            Node->Type != BooleanAlgebraNodeType::OR) {
            DeMorgan(Binary->Left, Changed);
            DeMorgan(Binary->Right, Changed);
            return;
        }
    
        if (Binary->Left->Type != BooleanAlgebraNodeType::NOT ||
            Binary->Right->Type != BooleanAlgebraNodeType::NOT) {
            DeMorgan(Binary->Left, Changed);
            DeMorgan(Binary->Right, Changed);
            return;
        }
    
        auto* UnaryA = static_cast<UnaryExpressionNode*>(Binary->Left.get());
        auto* UnaryB = static_cast<UnaryExpressionNode*>(Binary->Right.get());
    
        if (!UnaryA || !UnaryB || !UnaryA->Node || !UnaryB->Node) return;
    
        BooleanAlgebraNodeType NewType =
            Node->Type == BooleanAlgebraNodeType::AND
                ? BooleanAlgebraNodeType::OR
                : BooleanAlgebraNodeType::AND;
    
        string NewOperator = NewType == BooleanAlgebraNodeType::AND ? "∧" : "∨";
    
        ASTNode Inner = std::make_unique<BinaryExpressionNode>(
            NewType,
            NewOperator,
            std::move(UnaryA->Node),
            std::move(UnaryB->Node)
        );
    
        Node = std::make_unique<UnaryExpressionNode>(
            BooleanAlgebraNodeType::NOT,
            "¬",
            std::move(Inner)
        );
    
        Changed = true;
    }
    static void Distributivity(ASTNode& Node, bool& Changed) {
        if (!Node) return;
        
        ASTNode Replacement;
        
        if (Distributes(Replacement, Node)) {
            ReplaceNode(Node, std::move(Replacement));
            Changed = true;
            return;
        }
        
        if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (!Binary || !Binary->Left || !Binary->Right) return;
            
            if (Node->Type != BooleanAlgebraNodeType::AND &&
                Node->Type != BooleanAlgebraNodeType::OR) {
                Distributivity(Binary->Left, Changed);
                Distributivity(Binary->Right, Changed);
                return;
            }
            
            ASTNode Left, Right, Common;
            
            if (!CanFactor(Node, Left, Right, Common)) {
                Distributivity(Binary->Left, Changed);
                Distributivity(Binary->Right, Changed);
                return;
            }
            
            string Operator = Node->Type == BooleanAlgebraNodeType::AND ? "∨" : "∧";
            
            BooleanAlgebraNodeType Type =
                Node->Type == BooleanAlgebraNodeType::AND
                    ? BooleanAlgebraNodeType::OR
                    : BooleanAlgebraNodeType::AND;
            
            Binary->Left = std::move(Left);
            Binary->Right = std::move(Right);
            
            ASTNode WrappingBinary = std::make_unique<BinaryExpressionNode>(
                Type,
                Operator,
                std::move(Common),
                Node->Clone()
            );
            
            Node = std::move(WrappingBinary);
            Changed = true;
        } else if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (Unary) Distributivity(Unary->Node, Changed);
        }
    }
    static void SimplifyConstants(ASTNode& Node, bool& Changed) {
        if (!Node) return;
        if (IsUnaryOperator(Node->Type)) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (!Unary || !Unary->Node) return;
            if (Unary->Node->Type != BooleanAlgebraNodeType::Constant) { SimplifyConstants(Unary->Node, Changed); return; }
            auto* Constant = static_cast<ConstantNode*>(Unary->Node.get());
            if (!Constant) return;
            ASTNode ConstantAST = std::make_unique<ConstantNode>(!Constant->Value);
            Node = std::move(ConstantAST);
            Changed = true;
        } else if (IsBinaryOperator(Node->Type)) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
            if (!Binary || !Binary->Left || !Binary->Right) return;
            bool Constants = Binary->Left->Type == BooleanAlgebraNodeType::Constant && Binary->Right->Type == BooleanAlgebraNodeType::Constant;
            if (!Constants) { SimplifyConstants(Binary->Left, Changed); SimplifyConstants(Binary->Right, Changed); return;}
            LogicEvaluator::EvaluateEquation(&Node);
            Changed = true;
        }
    }
    
    // --{ Helper Functions }--
    static ASTNode NegateExpression(const ASTNode& Node) {
        if (!Node) return nullptr;
    
        if (Node->Type == BooleanAlgebraNodeType::NOT) {
            auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
            if (!Unary || !Unary->Node) return nullptr;
            return Unary->Node->Clone();
        }
    
        if (Node->Type == BooleanAlgebraNodeType::AND ||
            Node->Type == BooleanAlgebraNodeType::OR) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
    
            if (!Binary || !Binary->Left || !Binary->Right) return nullptr;
    
            BooleanAlgebraNodeType NewType =
                Node->Type == BooleanAlgebraNodeType::AND
                    ? BooleanAlgebraNodeType::OR
                    : BooleanAlgebraNodeType::AND;
    
            string NewOperator =
                NewType == BooleanAlgebraNodeType::AND ? "∧" : "∨";
    
            return std::make_unique<BinaryExpressionNode>(
                NewType,
                NewOperator,
                NegateExpression(Binary->Left),
                NegateExpression(Binary->Right)
            );
        }
    
        return std::make_unique<UnaryExpressionNode>(
            BooleanAlgebraNodeType::NOT,
            "¬",
            Node->Clone()
        );
    }
    static void DistributeAll(ASTNode& Node, const ASTNode& DistributingNode, BooleanAlgebraNodeType Type) {
        if (!Node || !DistributingNode) return;
    
        BooleanAlgebraNodeType ExpectedType =
            Type == BooleanAlgebraNodeType::AND
                ? BooleanAlgebraNodeType::OR
                : BooleanAlgebraNodeType::AND;
    
        if (Node->Type == ExpectedType) {
            auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
    
            if (!Binary || !Binary->Left || !Binary->Right) return;
    
            DistributeAll(Binary->Left, DistributingNode, Type);
            DistributeAll(Binary->Right, DistributingNode, Type);
    
            Binary->Type = Type == BooleanAlgebraNodeType::AND
                ? BooleanAlgebraNodeType::OR
                : BooleanAlgebraNodeType::AND;
    
            Binary->Operator = NodeTypeToOperator(Binary->Type);
            return;
        }
    
        Node = std::make_unique<BinaryExpressionNode>(
            Type,
            NodeTypeToOperator(Type),
            DistributingNode->Clone(),
            Node->Clone()
        );
    }
    static void Absorbs(ASTNode& Replacement, BinaryExpressionNode* Binary, ASTNode& InnerNode, ASTNode& OtherNode) {
        if (!Binary || !InnerNode || !OtherNode) return;
        if (!IsBinaryOperator(InnerNode->Type)) return;
        
        auto* InnerBinary = static_cast<BinaryExpressionNode*>(InnerNode.get());
        if (!InnerBinary || !InnerBinary->Left || !InnerBinary->Right) return;
        if (!((InnerBinary->Type == BooleanAlgebraNodeType::OR && Binary->Type == BooleanAlgebraNodeType::AND) ||
            (InnerBinary->Type == BooleanAlgebraNodeType::AND && Binary->Type == BooleanAlgebraNodeType::OR)))
            return;
        
        ASTNode* BestMatch = nullptr;
        if (BooleanEquivalent(InnerBinary->Left, OtherNode))
            BestMatch = &InnerBinary->Left;
        if (BooleanEquivalent(InnerBinary->Right, OtherNode) && (!BestMatch || GetCost(InnerBinary->Right) < GetCost(*BestMatch)))
            BestMatch = &InnerBinary->Right;
        if (!BestMatch || !*BestMatch) return;
        if (!Replacement || GetCost(*BestMatch) < GetCost(Replacement))
            Replacement = (*BestMatch)->Clone();
    }
    static bool DeMorganWorks(ASTNode& Replacement, ASTNode& Node) {
        if (!Node || Node->Type != BooleanAlgebraNodeType::NOT) return false;
    
        auto* Unary = static_cast<UnaryExpressionNode*>(Node.get());
        if (!Unary || !Unary->Node || !IsBinaryOperator(Unary->Node->Type)) return false;
    
        ASTNode Candidate = NegateExpression(Unary->Node);
        if (!Candidate) return false;
    
        if (!Reduce(Candidate, SkipLaw::DeMorgan)) return false;
        if (GetCost(Candidate) >= GetCost(Node)) return false;
    
        Replacement = std::move(Candidate);
        return true;
    }
    static bool Distributes(ASTNode& Replacement, ASTNode& Node) {
        if (!Node || !IsBinaryOperator(Node->Type)) return false;
    
        if (Node->Type != BooleanAlgebraNodeType::AND &&
            Node->Type != BooleanAlgebraNodeType::OR) return false;
    
        auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
    
        if (!Binary || !Binary->Left || !Binary->Right) return false;
    
        BooleanAlgebraNodeType ExpectedType =
            Node->Type == BooleanAlgebraNodeType::AND
                ? BooleanAlgebraNodeType::OR
                : BooleanAlgebraNodeType::AND;
    
        ASTNode BestCandidate;
        int OriginalCost = GetCost(Node);
    
        auto TryExpand = [&](ASTNode& Expand, ASTNode& Distributing) {
            if (!Expand || Expand->Type != ExpectedType) return;
    
            ASTNode Candidate = Expand->Clone();
    
            DistributeAll(Candidate, Distributing, Node->Type);
    
            Reduce(Candidate, SkipLaw::Distributivity);
    
            int CandidateCost = GetCost(Candidate);
    
            if (CandidateCost >= OriginalCost) return;
    
            if (!BestCandidate || CandidateCost < GetCost(BestCandidate))
                BestCandidate = std::move(Candidate);
        };
    
        TryExpand(Binary->Left, Binary->Right);
        TryExpand(Binary->Right, Binary->Left);
    
        if (!BestCandidate) return false;
    
        Replacement = std::move(BestCandidate);
        return true;
    }
    static bool CanFactor(ASTNode& Node, ASTNode& Left, ASTNode& Right, ASTNode& Common) {
        if (!Node || !IsBinaryOperator(Node->Type)) return false;
    
        auto* Binary = static_cast<BinaryExpressionNode*>(Node.get());
        if (!Binary || !Binary->Left || !Binary->Right) return false;
    
        if (Binary->Type != BooleanAlgebraNodeType::AND &&
            Binary->Type != BooleanAlgebraNodeType::OR) return false;
    
        if (!IsBinaryOperator(Binary->Left->Type) ||
            !IsBinaryOperator(Binary->Right->Type)) return false;
    
        auto* BinaryA = static_cast<BinaryExpressionNode*>(Binary->Left.get());
        auto* BinaryB = static_cast<BinaryExpressionNode*>(Binary->Right.get());
    
        if (!BinaryA || !BinaryA->Left || !BinaryA->Right ||
            !BinaryB || !BinaryB->Left || !BinaryB->Right) return false;
    
        if (BinaryA->Type != BinaryB->Type) return false;
    
        if (!((BinaryA->Type == BooleanAlgebraNodeType::OR &&
                Binary->Type == BooleanAlgebraNodeType::AND) ||
              (BinaryA->Type == BooleanAlgebraNodeType::AND &&
                Binary->Type == BooleanAlgebraNodeType::OR))) return false;
    
        ASTNode* CommonA = nullptr;
        ASTNode* CommonB = nullptr;
        ASTNode* RemainingA = nullptr;
        ASTNode* RemainingB = nullptr;
    
        if (BooleanEquivalent(BinaryA->Left, BinaryB->Left)) {
            CommonA = &BinaryA->Left;
            CommonB = &BinaryB->Left;
            RemainingA = &BinaryA->Right;
            RemainingB = &BinaryB->Right;
        }
        else if (BooleanEquivalent(BinaryA->Left, BinaryB->Right)) {
            CommonA = &BinaryA->Left;
            CommonB = &BinaryB->Right;
            RemainingA = &BinaryA->Right;
            RemainingB = &BinaryB->Left;
        }
        else if (BooleanEquivalent(BinaryA->Right, BinaryB->Left)) {
            CommonA = &BinaryA->Right;
            CommonB = &BinaryB->Left;
            RemainingA = &BinaryA->Left;
            RemainingB = &BinaryB->Right;
        }
        else if (BooleanEquivalent(BinaryA->Right, BinaryB->Right)) {
            CommonA = &BinaryA->Right;
            CommonB = &BinaryB->Right;
            RemainingA = &BinaryA->Left;
            RemainingB = &BinaryB->Left;
        }
    
        if (!CommonA || !CommonB || !RemainingA || !RemainingB || !*CommonA || !*CommonB || !*RemainingA || !*RemainingB) return false;
    
        ASTNode CommonLeft = (*CommonA)->Clone();
        ASTNode CommonRight = (*CommonB)->Clone();
    
        Left = (*RemainingA)->Clone();
        Right = (*RemainingB)->Clone();
    
        Common = std::move(
            GetCost(CommonLeft) <= GetCost(CommonRight)
                ? CommonLeft
                : CommonRight
        );
    
        return true;
    }
    
    unordered_set<SkipLaw> Skipping;
    static bool ReduceOnce(ASTNode& RootNode, bool& Changed, SkipLaw Law) {
        bool LoopChanged = false;
        
        BooleanAlgebraLaws::DoubleNegation(RootNode, LoopChanged);
        BooleanAlgebraLaws::Annihlation(RootNode, LoopChanged);
        BooleanAlgebraLaws::Identity(RootNode, LoopChanged);
        BooleanAlgebraLaws::Idempotence(RootNode, LoopChanged);
        BooleanAlgebraLaws::Complementation(RootNode, LoopChanged);
        BooleanAlgebraLaws::Absorption(RootNode, LoopChanged);
        if (!Skipping.contains(SkipLaw::DeMorgan      )) BooleanAlgebraLaws::DeMorgan(RootNode, LoopChanged);
        if (!Skipping.contains(SkipLaw::Distributivity)) BooleanAlgebraLaws::Distributivity(RootNode, LoopChanged);
        BooleanAlgebraLaws::SimplifyConstants(RootNode, Changed);
        
        Changed = Changed || LoopChanged;
        return LoopChanged;
    }
    static bool Reduce(ASTNode& Node, SkipLaw Law) {
        bool Changed = false;
        bool Inserted = Skipping.insert(Law).second;
        while (ReduceOnce(Node, Changed, Law)) {}
        if (Inserted) Skipping.erase(Law);
        return Changed;
    }
}