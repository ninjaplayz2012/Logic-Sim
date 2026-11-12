#pragma once

// --{ Includes }--
#include <string>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <utility>
#include <iostream>
#include "Classes/LRUCache.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraASTTools.hpp"

// --{ Using(s) }--
using std::string, std::vector, std::pair;
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

// --{ Enums }--
enum class OperationType {
    AND,
    OR,
    NOT,
    XOR,
    XNOR,
    NAND,
    NOR,
    PUSH
};

// --{ Classes }--
struct Operation {
    OperationType OPCode;
    bool Input;
};

struct CompiledExpression {
    vector<Operation> Operations;
    vector<pair<size_t, size_t>> VariablePushes;
};

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
            EvaluateEquation(&Node, Cache); 
            auto* Constant = static_cast<ConstantNode*>(Node.get()); 
            return Constant->Value; 
        }
        static void EvaluateEquation(ASTNode* Node, bool Cache = false) { 
            if (!CanEvaluate(*Node)) return;
            
            // --( Cache )--
            auto Cached = ExpressionCache.Get(SerializeNode(*Node));
            if (Cached.has_value()) { 
                ASTNode Constant = std::make_unique<ConstantNode>(Cached.value()); 
                *Node = std::move(Constant); 
                return; 
            }
            
            // --( Node Evaluation )--
            vector<Operation> Operations = ConvertToByteCode(*Node);
            bool Value = EvaluateByteCode(Operations);
            
            // --( Caching )--
            if (Cache) {
                ExpressionCache.Put(SerializeNode(*Node), Value);
            }
            
            // --( Updating )--
            *Node = std::make_unique<ConstantNode>(Value);
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
        
        static vector<Operation> ConvertToByteCode(const ASTNode& Node) {
            vector<Operation> Operations;
            ConvertToByteCode(Node, Operations);
            return Operations;
        }
        static void ConvertToByteCode(const ASTNode& Node, vector<Operation>& Operations) {
            if (!Node) throw std::invalid_argument("Cannot compile a null AST node.");
        
            if (Node->Type == BooleanAlgebraNodeType::Constant) {
                auto* Constant = static_cast<const ConstantNode*>(Node.get());
                Operations.emplace_back(OperationType::PUSH, Constant->Value);
            }
            else if (IsUnaryOperator(Node->Type)) {
                auto* Unary = static_cast<const UnaryExpressionNode*>(Node.get());
                ConvertToByteCode(Unary->Node, Operations);
                Operations.push_back({ OperationType::NOT, {} });
            }
            else if (IsBinaryOperator(Node->Type)) {
                auto* Binary = static_cast<const BinaryExpressionNode*>(Node.get());
                ConvertToByteCode(Binary->Left, Operations);
                ConvertToByteCode(Binary->Right, Operations);
        
                OperationType OPCode;
                switch (Node->Type) {
                    case BooleanAlgebraNodeType::AND:  OPCode = OperationType::AND;  break;
                    case BooleanAlgebraNodeType::OR:   OPCode = OperationType::OR;   break;
                    case BooleanAlgebraNodeType::XOR:  OPCode = OperationType::XOR;  break;
                    case BooleanAlgebraNodeType::XNOR: OPCode = OperationType::XNOR; break;
                    case BooleanAlgebraNodeType::NAND: OPCode = OperationType::NAND; break;
                    case BooleanAlgebraNodeType::NOR:  OPCode = OperationType::NOR;  break;
                    default: throw std::runtime_error("Invalid Binary Node Type.");
                }
                Operations.push_back({ OPCode, {} });
            }
            else throw std::runtime_error("Cannot compile invalid AST node type.");
        }
        
        static void CompileExpression(const ASTNode& Node, const unordered_map<string, size_t>& VariableIndices, CompiledExpression& Result) {
            if (!Node) throw std::invalid_argument("AST contains a null node.");
            
            if (Node->Type == BooleanAlgebraNodeType::Variable) {
                auto* Variable = static_cast<const VariableNode*>(Node.get());
                size_t VariableIndex = VariableIndices.at(Variable->Value);
                size_t InstructionIndex = Result.Operations.size();
                
                Result.Operations.push_back({ OperationType::PUSH, false });
                Result.VariablePushes.emplace_back(InstructionIndex, VariableIndex);
            } else if (Node->Type == BooleanAlgebraNodeType::Constant) {
                auto* Constant = static_cast<const ConstantNode*>(Node.get());
                Result.Operations.push_back({ OperationType::PUSH, Constant->Value });
            } else if (IsUnaryOperator(Node->Type)) {
                auto* Unary = static_cast<const UnaryExpressionNode*>(Node.get());
                
                CompileExpression(Unary->Node, VariableIndices, Result);
                
                Result.Operations.push_back({ OperationType::NOT, false });
            } else if (IsBinaryOperator(Node->Type)){
                auto* Binary = static_cast<const BinaryExpressionNode*>(Node.get());
                
                CompileExpression(Binary->Left , VariableIndices, Result);
                CompileExpression(Binary->Right, VariableIndices, Result);
                
                OperationType OPCode;
                
                switch (Node->Type) {
                    case BooleanAlgebraNodeType::AND:  OPCode = OperationType::AND;  break;
                    case BooleanAlgebraNodeType::OR:   OPCode = OperationType::OR;   break;
                    case BooleanAlgebraNodeType::XOR:  OPCode = OperationType::XOR;  break;
                    case BooleanAlgebraNodeType::XNOR: OPCode = OperationType::XNOR; break;
                    case BooleanAlgebraNodeType::NAND: OPCode = OperationType::NAND; break;
                    case BooleanAlgebraNodeType::NOR:  OPCode = OperationType::NOR;  break;
                    default: throw std::runtime_error("Invalid Binary Node Type.");
                }
                
                Result.Operations.push_back({ OPCode, false });
            } else throw std::runtime_error("Cannot compile invalid AST node type.");
        }
        static CompiledExpression CompileExpression(const ASTNode& RootNode, const unordered_map<string, size_t>& VariableIndices) {
            CompiledExpression Result;
            CompileExpression(RootNode, VariableIndices, Result);
            return Result;
        };
        
        static bool EvaluateByteCode(vector<Operation>& Operations);
        static bool EvaluateAndCompareByteCode(vector<Operation>& OperationsA, vector<Operation>& OperationsB);
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
        auto* ConstantA = static_cast<ConstantNode*>(NodeA.get());
        auto* ConstantB = static_cast<ConstantNode*>(NodeB.get());
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
    
    CompiledExpression CompiledA = LogicEvaluator::CompileExpression(NodeA, VariableIndices);
    CompiledExpression CompiledB = LogicEvaluator::CompileExpression(NodeB, VariableIndices);
    
    size_t Size = Variables.size();
    vector<bool> Values(Size, false);
    
    bool Done = false;
    
    while (!Done) {
        for (const auto& [InstructionIndex, VariableIndex] : CompiledA.VariablePushes)
            CompiledA.Operations[InstructionIndex].Input = Values[VariableIndex];
        
        for (const auto& [InstructionIndex, VariableIndex] : CompiledB.VariablePushes)
            CompiledB.Operations[InstructionIndex].Input = Values[VariableIndex];
        
        if (!LogicEvaluator::EvaluateAndCompareByteCode(CompiledA.Operations, CompiledB.Operations)) return false;
        
        size_t i = 0;
        
        while (i < Size && Values[i]) {
            Values[i] = false;
            i++;
        }
        
        if (i == Size)
            Done = true;
        else
            Values[i] = true;
    }
    
    return true;
}