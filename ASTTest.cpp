#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "Classes/LogicGates/AST/BooleanAlgebraAST.hpp"
#include "Classes/LogicGates/AST/BooleanAlgebraASTTools.hpp"

using namespace BooleanAlgebraASTTools;

int Passed = 0;
int Failed = 0;

void Test(const std::string& Name, bool Result) {
    if (Result) {
        std::cout << "[PASS] " << Name << '\n';
        Passed++;
    } else {
        std::cout << "[FAIL] " << Name << '\n';
        Failed++;
    }
}

void TestParsing(const std::string& Name, const std::string& Equation) {
    try {
        ASTNode Node = ParseEquation(Equation);

        Test(Name, Node != nullptr && ValidateNode(Node));
    } catch (...) {
        Test(Name, false);
    }
}

void TestSerialization(const std::string& Name, const std::string& Equation) {
    try {
        ASTNode Node = ParseEquation(Equation);

        Test(Name, SerializeNode(Node) == Equation);
    } catch (...) {
        Test(Name, false);
    }
}

void TestRoundTrip(const std::string& Name, const std::string& Equation) {
    try {
        ASTNode Original = ParseEquation(Equation);

        if (!Original || !ValidateNode(Original)) {
            Test(Name, false);
            return;
        }

        std::string Serialized = SerializeNode(Original);
        ASTNode Deserialized = ParseEquation(Serialized);

        Test(Name,
            Deserialized &&
            ValidateNode(Deserialized) &&
            StructurallyEqual(Original, Deserialized) &&
            SerializeNode(Deserialized) == Equation
        );
    } catch (...) {
        Test(Name, false);
    }
}

int main() {
    std::cout << "\n========== AST PARSING TESTS ==========\n\n";

    // --{ StringToNodeType }--

    {
        std::vector<std::pair<std::string, BooleanAlgebraNodeType>> Tests = {
            {"¬", BooleanAlgebraNodeType::NOT},
            {"∧", BooleanAlgebraNodeType::AND},
            {"∨", BooleanAlgebraNodeType::OR},
            {"↓", BooleanAlgebraNodeType::NOR},
            {"⊕", BooleanAlgebraNodeType::XOR},
            {"⊙", BooleanAlgebraNodeType::XNOR},
            {"↑", BooleanAlgebraNodeType::NAND},
            {"Constant", BooleanAlgebraNodeType::Constant},
            {"Variable", BooleanAlgebraNodeType::Variable},
            {"Unknown", BooleanAlgebraNodeType::Unknown},
            {"Invalid", BooleanAlgebraNodeType::Unknown},
            {"", BooleanAlgebraNodeType::Unknown}
        };

        for (auto& [Input, Expected] : Tests) {
            Test("StringToNodeType: " + Input,
                StringToNodeType(Input) == Expected);
        }
    }

    // --{ Basic Parsing }--

    TestParsing("Parse variable A", "A");
    TestParsing("Parse variable B", "B");
    TestParsing("Parse multi-character variable", "Variable123");

    TestParsing("Parse constant 0", "0");
    TestParsing("Parse constant 1", "1");

    // --{ Unary Parsing }--

    TestParsing("Parse NOT A", "(¬A)");
    TestParsing("Parse NOT B", "(¬B)");
    TestParsing("Parse NOT 0", "(¬0)");
    TestParsing("Parse NOT 1", "(¬1)");

    TestParsing("Parse NOT of binary expression",
        "(¬(A∧B))");

    TestParsing("Parse nested NOT",
        "(¬(¬A))");

    TestParsing("Parse triple NOT",
        "(¬(¬(¬A)))");

    // --{ Binary Parsing }--

    TestParsing("Parse AND", "(A∧B)");
    TestParsing("Parse OR", "(A∨B)");
    TestParsing("Parse XOR", "(A⊕B)");
    TestParsing("Parse XNOR", "(A⊙B)");
    TestParsing("Parse NOR", "(A↓B)");
    TestParsing("Parse NAND", "(A↑B)");

    TestParsing("Parse constant AND", "(1∧0)");
    TestParsing("Parse constant OR", "(0∨1)");
    TestParsing("Parse constant XOR", "(1⊕1)");

    // --{ Nested Parsing }--

    TestParsing("Parse left-nested expression",
        "((A∧B)∨C)");

    TestParsing("Parse right-nested expression",
        "(A∧(B∨C))");

    TestParsing("Parse both sides nested",
        "((A∧B)∨(C∧D))");

    TestParsing("Parse nested unary and binary",
        "((¬A)∧(B∨C))");

    TestParsing("Parse deeply nested expression",
        "((((A∧B)∨C)⊕D)↑(¬E))");

    TestParsing("Parse mixed operators",
        "((A∧B)∨((C⊕D)↓(¬E)))");

    // --{ Serialization }--

    TestSerialization("Serialize variable", "A");
    TestSerialization("Serialize constant 0", "0");
    TestSerialization("Serialize constant 1", "1");

    TestSerialization("Serialize NOT", "(¬A)");
    TestSerialization("Serialize AND", "(A∧B)");
    TestSerialization("Serialize OR", "(A∨B)");
    TestSerialization("Serialize XOR", "(A⊕B)");
    TestSerialization("Serialize XNOR", "(A⊙B)");
    TestSerialization("Serialize NOR", "(A↓B)");
    TestSerialization("Serialize NAND", "(A↑B)");

    TestSerialization("Serialize nested expression",
        "((A∧B)∨(¬C))");

    TestSerialization("Serialize deeply nested expression",
        "((((A∧B)∨C)⊕D)↑(¬E))");

    // --{ Round Trip }--

    TestRoundTrip("Round trip variable", "A");
    TestRoundTrip("Round trip constant 0", "0");
    TestRoundTrip("Round trip constant 1", "1");

    TestRoundTrip("Round trip NOT", "(¬A)");
    TestRoundTrip("Round trip AND", "(A∧B)");
    TestRoundTrip("Round trip OR", "(A∨B)");
    TestRoundTrip("Round trip XOR", "(A⊕B)");
    TestRoundTrip("Round trip XNOR", "(A⊙B)");
    TestRoundTrip("Round trip NOR", "(A↓B)");
    TestRoundTrip("Round trip NAND", "(A↑B)");

    TestRoundTrip("Round trip nested expression",
        "((A∧B)∨(¬C))");

    TestRoundTrip("Round trip deeply nested expression",
        "((((A∧B)∨C)⊕D)↑(¬E))");

    TestRoundTrip("Round trip mixed expression",
        "((A↓(B⊕C))∧((¬D)↑E))");

    // --{ Results }--

    std::cout << "\n========== TEST RESULTS ==========\n";
    std::cout << "Passed: " << Passed << '\n';
    std::cout << "Failed: " << Failed << '\n';
    std::cout << "Total:  " << Passed + Failed << '\n';

    return Failed == 0 ? 0 : 1;
}