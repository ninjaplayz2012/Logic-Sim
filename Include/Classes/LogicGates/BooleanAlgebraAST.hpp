#pragma once

// --{ Includes }--
#include <stdexcept>
#include <iostream>
#include <string>
#include <memory>

// --{ Usings }--
using std::string;
using std::unique_ptr;

// --{ Enums }--
enum class BooleanAlgebraNodeType {
    Variable,
    Constant,
    AND,
    OR,
    NOT,
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
};
struct VariableNode : BooleanAlgebraNode {
    public:
        explicit VariableNode(string Value) : BooleanAlgebraNode(BooleanAlgebraNodeType::Variable), Value(Value) {}
        void print(int Indent) const override { std::cout << string(Indent, '\t') + "Variable: " + Value + "\n"; }
        
    private:
        string Value;
};
struct ConstantNode : BooleanAlgebraNode {
    public:
        explicit ConstantNode(int Value) : BooleanAlgebraNode(BooleanAlgebraNodeType::Constant), Value(Value) {}
        void print(int Indent) const override { std::cout << string(Indent, '\t') + "Constant: " << Value << "\n"; }
        
    private:
        int Value;
};
struct UnaryExpressionNode : BooleanAlgebraNode {
    public:
        UnaryExpressionNode(BooleanAlgebraNodeType Type, string Operator, unique_ptr<BooleanAlgebraNode> Node)
            : BooleanAlgebraNode(Type), Operator(Operator), Node(std::move(Node)) { if (!this->Node) throw std::invalid_argument("Invalid expression, missing Node."); }
        
        void print(int Indent) const override {
            std::cout << std::string(Indent, '\t') << "Unary Expression {\n" << std::string(Indent + 1, '\t') << "Operator: " << Operator << "\n";
            Node->print(Indent + 1);
            std::cout << std::string(Indent, '\t') << "}\n";
        }
    private:
        unique_ptr<BooleanAlgebraNode> Node;
        string Operator;
};
struct BinaryExpressionNode : BooleanAlgebraNode {
    public:
        BinaryExpressionNode(BooleanAlgebraNodeType Type, string Operator, unique_ptr<BooleanAlgebraNode> Left, unique_ptr<BooleanAlgebraNode> Right)
            : BooleanAlgebraNode(Type), Operator(Operator), Left(std::move(Left)), Right(std::move(Right)) { if (!this->Left || !this->Right) throw std::invalid_argument("Invalid expression, missing Left/Right node."); }
        
        void print(int Indent) const override {
            std::cout << std::string(Indent, '\t') << "Binary Expression {\n" << std::string(Indent + 1, '\t') << "Operator: " << Operator << "\n";
            Left->print(Indent + 1);
            Right->print(Indent + 1);
            std::cout << std::string(Indent, '\t') << "}\n";
        }
    private:
        unique_ptr<BooleanAlgebraNode> Left;
        unique_ptr<BooleanAlgebraNode> Right;
        string Operator;
};

// --{ Functions }--
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
static unique_ptr<BooleanAlgebraNode> ParseEquation(const string& Equation) {
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
