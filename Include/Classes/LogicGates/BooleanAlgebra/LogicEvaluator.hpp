#pragma once

// --{ Includes }--
#include <string>
#include <stdexcept>
#include "Classes/LRUCache.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraASTTools.hpp"

// --{ Using(s) }--
using std::string;

// --{ Cache(s) }--
LRUCache<string, bool> ExpressionCache{ 10 };

// --{ Classes }--
class LogicEvaluator {
    public:
        // --[ String Evaluation ]--
        static bool EvaluateEquation(string Equation, bool Cache = false) {
            ASTNode Node = BooleanAlgebraASTTools::ParseEquation(Equation);
            EvaluateEquation(&Node, Cache);
            if (Node->Type != BooleanAlgebraNodeType::Constant) throw std::runtime_error("Failed to Evaluate Expression: " + Equation + "\n");
            auto* Constant = static_cast<ConstantNode*>(Node.get());
            return Constant->Value;
        }
        
        // --[ ASTNode Evaluation ]--
        static bool EvaluateEquation(ASTNode Node, bool Cache = false) { 
            ASTNode ClonedNode = Node->Clone();
            EvaluateEquation(&ClonedNode, Cache); 
            auto* Constant = static_cast<ConstantNode*>(Node.get()); 
            return Constant->Value; 
        }
        
        static void EvaluateEquation(ASTNode* Node, bool Cache = false) { 
            // --( Cache )--
            auto Cached = ExpressionCache.Get(BooleanAlgebraASTTools::SerializeNode(*Node));
            if (Cached.has_value()) { 
                ASTNode Constant = std::make_unique<ConstantNode>(Cached.value()); 
                *Node = std::move(Constant); 
                return; 
            }
            
            // --( Node Evaluation )--
            BooleanAlgebraNodeType Type = (*Node)->Type;
            
            if (BooleanAlgebraASTTools::IsUnaryOperator(Type)) {
                if (Type == BooleanAlgebraNodeType::NOT) NOT(Node);
                else throw std::runtime_error("Invalid Unary Node Type");
            } else if (BooleanAlgebraASTTools::IsBinaryOperator(Type)) {
                if      (Type == BooleanAlgebraNodeType::AND ) AND (Node);
                else if (Type == BooleanAlgebraNodeType::OR  ) OR  (Node);
                else if (Type == BooleanAlgebraNodeType::XOR ) XOR (Node);
                else if (Type == BooleanAlgebraNodeType::XNOR) XNOR(Node);
                else if (Type == BooleanAlgebraNodeType::NOR ) NOR (Node);
                else if (Type == BooleanAlgebraNodeType::NAND) NAND(Node);
                else throw std::runtime_error("Invalid Binary Node Type");
            } else if (Type == BooleanAlgebraNodeType::Variable) {
                auto* Variable = static_cast<VariableNode*>((*Node).get());
                std::cout << "Value: " << Variable->Value << "\n";
                throw std::runtime_error("Tried evaluating an invalid type: " + BooleanAlgebraASTTools::NodeTypeToString(Type) + "\n");
            }
            
            // --( Caching )--
            if (Cache) {
                auto* Constant = static_cast<ConstantNode*>((*Node).get());
                ExpressionCache.Put(BooleanAlgebraASTTools::SerializeNode(*Node), Constant->Value);
            }
        }
        
        // --[ Saved Logic Gate Evaluation ]--
        
    private:
        static void NOT(ASTNode* Node) {
            if ((*Node)->Type != BooleanAlgebraNodeType::NOT) throw std::invalid_argument("Node is not a NOT gate.");
            auto* Unary = static_cast<UnaryExpressionNode*>((*Node).get());
            
            if (Unary->Node->Type != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Unary->Node);
            auto* Constant = static_cast<ConstantNode*>(Unary->Node.get());
            ASTNode NegatedConstant = std::make_unique<ConstantNode>(!Constant->Value);
            
            *Node = std::move(NegatedConstant);
        }
        static void AND(ASTNode* Node) {
            if ((*Node)->Type != BooleanAlgebraNodeType::AND) throw std::invalid_argument("Node is not an AND gate.");
            auto* Binary = static_cast<BinaryExpressionNode*>((*Node).get());
            
            if (Binary->Left->Type  != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Left );
            if (Binary->Right->Type != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Right);
            auto* ConstantA = static_cast<ConstantNode*>(Binary->Left.get() );
            auto* ConstantB = static_cast<ConstantNode*>(Binary->Right.get());
            ASTNode CombinedConstant = std::make_unique<ConstantNode>(ConstantA->Value && ConstantB->Value);
            
            *Node = std::move(CombinedConstant);
        }
        static void OR(ASTNode* Node) {
            if ((*Node)->Type != BooleanAlgebraNodeType::OR) throw std::invalid_argument("Node is not an OR gate.");
            auto* Binary = static_cast<BinaryExpressionNode*>((*Node).get());
            
            if (Binary->Left->Type  != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Left );
            if (Binary->Right->Type != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Right);
            auto* ConstantA = static_cast<ConstantNode*>(Binary->Left.get() );
            auto* ConstantB = static_cast<ConstantNode*>(Binary->Right.get());
            ASTNode CombinedConstant = std::make_unique<ConstantNode>(ConstantA->Value || ConstantB->Value);
            
            *Node = std::move(CombinedConstant);
        }
        static void XOR(ASTNode* Node) {
            if ((*Node)->Type != BooleanAlgebraNodeType::XOR) throw std::invalid_argument("Node is not a XOR gate.");
            auto* Binary = static_cast<BinaryExpressionNode*>((*Node).get());
            
            if (Binary->Left->Type  != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Left );
            if (Binary->Right->Type != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Right);
            auto* ConstantA = static_cast<ConstantNode*>(Binary->Left.get() );
            auto* ConstantB = static_cast<ConstantNode*>(Binary->Right.get());
            ASTNode CombinedConstant = std::make_unique<ConstantNode>(ConstantA->Value != ConstantB->Value);
            
            *Node = std::move(CombinedConstant);
        }
        static void XNOR(ASTNode* Node) {
            if ((*Node)->Type != BooleanAlgebraNodeType::XNOR) throw std::invalid_argument("Node is not a XNOR gate.");
            auto* Binary = static_cast<BinaryExpressionNode*>((*Node).get());
            
            if (Binary->Left->Type  != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Left );
            if (Binary->Right->Type != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Right);
            auto* ConstantA = static_cast<ConstantNode*>(Binary->Left.get() );
            auto* ConstantB = static_cast<ConstantNode*>(Binary->Right.get());
            ASTNode CombinedConstant = std::make_unique<ConstantNode>(ConstantA->Value == ConstantB->Value);
            
            *Node = std::move(CombinedConstant);
        }
        static void NOR(ASTNode* Node) {
            if ((*Node)->Type != BooleanAlgebraNodeType::NOR) throw std::invalid_argument("Node is not a NOR gate.");
            auto* Binary = static_cast<BinaryExpressionNode*>((*Node).get());
            
            if (Binary->Left->Type  != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Left );
            if (Binary->Right->Type != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Right);
            auto* ConstantA = static_cast<ConstantNode*>(Binary->Left.get() );
            auto* ConstantB = static_cast<ConstantNode*>(Binary->Right.get());
            ASTNode CombinedConstant = std::make_unique<ConstantNode>(!(ConstantA->Value || ConstantB->Value));
            
            *Node = std::move(CombinedConstant);
        }
        static void NAND(ASTNode* Node) {
            if ((*Node)->Type != BooleanAlgebraNodeType::NAND) throw std::invalid_argument("Node is not a NAND gate.");
            auto* Binary = static_cast<BinaryExpressionNode*>((*Node).get());
            
            if (Binary->Left->Type  != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Left );
            if (Binary->Right->Type != BooleanAlgebraNodeType::Constant) EvaluateEquation(&Binary->Right);
            auto* ConstantA = static_cast<ConstantNode*>(Binary->Left.get() );
            auto* ConstantB = static_cast<ConstantNode*>(Binary->Right.get());
            ASTNode CombinedConstant = std::make_unique<ConstantNode>(!(ConstantA->Value && ConstantB->Value));
            
            *Node = std::move(CombinedConstant);
        }
};

// --{ Methods }--
static bool BooleanAlgebraASTTools::BooleanEquivalent(const ASTNode& NodeA, const ASTNode& NodeB) {
    // --( Nullptr Check )--
    if (!NodeA || !NodeB) return false;
    
    // --( Variable Collection )--
    auto NodeAVariables = GetVariables(NodeA);
    auto NodeBVariables = GetVariables(NodeB);
    
    if (NodeAVariables.size() != NodeBVariables.size()) return false;
    size_t Size = NodeAVariables.size();
    
    // --( Indices Mapping )--
    unordered_map<string, size_t> NodeBIndices;
    
    for (size_t j = 0; j < Size; j++) 
        NodeBIndices[NodeBVariables[j].first] = j;
    for (const auto& Variable : NodeAVariables) 
        if (!NodeBIndices.contains(Variable.first)) return false;
    
    // --( Loop )--
    
    // --/ Loop Variable Initialization \--
    vector<bool> Values(Size, false);
    bool Done = false;
    size_t i;
    
    // --/ Main Loop \--
    while (!Done) {
        // --< Cloning >--
        ASTNode CloneA = NodeA->Clone();
        ASTNode CloneB = NodeB->Clone();
        
        auto CloneAVariables = GetVariables(CloneA);
        auto CloneBVariables = GetVariables(CloneB);
        
        // --< Constant Replacing >--
        for (size_t j = 0; j < Size; j++) {
            ASTNode ConstantA = std::make_unique<ConstantNode>(Values[j]);
            ASTNode ConstantB = std::make_unique<ConstantNode>(Values[j]);
            
            for (ASTNode* Variable : CloneAVariables[j].second) {
                *Variable = ConstantA->Clone();
            }
            
            size_t MatchingIndex = NodeBIndices[CloneAVariables[j].first];
            
            for (ASTNode* Variable : CloneBVariables[MatchingIndex].second) {
                *Variable = ConstantB->Clone();
            }
        }
        
        // --< Evaluation >--
        if (LogicEvaluator::EvaluateEquation(SerializeNode(CloneA)) != 
            LogicEvaluator::EvaluateEquation(SerializeNode(CloneB))) return false;
        
        // --< Loop Conditions >--
        for (i = 0; i < Size && Values[i]; i++) {
            Values[i] = false;
        }
        
        if (i == Size) Done = true;
        else Values[i] = true;
    }
    
    return true;
}