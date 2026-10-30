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

enum class BooleanLaw {
    DoubleNegation,
    DeMorganFactor,
    DeMorganExpand,
    DeMorgan,
    Complementation,
    Annihilation,
    Identity,
    Idempotence,
    Absorption,
    DistributivityFactor,
    DistributivityExpand,
    Distributivity,
    Associativity,
    Commutativity,
    Parentheses
};

namespace BooleanAlgebraHelpers {
    static size_t FindClosed(const string& Equation, size_t Open) {
        size_t Depth = 0;
        
        for (size_t i = Open; i < Equation.size(); i++) {
            if (Equation[i] == '(') Depth++;
            else if (Equation[i] == ')' && --Depth == 0) return i;
        }
        
        return string::npos;
    }
    
    static bool SplitTopLevel(string& Equation, string& Left, string& Right, string& Operator) {
        size_t Depth = 0;
        
        for (size_t i = 0; i < Equation.size(); i++) {
            char Character = Equation[i];
            if (Character == '(') Depth++;
            else if (Character == ')') Depth--;
            else if (Depth == 0 && (Equation.compare(i, 3, "∧") == 0 || Equation.compare(i, 3, "∨") == 0)) { Left = Equation.substr(0, i); Right = Equation.substr(i + 3); Operator = Equation.substr(i, 3); return true; }
        }
        
        return false;
    }
    
    static bool SplitTopLevel(string& Equation, string& Left, string& Right, string& Operator, size_t& LeftIndex, size_t& RightIndex, size_t& OperatorIndex) {
        size_t Depth = 0;
        
        for (size_t i = 0; i < Equation.size(); i++) {
            char Character = Equation[i];
            if (Character == '(') Depth++;
            else if (Character == ')') Depth--;
            else if (Depth == 0 && (Equation.compare(i, 3, "∧") == 0 || Equation.compare(i, 3, "∨") == 0)) { 
                Left = Equation.substr(0, i);     LeftIndex = 0;
                Right = Equation.substr(i + 3);   RightIndex = i + 3;
                Operator = Equation.substr(i, 3); OperatorIndex = i;
                return true; 
            }
        }
        
        return false;
    }
    
    static bool IsGroup(const string& Text) {
        return Text.size() >= 2 && Text.front() == '(' && FindClosed(Text, 0) == Text.size() - 1;
    }
    
    inline static const string NegationPrefix = "(¬";
    
    // 
    static bool FlattenEquation(string& Equation, vector<string>& Terms, string& FlattenOperator, bool Debug) {
        size_t Depth = 0;
        
        string Left, Right, Operator;
        for (size_t i = 0; i < Equation.size(); i++) {
            char Character = Equation[i];
            if (Character == '(') Depth++;
            else if (Character == ')') Depth--;
            else if (Depth == 1 && (Equation.compare(i, 3, "∧") == 0 || Equation.compare(i, 3, "∨") == 0)) { 
                Left = Equation.substr(1, i - 1); 
                Right = Equation.substr(i + 3, Equation.size() - 1 - (i + 3)); 
                Operator = Equation.substr(i, 3); 
                if (FlattenOperator.empty()) FlattenOperator = Operator;
                break;
            }
        }
        
        if (Debug) {
            std::cout << "Left: " + Left + "\n";
            std::cout << "Right: " + Right + "\n";
            std::cout << "Operator: " + Operator + "\n";
            std::cout << "Flatten Operator: " + FlattenOperator + "\n";
            std::cout << "LeftGroup: "  << (IsGroup(Left ) && Left.compare(0, NegationPrefix.size(), NegationPrefix)  ? "True\n" : "False\n");
            std::cout << "RightGroup: " << (IsGroup(Right) && Right.compare(0, NegationPrefix.size(), NegationPrefix) ? "True\n" : "False\n");
            std::cout << "Correct Operator: " << (Operator == FlattenOperator ? "True\n\n" : "False\n\n");
        }
        
        
        if (Operator.empty() || Operator != FlattenOperator) {Terms.push_back(Equation); return false;}
        if (IsGroup(Left ) && Left.compare(0, NegationPrefix.size(), NegationPrefix) ) FlattenEquation(Left , Terms, FlattenOperator, Debug); else Terms.push_back(Left );
        if (IsGroup(Right) && Right.compare(0, NegationPrefix.size(), NegationPrefix)) FlattenEquation(Right, Terms, FlattenOperator, Debug); else Terms.push_back(Right);
        
        return true;
    }
    
    static string LawToString(BooleanLaw Law) {
        switch ((int)Law) {
            case 0:
                return "DoubleNegation";
                break;
            case 1:
                return "DeMorgan Factor";
                break;
            case 2:
                return "DeMorgan Expand";
                break;
            case 3:
                return "DeMorgan";
                break;
            case 4:
                return "Complementation";
                break;
            case 5:
                return "Annihilation";
                break;
            case 6:
                return "Identity";
                break;
            case 7:
                return "Idempotence";
                break;
            case 8:
                return "Absorption";
                break;
            case 9:
                return "Distributivity Factor";
                break;
            case 10:
                return "Distributivity Expand";
                break;
            case 11:
                return "Distributivity";
                break;
            case 12:
                return "Associativity";
                break;
            case 13:
                return "Commutativity";
                break;
            case 14:
                return "Parentheses";
                break;
            default:
                return "None";
        }
    }
}