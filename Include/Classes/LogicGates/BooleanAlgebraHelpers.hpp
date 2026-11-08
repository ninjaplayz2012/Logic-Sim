#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <regex>

using std::vector;
using std::string;
using std::unordered_map;
using std::regex;

namespace BooleanAlgebraHelpers {
    static vector<string> Operators = { "∧", "∨", "↓", "⊕", "⊙", "↑" };
    
    static size_t FindClosed(const string& Equation, size_t Open) {
        size_t Depth = 0;
        
        for (size_t i = Open; i < Equation.size(); i++) {
            if (Equation[i] == '(') Depth++;
            else if (Equation[i] == ')' && --Depth == 0) return i;
        }
        
        return string::npos;
    }
    static bool SplitTopLevel(const string& Equation, string& Left, string& Right, string& Operator) {
        size_t Depth = 0;
        string TestEquation = Equation.substr(1, Equation.size() - 2);
        
        for (size_t i = 0; i < TestEquation.size(); i++) {
            char Character = TestEquation[i];
            if (Character == '(') Depth++;
            else if (Character == ')') Depth--;
            else if (Depth == 0) { 
                for (auto& TestOperator : Operators) {
                    if (TestEquation.compare(i, TestOperator.size(), TestOperator)) continue;
                    Left = TestEquation.substr(0, i); 
                    Right = TestEquation.substr(i + TestOperator.size()); 
                    Operator = TestOperator; 
                    return true;
                }
            }
        }
        
        return false;
    }
    static bool SplitTopLevel(const string& Equation, string& Left, string& Right, string& Operator, size_t& LeftIndex, size_t& RightIndex, size_t& OperatorIndex) {
        size_t Depth = 0;
        string TestEquation = Equation.substr(1, Equation.size() - 2);
        
        for (size_t i = 0; i < TestEquation.size(); i++) {
            char Character = TestEquation[i];
            if (Character == '(') Depth++;
            else if (Character == ')') Depth--;
            else if (Depth == 0) { 
                for (auto& TestOperator : Operators) {
                    if (TestEquation.compare(i, TestOperator.size(), TestOperator)) continue;
                    Left = TestEquation.substr(0, i);                       LeftIndex = 0;
                    Right = TestEquation.substr(i + TestOperator.size());   RightIndex = i + 3;
                    Operator = TestOperator;                                OperatorIndex = i;
                    return true; 
                }
            }
        }
        
        return false;
    }
    static bool IsGroup(const string& Text) {
        return Text.size() >= 2 && Text.front() == '(' && FindClosed(Text, 0) == Text.size() - 1;
    }
}