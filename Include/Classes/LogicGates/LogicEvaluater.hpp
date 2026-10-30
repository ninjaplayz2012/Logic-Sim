#pragma once
#include "Classes/LogicGates/LogicGates.hpp"
#include "Classes/LogicGates/BooleanAlgebraHelpers.hpp"
#include <vector>
#include <regex>
#include <iterator>
#include <algorithm>
#include <windows.h>
#include <unordered_set>
#include <fstream>

using std::string;
using std::vector;
using std::regex;
using std::unordered_set;

class LogicEvaluator {
    public:
        inline static bool Debug = false;
        inline static bool DeepDebug = true;
        inline static bool RecursiveDebug = false;
        inline static int RecursiveDebugDepth = 3;
        
        static string SimplifyEquation(LogicGate* Gate) { return SimplifyEquation(Gate->GetEquation()); }
        static string SimplifyEquation(string Equation) { SimplifyEquation(&Equation); return Equation; }
        static void SimplifyEquation(string* Equation) {
            unordered_set<string> Seen;
            while (Seen.insert(*Equation).second) SimplifyOnce(*Equation, Debug);
        }
        
    private:
        static void SimplifyOnce(string& Equation, bool SimplifyDebug) {
            if (SimplifyDebug) std::cout << "(Pre-DoubleNegation) Equation: " + Equation + "\n";
            if (Equation.find("(¬(¬") != string::npos) HandleDoubleNegation(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-DoubleNegation) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-DeMorgan) Equation: " + Equation + "\n";
            if (Equation.find(DeMorganPrefixExpand) != string::npos || Equation.find(DeMorganPrefixFactor) != string::npos) HandleDeMorgan(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-DeMorgan) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-Complementation) Equation: " + Equation + "\n";
            HandleComplementation(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-Complementation) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-Annihilation) Equation: " + Equation + "\n";
            HandleAnnihilation(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-Annihilation) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-Identity) Equation: " + Equation + "\n";
            HandleIdentity(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-Identity) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-Idempotence) Equation: " + Equation + "\n";
            HandleIdempotence(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-Idempotence) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-Absorption) Equation: " + Equation + "\n";
            HandleAbsorption(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-Absorption) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-Distributivity) Equation: " + Equation + "\n";
            HandleDistributivity(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-Distributivity) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-Associativity) Equation: " + Equation + "\n";
            HandleAssociativity(Equation, DeepDebug);
            if (SimplifyDebug) std::cout << "(Post-Associativity) Equation: " + Equation + "\n";
            
            if (SimplifyDebug) std::cout << "(Pre-Parentheses) Equation: " + Equation + "\n";
            HandleParentheses(Equation);
            if (SimplifyDebug) std::cout << "(Post-Parentheses) Equation: " + Equation + "\n";
        }
        
        // --[ Regex ]--
        
        // --( Double Negation )--
        inline static const string DoubleNegationPrefix = "(¬(¬";
        
        // --( DeMorgan )--
        inline static const string DeMorganPrefixFactor = "((¬";
        inline static const string DeMorganPrefixExpand = "(¬(";
        inline static const regex DeMorganRegex{ R"(^\(¬(.+)\)$)" };
        
        // --( Complementation Regex )--
        inline static const string ComplementationPrefix = "(¬";
        
        // --( Parentheses Regex )--
        inline static const regex ParenthesesRegex1{ R"(\(([^¬∨∧\n]+?)\))" };
        
        // --[ Helper Functions ]--
        inline static unordered_set<BooleanLaw> Skipping{};
        inline static int Depth = 0;
        
        static size_t Cost(const string& Equation) {
            size_t OperatorCount = 0;
            for (const char* Op : { "∧", "∨", "¬" })
                for (size_t OperatorIndex = Equation.find(Op); OperatorIndex != string::npos; OperatorIndex = Equation.find(Op, OperatorIndex + 1)) OperatorCount++;
            return OperatorCount;
        }
        static string Reduce(string Equation, BooleanLaw SkipLaw) {
            unordered_set<string> Seen;
            Depth++;
            while (Seen.insert(Equation).second) Equation = ReduceOnce(Equation, SkipLaw);
            Depth--;
            return Equation;
        }
        static string ReduceOnce(string Equation, BooleanLaw SkipLaw) {
            const bool Inserted = Skipping.insert(SkipLaw).second;
            bool DebugReduce = RecursiveDebug && (RecursiveDebugDepth <= Depth);
            
            // if (DebugReduce) {
                std::cout << "Reducing:\n";
                std::cout << "Skip Law: " + BooleanAlgebraHelpers::LawToString(SkipLaw) + "\n";
                std::cout << "Skipping: {\n";
                for (auto& Law : Skipping) std::cout << "\t" + BooleanAlgebraHelpers::LawToString(Law) + "\n";
                std::cout << "}\n";
                // std::cout << "Equation: " + Equation;
            // }
            
            if (!Skipping.contains(BooleanLaw::DoubleNegation )) HandleDoubleNegation(Equation, DebugReduce);
            if (!Skipping.contains(BooleanLaw::DeMorgan       )) HandleDeMorgan(Equation, DebugReduce);
            if (!Skipping.contains(BooleanLaw::Complementation)) HandleComplementation(Equation, DebugReduce);
            if (!Skipping.contains(BooleanLaw::Annihilation   )) HandleAnnihilation(Equation, DebugReduce);
            if (!Skipping.contains(BooleanLaw::Identity       )) HandleIdentity(Equation, DebugReduce);
            if (!Skipping.contains(BooleanLaw::Idempotence    )) HandleIdempotence(Equation, DebugReduce);
            if (!Skipping.contains(BooleanLaw::Absorption     )) HandleAbsorption(Equation, DebugReduce);
            if (!Skipping.contains(BooleanLaw::Distributivity )) HandleDistributivity(Equation, DebugReduce);
            if (!Skipping.contains(BooleanLaw::Associativity  )) HandleAssociativity(Equation, DebugReduce);
            
            if (DebugReduce) std::cout << "Done Reducing: " + Equation + "\n";
            if (Inserted) Skipping.erase(SkipLaw);
            return Equation;
        }
        static bool TestLaw(string Equation, BooleanLaw SkipLaw) {
            return Reduce(Equation, SkipLaw) != Equation;
        }
        
        static void HandleDeMorganExpand(string& Equation, bool DebugDeMorgan) {
            // ¬(A ∧ B) -> ¬A ∨ ¬B or ¬(A ∨ B) -> ¬A ∧ ¬B
            string NewEquation;
            size_t Index = 0;
            
            if (DebugDeMorgan) std::cout << "Expanding DeMorgan:\n";
            
            while (Index < Equation.size()) {
                if (Equation.compare(Index, DeMorganPrefixExpand.size(), DeMorganPrefixExpand)) { NewEquation += Equation[Index]; Index++; continue; }
                const size_t ClosingParenthesesIndex = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                const size_t InnerOpenParentheses = Index + DeMorganPrefixExpand.size() - 1;
                
                if (ClosingParenthesesIndex == string::npos || BooleanAlgebraHelpers::FindClosed(Equation, InnerOpenParentheses) != ClosingParenthesesIndex - 1) { NewEquation += Equation[Index]; Index++; continue; }
                string Left, Right, Operator;
                string Inside = Equation.substr(InnerOpenParentheses + 1, ClosingParenthesesIndex - (InnerOpenParentheses + 2));
                
                if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                const string FlippedOperator = (Operator == "∨" ? "∧" : "∨");
                const string Replacement = "((¬" + Left + ")" + FlippedOperator + "(¬" + Right + "))";
                const string TrialEquation = NewEquation + Replacement + Equation.substr(ClosingParenthesesIndex + 1);
                
                if (Cost(Reduce(TrialEquation, BooleanLaw::DeMorganFactor)) >= Cost(Equation)) { NewEquation += Equation[Index]; Index++; continue; }
                
                if (DebugDeMorgan) {
                    std::cout << "Found DeMorgan Expanding: {\n";
                    std::cout << "\tIndex: " << Index << "–" << ClosingParenthesesIndex << "\n";
                    std::cout << "\tVersion: " << (Operator == "∧" ? "AND" : "OR") << "\n";
                    std::cout << "\tEquation: " + Inside + "\n";
                    std::cout << "\tReplacement: " + Replacement + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += Replacement;
                Index = ClosingParenthesesIndex + 1;
            }
            
            Equation = NewEquation;
        }
        static void HandleDeMorganFactor(string& Equation, bool DebugDeMorgan) {
            // ((¬A) ∨ (¬B)) -> (¬(A ∧ B)) or ((¬A) ∧ (¬B)) -> (¬(A ∨ B))
            string NewEquation;
            size_t Index = 0;
            
            if (DebugDeMorgan) std::cout << "Factoring DeMorgan:\n";
            
            while (Index < Equation.size()) {
                if (Equation.compare(Index, DeMorganPrefixFactor.size(), DeMorganPrefixFactor)) { NewEquation += Equation[Index]; Index++; continue; }
                const size_t ClosingParenthesesIndex = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                const size_t StartInside = Index + 1;
                
                if (ClosingParenthesesIndex == string::npos) { NewEquation += Equation[Index]; Index++; continue; }
                string Left, Right, Operator;
                string Inside = Equation.substr(StartInside, ClosingParenthesesIndex - StartInside);
                
                if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                if (std::regex_match(Left , DeMorganRegex)) Left  = std::regex_replace(Left , DeMorganRegex, "$1"); else { NewEquation += Equation[Index]; Index++; continue; }
                if (std::regex_match(Right, DeMorganRegex)) Right = std::regex_replace(Right, DeMorganRegex, "$1"); else { NewEquation += Equation[Index]; Index++; continue; }
                
                const string FlippedOperator = (Operator == "∨" ? "∧" : "∨");
                const string Replacement = "(¬(" + Left + FlippedOperator + Right + "))";
                const string Collapsed = NewEquation + Replacement + Equation.substr(ClosingParenthesesIndex + 1);
                
                if (Cost(Reduce(Equation, BooleanLaw::DeMorgan)) < Cost(Collapsed)) { NewEquation += Equation[Index]; Index++; continue; }
                if (DebugDeMorgan) {
                    std::cout << "Found DeMorgan Factoring: {\n";
                    std::cout << "\tIndex: " << Index << "–" << ClosingParenthesesIndex << "\n";
                    std::cout << "\tVersion: " << (Operator == "∧" ? "AND" : "OR") << "\n";
                    std::cout << "\tEquation: " + Inside + "\n";
                    std::cout << "\tReplacement: " + Replacement + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += Replacement;
                Index = ClosingParenthesesIndex + 1;
            }
            
            Equation = NewEquation;
        }
        
        static bool Absorbs(const string& Plain, const string& Group, const string& OuterOperator) {
            if (!BooleanAlgebraHelpers::IsGroup(Group)) return false;
            
            string Inside = Group.substr(1, Group.size() - 2);
            string InnerLeft, InnerRight, InnerOperator;
            
            if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, InnerLeft, InnerRight, InnerOperator)) return false;
            if (InnerOperator == OuterOperator) return false;
            
            return InnerLeft == Plain || InnerRight == Plain;
        }
        static bool Distributes(const string& Plain, const string& Group, const string& OuterOperator, string& InnerLeft, string& InnerRight, string& InnerOperator) {
            if (!BooleanAlgebraHelpers::IsGroup(Group)) return false;
            
            string TempLeft, TempRight, TempOperator;
            string Inside = Group.substr(1, Group.size() - 2);
            if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, TempLeft, TempRight, TempOperator)) return false;
            if (OuterOperator == TempOperator) return false;
            InnerLeft = TempLeft;
            InnerRight = TempRight;
            InnerOperator = TempOperator;
            
            return true;
        }
        static bool Complements(const string& TermUno, const string& TermDos, bool& NegatingTerm) {
            if (TermUno == TermDos) return false;
            if (TermUno.compare(0, ComplementationPrefix.size(), ComplementationPrefix) == 0 && TermDos.compare(0, ComplementationPrefix.size(), ComplementationPrefix) == 0) return false;
            NegatingTerm = TermUno.compare(0, ComplementationPrefix.size(), ComplementationPrefix) == 0 ? true : false;
            return NegatingTerm ? TermUno.substr(ComplementationPrefix.size(), TermUno.size() - 2) == TermDos : TermDos.substr(ComplementationPrefix.size(), TermDos.size() - 2) == TermUno;
        }
        static bool HasDistributivityExpand(const string& Equation) {
            for (size_t Index = 0; Index < Equation.size(); Index++) {
                if (Equation[Index] != '(') continue;
                
                const size_t Close = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                if (Close == string::npos) continue;
                
                string Inside = Equation.substr(Index + 1, Close - Index - 1);
                string Left, Right, Operator, InnerLeft, InnerRight, InnerOperator;
                if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, Left, Right, Operator)) continue;
                
                if (Distributes(Right, Left, Operator, InnerLeft, InnerRight, InnerOperator)) return true;
                if (Distributes(Left, Right, Operator, InnerLeft, InnerRight, InnerOperator)) return true;
            }
            
            return false;
        }
        static bool HasDistributivityFactor(const string& Equation) {
            for (size_t Index = 0; Index < Equation.size(); Index++) {
                if (Equation[Index] != '(') continue;
                
                const size_t Close = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                if (Close == string::npos) continue;
                
                string Inside = Equation.substr(Index + 1, Close - Index - 1);
                string Left, Right, Operator;
                string LeftLeft, LeftRight, LeftOperator, RightLeft, RightRight, RightOperator;
                
                if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, Left, Right, Operator)) continue;
                if (!Distributes(Right, Left,  Operator, LeftLeft, LeftRight, LeftOperator)) continue;
                if (!Distributes(Left,  Right, Operator, RightLeft, RightRight, RightOperator)) continue;
                if (LeftOperator != RightOperator) continue;
                
                if (LeftLeft == RightLeft || LeftLeft == RightRight || LeftRight == RightLeft || LeftRight == RightRight) return true;
            }
            return false;
        }
        
        inline static unordered_set<string> ExpansionProducts{};
        static void HandleDistributivityExpand(string& Equation, bool DebugDistributivity) {
            string NewEquation;
            size_t Index = 0;
            
            if (DebugDistributivity) std::cout << "Expanding Distributivity:\n";
            while (Index < Equation.size()) {
                if (Equation[Index] != '(') { NewEquation += Equation[Index]; Index++; continue; }
                const size_t ClosingParenthesesIndex = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                
                if (ClosingParenthesesIndex == string::npos) { NewEquation += Equation[Index]; Index++; continue; }
                string Left, Right, Operator, InnerLeft, InnerRight, InnerOperator;
                string Inside = Equation.substr(Index + 1, ClosingParenthesesIndex - Index - 1);
                bool LeftIsGroup = false;
                bool Done = false;
                
                if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                if (Distributes(Right, Left, Operator, InnerLeft, InnerRight, InnerOperator)) { LeftIsGroup = true ; Done = true; }
                else if (Distributes(Left, Right, Operator, InnerLeft, InnerRight, InnerOperator)) { LeftIsGroup = false; Done = true; }
                if (!Done) { NewEquation += Equation[Index]; Index++; continue; }
                if (InnerLeft.empty() || InnerRight.empty() || (InnerOperator != "∧" && InnerOperator != "∨")) { NewEquation += Equation[Index]; Index++; continue; }
                
                const string Group = Equation.substr(Index, ClosingParenthesesIndex - Index + 1);
                if (ExpansionProducts.contains(Group)) { NewEquation += Equation[Index]; Index++; continue; }
                
                const string Replacement = "((" + (LeftIsGroup ? Right : Left) + Operator + InnerLeft + ")" + InnerOperator + "(" + (LeftIsGroup ? Right : Left) + Operator + InnerRight + "))";
                const string TrialEquation = NewEquation + Replacement + Equation.substr(ClosingParenthesesIndex + 1);
                
                const bool Inserted = ExpansionProducts.insert(Replacement).second;
                const size_t TrialCost = Cost(Reduce(TrialEquation, BooleanLaw::DistributivityFactor));
                if (Inserted) ExpansionProducts.erase(Replacement);
                if (TrialCost >= Cost(Equation)) { NewEquation += Equation[Index]; Index++; continue; }
                
                if (DebugDistributivity) {
                    std::cout << "Found Distributivity Expanding: {\n";
                    std::cout << "\tIndex: " << Index << "–" << ClosingParenthesesIndex << "\n";
                    std::cout << "\tVersion: " << (Operator == "∧" ? "AND" : "OR") << "\n";
                    std::cout << "\tEquation: " + Inside + "\n";
                    std::cout << "\tReplacement: " + Replacement + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += Replacement;
                Index = ClosingParenthesesIndex + 1;
            }
            
            Equation = NewEquation;
        }
        static void HandleDistributivityFactor(string& Equation, bool DebugDistributivity) {
            string NewEquation;
            size_t Index = 0;
            
            if (DebugDistributivity) std::cout << "Factoring Distributivity:\n";
            while (Index < Equation.size()) {
                if (Equation[Index] != '(') { NewEquation += Equation[Index]; Index++; continue; }
                const size_t ClosingParenthesesIndex = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                
                if (ClosingParenthesesIndex == string::npos || TestLaw(Equation, BooleanLaw::Distributivity)) { NewEquation += Equation[Index]; Index++; continue; }
                string Left, Right, Operator;
                string LeftInnerLeft,  LeftInnerRight,  LeftInnerOperator;
                string RightInnerLeft, RightInnerRight, RightInnerOperator;
                
                string Inside = Equation.substr(Index + 1, ClosingParenthesesIndex - Index - 1);
                bool LeftIsGroup = false;
                bool Done = false;
                
                if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                if (!Distributes(Right, Left, Operator,  LeftInnerLeft,  LeftInnerRight,  LeftInnerOperator)) { NewEquation += Equation[Index]; Index++; continue; }
                if (!Distributes(Left, Right, Operator, RightInnerLeft, RightInnerRight, RightInnerOperator)) { NewEquation += Equation[Index]; Index++; continue; }
                bool IsLeft  = LeftInnerLeft  == RightInnerLeft || LeftInnerLeft  == RightInnerRight;
                bool IsRight = LeftInnerRight == RightInnerLeft || LeftInnerRight == RightInnerRight;
                string InnerLeft = (IsLeft ? LeftInnerRight : LeftInnerLeft);
                string InnerRight = (IsLeft ? LeftInnerLeft : LeftInnerRight) == RightInnerLeft ? RightInnerRight : RightInnerLeft;
                
                if (!(IsLeft || IsRight) || RightInnerOperator != LeftInnerOperator) { NewEquation += Equation[Index]; Index++; continue; }
                const string Replacement = "("+ (IsLeft ? LeftInnerLeft : LeftInnerRight) + LeftInnerOperator + "(" + InnerLeft + Operator + InnerRight +"))";
                
                if (DebugDistributivity) {
                    std::cout << "Found Distributivity Factoring: {\n";
                    std::cout << "\tIndex: " << Index << "–" << ClosingParenthesesIndex << "\n";
                    std::cout << "\tVersion: " << (Operator == "∧" ? "AND" : "OR") << "\n";
                    std::cout << "\tEquation: " + Inside + "\n";
                    std::cout << "\tReplacement: " + Replacement + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += Replacement;
                Index = ClosingParenthesesIndex + 1;
            }
            
            Equation = NewEquation;
        }
        static void HandleAssociativityEquation(string& Equation, bool DebugAssociativity) {
            // --( Flattening )--
            vector<string> Terms;
            string Operator;
            BooleanAlgebraHelpers::FlattenEquation(Equation, Terms, Operator, DebugAssociativity);
            if (Terms.size() <= 2) return;
            
            if (DebugAssociativity) {
                std::cout << "Found Associativity: {\n";
                std::cout << "\tVersion: " << (Operator == "∧" ? "AND" : "OR") << "\n";
                std::cout << "\tEquation: " + Equation + "\n";
                std::cout << "\tTerms: {" << (Terms.size() > 0 ? "\n" : "");
                for (auto& Term : Terms) std::cout << "\t\t" + Term + "\n";
                std::cout << "\t}\n";
                std::cout << "}\n";
            }
            
            // --( Simplifying )--
            bool AnySimplified = false;
            
            while (true) {
                vector<string> NewTerms;
                vector<bool> Consumed(Terms.size(), false);
                bool PassSimplified = false;
                
                for (size_t i = 0; i < Terms.size(); i++) {
                    if (Consumed[i]) continue;
                    bool Simplified = false;
                    
                    for (size_t j = i + 1; j < Terms.size(); j++) {
                        if (Consumed[j]) continue;
                        
                        const string TempEquation = "(" + Terms[i] + Operator + Terms[j] + ")";
                        const string NewEquation = Reduce(TempEquation, BooleanLaw::Associativity);
                        
                        if (NewEquation == TempEquation) continue;
                        if (Cost(NewEquation) >= Cost(TempEquation)) continue;
                        
                        if (DebugAssociativity) {
                            std::cout << "Simplified: {\n";
                            std::cout << "\tRemoved Terms: " + Terms[i] + ", " + Terms[j];
                            std::cout << "\n\tNew Term/Equation: " + TempEquation + " -> " + NewEquation + "\n}\n";
                        }
                        
                        Consumed[i] = Consumed[j] = true;
                        NewTerms.push_back(NewEquation);
                        Simplified = PassSimplified = true;
                        break;
                    }
                    
                    if (!Simplified && DebugAssociativity) std::cout << "Not Simplified: " + Terms[i] + "\n";
                    if (!Simplified) NewTerms.push_back(Terms[i]);
                }
                
                if (!PassSimplified) break;
                Terms = NewTerms;
                AnySimplified = true;
            }
            
            if (DebugAssociativity) {
                std::cout << "Final Stats:\n";
                std::cout << "\tEquation: " + Equation + "\n";
                std::cout << "}\n";
                std::cout << "Terms: {" << (Terms.size() > 0 ? "\n" : "");
                for (auto& Term : Terms) std::cout << "\t" + Term + "\n";
                std::cout << "}\n";
            }
            
            // --( Reconstructing )--
            if (!AnySimplified) return;
            if (Terms.size() == 1) { Equation = Terms[0]; return; }
            
            Equation = "(" + Terms[0] + Operator + Terms[1] + ")";
            for (size_t i = 2; i < Terms.size(); i++) Equation = "(" + Equation + Operator + Terms[i] + ")";
        }
        
        // --[ Rule Functions ]--
        static void HandleDoubleNegation(string& Equation, bool DebugDoubleNegation) {
            string NewEquation = "";
            vector<bool> KeepParenthesesStack{};
            size_t Index = 0;
            bool FoundDoubleNegation = false;
            bool Keep = true;
            char Character = '\0';
            
            // --[ Process Double Negations ]--
            if (DebugDoubleNegation) {
                std::cout << "\nDouble Negation Stats:\n";
                std::cout << "Processing Ouput:\n";
            }
            
            while (Index < Equation.size()) {
                Character = Equation[Index];
                
                if (Character == ')') {
                    if (DebugDoubleNegation) std::cout << "Found Close Parentheses\n";
                    Keep = KeepParenthesesStack.back();
                    KeepParenthesesStack.pop_back();
                }
                
                if (Equation.compare(Index, DoubleNegationPrefix.size(), DoubleNegationPrefix) == 0) {
                    if (DebugDoubleNegation) std::cout << "Found double Negation\n";
                    Index += DoubleNegationPrefix.size();
                    FoundDoubleNegation = true;
                    KeepParenthesesStack.push_back(false);
                    KeepParenthesesStack.push_back(false);
                    continue;
                } else if (Character == ')' && !Keep) {
                    if (DebugDoubleNegation) std::cout << "Removed Close Parentheses\n";
                    Index++;
                    continue;
                } else if (Character == '(') {
                    if (DebugDoubleNegation) std::cout << "Found Open Parentheses\n";
                    KeepParenthesesStack.push_back(true);
                }
                
                NewEquation += Character;
                Index++;
            }
            
            if (DebugDoubleNegation) {
                std::cout << "\nFound Double Negation: " << (FoundDoubleNegation ? "True\n" : "False\n");
                std::cout << "Old Equation: " << Equation << "\n";
                std::cout << "New Equation: " << NewEquation << "\n\n";
            }
            
            // --[ Apply and Log Results ]--
            if (FoundDoubleNegation) Equation = NewEquation;
        }
        static void HandleDeMorgan(string& Equation, bool DebugDeMorgan) {
            std::smatch RegexMatches;
            
            if (DebugDeMorgan) {
                std::cout << "\nDeMorgan Stats:\n";
                std::cout << "Equation: " << Equation << "\n";
                std::cout << "Factor?: " << (Equation.find(DeMorganPrefixFactor) != string::npos ? "True\n" : "False\n");
                std::cout << "Expand?: " << (Equation.find(DeMorganPrefixExpand) != string::npos ? "True\n" : "False\n");
                std::cout << "Skipping Factor?: " << (Skipping.contains(BooleanLaw::DeMorganFactor) ? "True\n" : "False\n");
                std::cout << "Skipping Expand?: " << (Skipping.contains(BooleanLaw::DeMorganExpand) ? "True\n" : "False\n");
            }
            
            // ((¬(A∧(B∧C)))∧C) -> (((¬A)∧(¬(B∧C))))∧C) -> (((¬A)∧(¬B∧¬C)))∧C)
            if (Equation.find(DeMorganPrefixFactor) != string::npos && !Skipping.contains(BooleanLaw::DeMorganFactor)) HandleDeMorganFactor(Equation, DebugDeMorgan);
            if (Equation.find(DeMorganPrefixExpand) != string::npos && !Skipping.contains(BooleanLaw::DeMorganExpand)) HandleDeMorganExpand(Equation, DebugDeMorgan);
            
            if (DebugDeMorgan) std::cout << "New Equation: " << Equation << "\n\n";
        };
        static void HandleComplementation(string& Equation, bool DebugComplementation) {
            if (DebugComplementation) {
                std::cout << "\nComplementation Stats: \n";
                std::cout << "Equation: " << Equation << "\n";
            }
            
            string NewEquation, Left, Right, Operator;
            size_t Index = 0;
            bool FoundComplementation = false;
            
            while (Index < Equation.size()) {
                if (Equation[Index] != '(') { NewEquation += Equation[Index]; Index++; continue; }
                size_t ClosedParenthesesIndex = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                size_t StartInner = Index + 1;
                string Inner = Equation.substr(StartInner, ClosedParenthesesIndex - StartInner);
                
                if (ClosedParenthesesIndex == string::npos || !BooleanAlgebraHelpers::SplitTopLevel(Inner, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                bool AndComplementation = Operator == "∧" && (Left == Right);
                bool OrComplementation  = Operator == "∨" && (Left == Right);
                bool NegatingTerm;
                
                if (!(AndComplementation || OrComplementation) || !Complements(Left, Right, NegatingTerm)) { NewEquation += Equation[Index]; Index++; continue; }
                if (DebugComplementation) {
                    std::cout << "Found Complementation: {\n";
                    std::cout << "\tIndex: " << Index << "–" << ClosedParenthesesIndex << "\n";
                    std::cout << "\tVersion: " << (AndComplementation ? "AND" : "OR") << "\n";
                    std::cout << "\tNegating Term: " << (NegatingTerm ? "Left" : "Right") << "\n";
                    std::cout << "\tEquation: " + Inner + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += AndComplementation ? '0' : '1';
                Index = ClosedParenthesesIndex + 1;
                FoundComplementation = true;
            }
            
            if (DebugComplementation) {
                std::cout << "Found Complementation?: " <<(FoundComplementation ? "True" : "False") << "\n";
                std::cout << "New Equation: " << NewEquation << "\n\n";
            }
            
            Equation = NewEquation;
        }
        static void HandleAnnihilation(string& Equation, bool DebugAnnihilation) {
            string NewEquation, Left, Right, Operator;
            size_t Index = 0;
            bool FoundAnnihilation = false;
            
            if (DebugAnnihilation) {
                std::cout << "\nAnnihilation Stats: \n";
                std::cout << "Equation: " << Equation << "\n";
                std::cout << "Processing Annihilation:\n";
            }
            
            while (Index < Equation.size()) {
                if (Equation[Index] != '(') { NewEquation += Equation[Index]; Index++; continue; }
                size_t ClosedParenthesesIndex = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                size_t StartInner = Index + 1;
                string Inner = Equation.substr(StartInner, ClosedParenthesesIndex - StartInner);
                
                if (ClosedParenthesesIndex == string::npos || !BooleanAlgebraHelpers::SplitTopLevel(Inner, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                bool AndAnnihlation = Operator == "∧" && (Left == "0" || Right == "0");
                bool OrAnnihlation = Operator == "∨" && (Left == "1" || Right == "1");
                
                if (!(AndAnnihlation || OrAnnihlation)) { NewEquation += Equation[Index]; Index++; continue; }
                if (DebugAnnihilation) {
                    std::cout << "Found Annihilation: {\n";
                    std::cout << "\tIndex: " << Index << "–" << ClosedParenthesesIndex << "\n";
                    std::cout << "\tVersion: " << (AndAnnihlation ? "AND" : "OR") << "\n";
                    std::cout << "\tEquation: " + Inner + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += AndAnnihlation ? '0' : '1';
                Index = ClosedParenthesesIndex + 1;
                FoundAnnihilation = true;
            }
            
            if (DebugAnnihilation) {
                std::cout << "Found Annihilation?: " <<(FoundAnnihilation ? "True" : "False") << "\n";
                std::cout << "New Equation: " << NewEquation << "\n\n";
            }
            
            Equation = NewEquation;
        }
        static void HandleIdentity(string& Equation, bool DebugIdentity) {
            string NewEquation, Left, Right, Operator;
            bool FoundIdentity = false;
            size_t Index = 0;
            
            
            if (DebugIdentity) {
                std::cout << "\nIdentity Stats: \n";
                std::cout << "Equation: " << Equation << "\n";
                std::cout << "Processing Identity:\n";
            }
            
            while (Index < Equation.size()) {
                if (Equation[Index] != '(') { NewEquation += Equation[Index]; Index++; continue; }
                size_t ClosedParenthesesIndex = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                size_t StartInner = Index + 1;
                string Inner = Equation.substr(StartInner, ClosedParenthesesIndex - StartInner);
                
                if (ClosedParenthesesIndex == string::npos || !BooleanAlgebraHelpers::SplitTopLevel(Inner, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                bool AndIdentity = Operator == "∧" && (Left == "1" || Right == "1");
                bool OrIdentity = Operator == "∨" && (Left == "0" || Right == "0");
                
                if (!(AndIdentity || OrIdentity)) { NewEquation += Equation[Index]; Index++; continue; }
                if (DebugIdentity) {
                    std::cout << "Found Identity: {\n";
                    std::cout << "\tIndex: " << Index << "–" << ClosedParenthesesIndex << "\n";
                    std::cout << "\tVersion: " << (AndIdentity ? "AND" : "OR") << "\n";
                    std::cout << "\tEquation: " + Inner + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += ((AndIdentity ? Left == "1" : Left == "0") ? Right : Left);
                Index = ClosedParenthesesIndex + 1;
                FoundIdentity = true;
            }
            
            if (DebugIdentity) {
                std::cout << "Found Identity?: " <<(FoundIdentity ? "True" : "False") << "\n";
                std::cout << "New Equation: " << NewEquation << "\n\n";
            }
            
            Equation = NewEquation;
        }
        static void HandleIdempotence(string& Equation, bool DebugIdempotence) {
            if (DebugIdempotence) {
                std::cout << "\nIdempotence Stats: \n";
                std::cout << "Equation: " << Equation << "\n";
            }
            
            string NewEquation, Left, Right, Operator;
            size_t Index = 0;
            bool FoundIdempotence = false;
            
            while (Index < Equation.size()) {
                if (Equation[Index] != '(') { NewEquation += Equation[Index]; Index++; continue; }
                size_t ClosedParenthesesIndex = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                size_t StartInner = Index + 1;
                string Inner = Equation.substr(StartInner, ClosedParenthesesIndex - StartInner);
                
                if (ClosedParenthesesIndex == string::npos || !BooleanAlgebraHelpers::SplitTopLevel(Inner, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                bool AndIdempotence = Operator == "∧" && (Left == Right);
                bool OrIdempotence  = Operator == "∨" && (Left == Right);
                bool TermsEqual = Left == Right;
                
                if (!(AndIdempotence || OrIdempotence) || !TermsEqual) { NewEquation += Equation[Index]; Index++; continue; }
                if (DebugIdempotence) {
                    std::cout << "Found Idempotence: {\n";
                    std::cout << "\tIndex: " << Index << "–" << ClosedParenthesesIndex << "\n";
                    std::cout << "\tVersion: " << (AndIdempotence ? "AND" : "OR") << "\n";
                    std::cout << "\tTerm: " << Left << "\n";
                    std::cout << "\tEquation: " + Inner + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += Left;
                Index = ClosedParenthesesIndex + 1;
                FoundIdempotence = true;
            }
            
            if (DebugIdempotence) {
                std::cout << "Found Idempotence?: " <<(FoundIdempotence ? "True" : "False") << "\n";
                std::cout << "New Equation: " << NewEquation << "\n\n";
            }
            
            Equation = NewEquation;
        }
        static void HandleAbsorption(string& Equation, bool DebugAbsorption) {
            // A ∧ (A ∨ B) -> A or A ∨ (A ∧ B) -> A
            string NewEquation, Left, Right, Operator;
            bool FoundAbsorption = false;
            size_t Index = 0;
            
            if (DebugAbsorption) {
                std::cout << "\nAbsorption Stats: \n";
                std::cout << "Equation: " << Equation << "\n";
                std::cout << "Processing Absorption:\n";
            }
            
            while (Index < Equation.size()) {
                if (Equation[Index] != '(') { NewEquation += Equation[Index]; Index++; continue; }
                
                const size_t Close = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                if (Close == string::npos) { NewEquation += Equation[Index]; Index++; continue; }
                
                string Inner = Equation.substr(Index + 1, Close - Index - 1);
                if (!BooleanAlgebraHelpers::SplitTopLevel(Inner, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                
                string Survivor;
                if      (Absorbs(Right, Left, Operator)) Survivor = Right;
                else if (Absorbs(Left, Right, Operator)) Survivor = Left;
                else { NewEquation += Equation[Index]; Index++; continue; }
                
                if (DebugAbsorption) {
                    std::cout << "Found Absorption: {\n";
                    std::cout << "\tIndex: " << Index << "–" << Close << "\n";
                    std::cout << "\tVersion: " << (Operator == "∧" ? "AND" : "OR") << "\n";
                    std::cout << "\tEquation: " + Inner + "\n";
                    std::cout << "}\n";
                }
                
                NewEquation += Survivor;
                Index = Close + 1;
                FoundAbsorption = true;
            }
            
            if (DebugAbsorption) {
                std::cout << "Found Absorption?: " << (FoundAbsorption ? "True" : "False") << "\n";
                std::cout << "New Equation: " << NewEquation << "\n\n";
            }
            
            Equation = NewEquation;
        }
        static void HandleDistributivity(string& Equation, bool DebugDistributivity) {
            std::smatch RegexMatches;
            
            if (DebugDistributivity) {
                std::cout << "\nDistributivity Stats:\n";
                std::cout << "Equation: " << Equation << "\n";
            }
            
            if (HasDistributivityFactor(Equation) && !Skipping.contains(BooleanLaw::DistributivityFactor)) HandleDistributivityFactor(Equation, DebugDistributivity);
            if (HasDistributivityExpand(Equation) && !Skipping.contains(BooleanLaw::DistributivityExpand)) HandleDistributivityExpand(Equation, DebugDistributivity);
            
            if (DebugDistributivity) std::cout << "New Equation: " << Equation << "\n\n";
        };
        static void HandleAssociativity(string& Equation, bool DebugAssociativity) {
            if (Skipping.contains(BooleanLaw::Associativity)) return;
            string NewEquation, Left, Right, Operator;
            bool FoundAssociativity = false;
            size_t Index = 0;
            
            if (DebugAssociativity) {
                std::cout << "\nAssociativity Stats: \n";
                std::cout << "Equation: " << Equation << "\n";
                std::cout << "Processing Absorption:\n";
            }
            
            while (Index < Equation.size()) {
                if (Equation[Index] != '(') { NewEquation += Equation[Index]; Index++; continue; }
                
                const size_t Close = BooleanAlgebraHelpers::FindClosed(Equation, Index);
                if (Close == string::npos) { NewEquation += Equation[Index]; Index++; continue; }
                
                const string Group = Equation.substr(Index, Close - Index + 1);
                string Processed = Group;
                HandleAssociativityEquation(Processed, DebugAssociativity);
                
                if (Processed == Group) { NewEquation += Equation[Index]; Index++; continue; }
                
                FoundAssociativity = true;
                NewEquation += Processed;
                Index = Close + 1;
            }
            
            if (DebugAssociativity) {
                std::cout << "Found Associativity?: " << (FoundAssociativity ? "True" : "False") << "\n";
                std::cout << "New Equation: " << NewEquation << "\n\n";
            }
            
            Equation = NewEquation;
        }
        static void HandleParentheses(string& Equation) {
            Equation = std::regex_replace(Equation, ParenthesesRegex1, "$1");
        }
};