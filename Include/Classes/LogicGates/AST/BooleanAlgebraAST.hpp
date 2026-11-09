#pragma once

// --{ Includes }--
#include <stdexcept>
#include <iostream>
#include <string>
#include <memory>
#include "Classes/LogicGates/BooleanAlgebraHelpers.hpp"

// --{ Usings }--
using std::string;
using std::unique_ptr;

// --{ Enums }--
enum class BooleanAlgebraNodeType {
    Variable,
    Constant,
    NOT,
    AND,
    OR,
    XOR,
    XNOR,
    NOR,
    NAND,
    Unknown
};

// --{ Classes/Structs }--
struct BooleanAlgebraNode {
    BooleanAlgebraNodeType Type;
    BooleanAlgebraNode(BooleanAlgebraNodeType Type) : Type(Type) {}
    virtual ~BooleanAlgebraNode() = default;
    virtual void print(int Indent = 0) const = 0;
    virtual unique_ptr<BooleanAlgebraNode> Clone() const = 0;
};
struct VariableNode : BooleanAlgebraNode {
    explicit VariableNode(string Value) : BooleanAlgebraNode(BooleanAlgebraNodeType::Variable), Value(Value) {}
    void print(int Indent) const override { std::cout << string(Indent, '\t') + "Variable: " + Value + "\n"; }
    std::unique_ptr<BooleanAlgebraNode> Clone() const override { return std::make_unique<VariableNode>(*this); }
    string Value;
};
struct ConstantNode : BooleanAlgebraNode {
    explicit ConstantNode(int Value) : BooleanAlgebraNode(BooleanAlgebraNodeType::Constant), Value(Value) { if (Value != 1 && Value != 0) throw std::invalid_argument("Only 0 and 1 are acceptable constants."); }
    void print(int Indent) const override { std::cout << string(Indent, '\t') + "Constant: " << Value << "\n"; }
    std::unique_ptr<BooleanAlgebraNode> Clone() const override { return std::make_unique<ConstantNode>(*this); }
    int Value;
};
struct UnaryExpressionNode : BooleanAlgebraNode {
    UnaryExpressionNode(BooleanAlgebraNodeType Type, string Operator, unique_ptr<BooleanAlgebraNode> Node)
        : BooleanAlgebraNode(Type), Operator(Operator), Node(std::move(Node)) { if (!this->Node) throw std::invalid_argument("Invalid expression, missing Node."); }
    
    void print(int Indent) const override {
        std::cout << std::string(Indent, '\t') << "Unary Expression {\n" << std::string(Indent + 1, '\t') << "Operator: " << Operator << "\n";
        Node->print(Indent + 1);
        std::cout << std::string(Indent, '\t') << "}\n";
    }
    
    std::unique_ptr<BooleanAlgebraNode> Clone() const override { return std::make_unique<UnaryExpressionNode>(
        Type,
        Operator,
        Node->Clone()
    ); }
    
    unique_ptr<BooleanAlgebraNode> Node;
    string Operator;
};
struct BinaryExpressionNode : BooleanAlgebraNode {
    BinaryExpressionNode(BooleanAlgebraNodeType Type, string Operator, unique_ptr<BooleanAlgebraNode> Left, unique_ptr<BooleanAlgebraNode> Right)
        : BooleanAlgebraNode(Type), Operator(Operator), Left(std::move(Left)), Right(std::move(Right)) { if (!this->Left || !this->Right) throw std::invalid_argument("Invalid expression, missing Left/Right node."); }
    
    void print(int Indent) const override {
        std::cout << std::string(Indent, '\t') << "Binary Expression {\n" << std::string(Indent + 1, '\t') << "Operator: " << Operator << "\n";
        Left->print(Indent + 1);
        Right->print(Indent + 1);
        std::cout << std::string(Indent, '\t') << "}\n";
    }
    
    std::unique_ptr<BooleanAlgebraNode> Clone() const override { return std::make_unique<BinaryExpressionNode>(
        Type,
        Operator,
        Left->Clone(),
        Right->Clone()
    ); }
    
    unique_ptr<BooleanAlgebraNode> Left;
    unique_ptr<BooleanAlgebraNode> Right;
    string Operator;
};

// --{ Class Aliases }--
using ASTNode = unique_ptr<BooleanAlgebraNode>;