#pragma once

// --{ Includes }--
#include <string>
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include "Classes/LRUCache.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraASTTools.hpp"

// --{ Using(s) }--
using std::string;
using namespace BooleanAlgebraASTTools;

// --{ Cache(s) }--
LRUCache<string, bool> ExpressionCache{ 1000 };

// --{ Helper Functions }--
static size_t GetDisplayWidth(const string& Value) {
    size_t Count = 0;
    
    for (unsigned char Character : Value)
        if ((Character & 0xC0) != 0x80)
            Count++;
    
    return Count;
}

// --{ Classes }--
class LogicEvaluator {
    public:
        inline static bool Debug = false;
        
        // --[ String Evaluation ]--
        static bool EvaluateEquation(string Equation, bool Cache = false) {
            ASTNode Node = ParseEquation(Equation);
            if (Debug) {
                std::cout << "Parsed Node: \n";
                Node->print();
                std::cout << "\n";
            }
            
            EvaluateEquation(&Node, Cache);
            if (Node->Type != BooleanAlgebraNodeType::Constant) throw std::runtime_error("Failed to Evaluate Expression: " + Equation + "\n");
            auto* Constant = static_cast<ConstantNode*>(Node.get());
            return Constant->Value;
        }
        static bool CanEvaluate(const string& Equation) {
            ASTNode Node = ParseEquation(Equation);
            return GetVariables(Node).size() == 0;
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
            auto Cached = ExpressionCache.Get(SerializeNode(*Node));
            if (Cached.has_value()) { 
                ASTNode Constant = std::make_unique<ConstantNode>(Cached.value()); 
                *Node = std::move(Constant); 
                return; 
            }
            
            // --( Node Evaluation )--
            BooleanAlgebraNodeType Type = (*Node)->Type;
            
            if (IsUnaryOperator(Type)) {
                if (Type == BooleanAlgebraNodeType::NOT) NOT(Node);
                else throw std::runtime_error("Invalid Unary Node Type");
            } else if (IsBinaryOperator(Type)) {
                if      (Type == BooleanAlgebraNodeType::AND ) AND (Node);
                else if (Type == BooleanAlgebraNodeType::OR  ) OR  (Node);
                else if (Type == BooleanAlgebraNodeType::XOR ) XOR (Node);
                else if (Type == BooleanAlgebraNodeType::XNOR) XNOR(Node);
                else if (Type == BooleanAlgebraNodeType::NOR ) NOR (Node);
                else if (Type == BooleanAlgebraNodeType::NAND) NAND(Node);
                else throw std::runtime_error("Invalid Binary Node Type");
            } else if (Type != BooleanAlgebraNodeType::Constant) {
                if (Type == BooleanAlgebraNodeType::Variable) {
                    auto* Variable = static_cast<VariableNode*>((*Node).get());
                    std::cout << "Value: " << Variable->Value << "\n";
                }
                
                throw std::runtime_error("Tried evaluating an invalid type: " + NodeTypeToString(Type) + "\n");
            }
            
            // --( Caching )--
            if (Cache) {
                auto* Constant = static_cast<ConstantNode*>((*Node).get());
                ExpressionCache.Put(SerializeNode(*Node), Constant->Value);
            }
        }
        static bool CanEvaluate(const ASTNode& Node) { return GetVariables(Node).size() == 0; }
        
        // --[ Saved Logic Gate Evaluation ]--
        
        
        // --[ Truth Tables ]--
        static void PrintTruthTable(const vector<ASTNode>& Nodes) {
            if (Nodes.empty()) return;
            
            // --( Variable Collection )--
            vector<string> Variables;
            unordered_map<string, size_t> VariableIndices;
            vector<VariablesVector> NodeVariables;
            
            for (const auto& Node : Nodes) {
                NodeVariables.push_back(GetVariables(Node));
                
                for (const auto& Variable : NodeVariables.back()) {
                    if (!VariableIndices.contains(Variable.first)) {
                        VariableIndices[Variable.first] = Variables.size();
                        Variables.push_back(Variable.first);
                    }
                }
            }
            
            // --( Table Headers & Width )--
            vector<string> Headers;
            for (const auto& Variable : Variables)
                Headers.push_back(Variable);
            for (const auto& Node : Nodes)
                Headers.push_back(SerializeNode(Node));
            
            vector<size_t> Widths;
            for (const auto& Header : Headers)
                Widths.push_back(GetDisplayWidth(Header));
            
            auto PrintLine = [&]() {
                std::cout << '+';
                for (size_t Width : Widths)
                    std::cout << string(Width + 2, '-') << '+';
                std::cout << '\n';
            };
            auto PrintRow = [&](const vector<string>& Cells) {
                std::cout << '|';
                
                for (size_t i = 0; i < Cells.size(); i++) {
                    size_t CellWidth = GetDisplayWidth(Cells[i]);
                    size_t Padding = Widths[i] - CellWidth;
                    
                    std::cout << ' ' << Cells[i] << string(Padding + 1, ' ') << '|';
                }
                
                std::cout << '\n';
            };
            
            PrintLine();
            PrintRow(Headers);
            PrintLine();
            
            // --( Loop Variables )--
            vector<bool> Values(Variables.size(), false);
            bool Done = false;
            
            // --( Loop )--
            while (!Done) {
                // --/ Cell Data Collection \--
                vector<string> Cells;
                
                for (bool Value : Values)
                    Cells.push_back(Value ? "1" : "0");
                
                // --/ Node Evaluation \--
                for (size_t i = 0; i < Nodes.size(); i++) {
                    ASTNode Clone = Nodes[i]->Clone();
                    auto CloneVariables = GetVariables(Clone);
                    
                    for (size_t j = 0; j < CloneVariables.size(); j++) {
                        bool Value = Values[VariableIndices[CloneVariables[j].first]];
                        
                        for (ASTNode* Variable : CloneVariables[j].second)
                            *Variable = std::make_unique<ConstantNode>(Value);
                    }
                    
                    Cells.push_back(std::to_string(EvaluateEquation(
                        SerializeNode(Clone)
                    )));
                }
                
                // --/ Node Printing \--
                PrintRow(Cells);
                PrintLine();
                
                // --/ Loop Variable Updating \--
                size_t i = 0;
                
                while (i < Values.size() && Values[i]) {
                    Values[i] = false;
                    i++;
                }
                
                if (i == Values.size()) Done = true;
                else Values[i] = true;
            }
        }
        
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
    if (!NodeA || !NodeB) return false;
    
    if (NodeA->Type == BooleanAlgebraNodeType::Variable && NodeB->Type == BooleanAlgebraNodeType::Variable) {
        auto* VariableA = static_cast<VariableNode*>(NodeA.get());
        auto* VariableB = static_cast<VariableNode*>(NodeB.get());
        return VariableA->Value == VariableB->Value;
    }
    if (NodeA->Type == BooleanAlgebraNodeType::Constant && NodeB->Type == BooleanAlgebraNodeType::Constant) {
        auto*  ConstantA = static_cast<ConstantNode*>(NodeA.get());
        auto*  ConstantB = static_cast<ConstantNode*>(NodeB.get());
        return ConstantA->Value == ConstantB->Value;
    }
    
    auto NodeAVariables = GetVariables(NodeA);
    auto NodeBVariables = GetVariables(NodeB);
    
    vector<string> Variables;
    unordered_map<string, size_t> VariableIndices;
    
    for (const auto& Variable : NodeAVariables) {
        VariableIndices[Variable.first] = Variables.size();
        Variables.push_back(Variable.first);
    }
    
    for (const auto& Variable : NodeBVariables) {
        if (!VariableIndices.contains(Variable.first)) {
            VariableIndices[Variable.first] = Variables.size();
            Variables.push_back(Variable.first);
        }
    }
    
    size_t Size = Variables.size();
    vector<bool> Values(Size, false);
    bool Done = false;
    
    while (!Done) {
        ASTNode CloneA = NodeA->Clone();
        ASTNode CloneB = NodeB->Clone();
        
        auto CloneAVariables = GetVariables(CloneA);
        auto CloneBVariables = GetVariables(CloneB);
        
        for (const auto& Variable : CloneAVariables) {
            size_t Index = VariableIndices.at(Variable.first);
            ASTNode Constant = std::make_unique<ConstantNode>(Values[Index]);
            
            for (ASTNode* VariableNode : Variable.second)
                *VariableNode = Constant->Clone();
        }
        
        for (const auto& Variable : CloneBVariables) {
            size_t Index = VariableIndices.at(Variable.first);
            ASTNode Constant = std::make_unique<ConstantNode>(Values[Index]);
            
            for (ASTNode* VariableNode : Variable.second)
                *VariableNode = Constant->Clone();
        }
        
        if (LogicEvaluator::EvaluateEquation(SerializeNode(CloneA)) != LogicEvaluator::EvaluateEquation(SerializeNode(CloneB))) return false;
        
        size_t i = 0;
        
        for (; i < Size && Values[i]; i++)
            Values[i] = false;
        
        if (i == Size)
            Done = true;
        else
            Values[i] = true;
    }

    return true;
}
