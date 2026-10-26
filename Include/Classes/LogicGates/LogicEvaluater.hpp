#pragma once
#include "Classes/LogicGates/LogicGates.hpp"
#include "Classes/LogicGates/BooleanAlgebraHelpers.hpp"
#include <vector>
#include <regex>
#include <iterator>
#include <algorithm>
#include <windows.h>

using std::string;
using std::vector;
using std::regex;

class LogicEvaluator {
    public:
        inline static bool Debug = false;
        inline static bool DeepDebug = false;
        
        static string SimplifyEquation(LogicGate* Gate) { return SimplifyEquation(Gate->GetEquation()); }
        static string SimplifyEquation(string Equation) { SimplifyEquation(&Equation); return Equation; }
        static void SimplifyEquation(string* Equation) {
            string PreviousEquation;
            
            do {
                PreviousEquation = *Equation;
                SimplifyOnce(*Equation);
            } while (*Equation != PreviousEquation);
            
            // HandleParentheses(*Equation);
        }
        
    private:
        static void SimplifyOnce(string& Equation) {
            
            if (Debug) std::cout << "(Pre-DoubleNegation) Equation: " + Equation + "\n";
            if (Equation.find("(¬(¬") != string::npos) HandleDoubleNegation(Equation, DeepDebug);
            if (Debug) std::cout << "(Post-DoubleNegation) Equation: " + Equation + "\n";
            
            if (Debug) std::cout << "(Pre-DeMorgan) Equation: " + Equation + "\n";
            HandleDeMorgan(Equation, DeepDebug);
            if (Debug) std::cout << "(Post-DeMorgan) Equation: " + Equation + "\n";
            
            if (Debug) std::cout << "(Pre-Complementation) Equation: " + Equation + "\n";
            if (std::regex_search(Equation, ComplementationRegex1) || std::regex_search(Equation, ComplementationRegex2)) HandleComplementation(Equation, DeepDebug);
            if (Debug) std::cout << "(Post-Complementation) Equation: " + Equation + "\n";
            
            if (Debug) std::cout << "(Pre-Annihilation) Equation: " + Equation + "\n";
            HandleAnnihilation(Equation, DeepDebug);
            if (Debug) std::cout << "(Post-Annihilation) Equation: " + Equation + "\n";
            
            if (Debug) std::cout << "(Pre-Identity) Equation: " + Equation + "\n";
            HandleIdentity(Equation, DeepDebug);
            if (Debug) std::cout << "(Post-Identity) Equation: " + Equation + "\n";
            
            if (Debug) std::cout << "(Pre-Identity) Equation: " + Equation + "\n";
            HandleIdempotence(Equation, DeepDebug);
            if (Debug) std::cout << "(Post-Identity) Equation: " + Equation + "\n";
            
            if (Debug) std::cout << "(Pre-Absorption) Equation: " + Equation + "\n";
            HandleAbsorption(Equation, DeepDebug);
            if (Debug) std::cout << "(Post-Absorption) Equation: " + Equation + "\n";
            
            if (Debug) std::cout << "(Pre-Distributivity) Equation: " + Equation + "\n";
            HandleDistributivity(Equation, DeepDebug);
            if (Debug) std::cout << "(Post-Distributivity) Equation: " + Equation + "\n";
            
            // if (Debug) std::cout << "(Pre-Associativity) Equation: " + Equation + "\n";
            // HandleAssociativity(Equation, DeepDebug);
            // if (Debug) std::cout << "(Post-Associativity) Equation: " + Equation + "\n";
            
            // if (Debug) std::cout << "(Pre-Commutativity) Equation: " + Equation + "\n";
            // HandleCommunitvity(Equation, DeepDebug);
            // if (Debug) std::cout << "(Post-Commutativity) Equation: " + Equation + "\n";
            
            if (Debug) std::cout << "(Pre-Parentheses) Equation: " + Equation + "\n";
            HandleParentheses(Equation);
            if (Debug) std::cout << "(Post-Parentheses) Equation: " + Equation + "\n";
        }
        
        // --[ Regex ]--
        
        // --( Double Negation )--
        inline static const string DoubleNegationPrefix = "(¬(¬";
        
        // --( DeMorgan )--
        inline static const string DeMorganPrefixFactor = "((¬";
        inline static const string DeMorganPrefixExpand = "(¬(";
        inline static const regex DeMorganRegex{ R"(^\(¬(.+)\)$)" };
        
        // --( Complementation Regex )--
        inline static const regex ComplementationRegex1{ R"((\((.+)∨\(¬\2\)\))|(\(\(¬(.+)\)∨\4\)))" };
        inline static const regex ComplementationRegex2{ R"((\((.+)∧\(¬\2\)\))|(\(\(¬(.+)\)∧\4\)))" };
        
        // --( Idempotence Regex )--
        inline static const regex IdempotenceRegex{ R"(\((.+)(?:∨|∧)\1\))" };
        
        // --( Parentheses Regex )--
        inline static const regex ParenthesesRegex1{ R"(\(([^¬∨∧\n]+?)\))" };
        
        // --[ Helper Functions ]--
        inline static vector<BooleanLaw> Skipping{};
        
        static bool TestLaw(string Equation, BooleanLaw SkipLaw) {
            string PreviousEquation = Equation;
            
            Skipping.push_back(SkipLaw);
            if (DeepDebug) {
                std::cout << "Testing Laws:\n";
                std::cout << "Skipping: {\n";
                for (BooleanLaw Law : Skipping) { std::cout << "\t" << BooleanAlgebraHelpers::LawToString(Law) << "\n"; }
                std::cout << "}\n\n";
            }
            
            if (!std::ranges::contains(Skipping, BooleanLaw::DoubleNegation )) HandleDoubleNegation(Equation, false);
            if (!std::ranges::contains(Skipping, BooleanLaw::DeMorgan       )) HandleDeMorgan(Equation, false);
            if (!std::ranges::contains(Skipping, BooleanLaw::Complementation)) HandleComplementation(Equation, false);
            if (!std::ranges::contains(Skipping, BooleanLaw::Annihilation   )) HandleAnnihilation(Equation, false);
            if (!std::ranges::contains(Skipping, BooleanLaw::Identity       )) HandleIdentity(Equation, false);
            if (!std::ranges::contains(Skipping, BooleanLaw::Idempotence    )) HandleIdempotence(Equation, false);
            if (!std::ranges::contains(Skipping, BooleanLaw::Absorption     )) HandleAbsorption(Equation, false);
            if (!std::ranges::contains(Skipping, BooleanLaw::Distributivity )) HandleDistributivity(Equation, false);
            // if (std::ranges::contains(Skipping, BooleanLaw::Associativity  )) HandleAssociativity(Equation, false);
            // if (std::ranges::contains(Skipping, BooleanLaw::Commutativity  )) HandleCommutativity(Equation, false);
            
            Skipping.pop_back();
            return Equation != PreviousEquation;
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
                
                if (!TestLaw(TrialEquation, BooleanLaw::DeMorgan)) { NewEquation += Equation[Index]; Index++; continue; }
                
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
                
                if (ClosingParenthesesIndex == string::npos || TestLaw(Equation, BooleanLaw::DeMorgan)) { NewEquation += Equation[Index]; Index++; continue; }
                string Left, Right, Operator;
                string Inside = Equation.substr(StartInside, ClosingParenthesesIndex - StartInside);
                
                if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, Left, Right, Operator)) { NewEquation += Equation[Index]; Index++; continue; }
                if (std::regex_match(Left , DeMorganRegex)) Left  = std::regex_replace(Left , DeMorganRegex, "$1"); else { NewEquation += Equation[Index]; Index++; continue; }
                if (std::regex_match(Right, DeMorganRegex)) Right = std::regex_replace(Right, DeMorganRegex, "$1"); else { NewEquation += Equation[Index]; Index++; continue; }
                
                const string FlippedOperator = (Operator == "∨" ? "∧" : "∨");
                const string Replacement = "(¬(" + Left + FlippedOperator + Right + "))";
                
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
            
            string Inside = Group.substr(1, Group.size() - 2);
            if (!BooleanAlgebraHelpers::SplitTopLevel(Inside, InnerLeft, InnerRight, InnerOperator)) return false;
            
            return InnerOperator != OuterOperator;
        }
        
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
                if (Distributes(Left, Right, Operator, InnerLeft, InnerRight, InnerOperator)) { LeftIsGroup = false; Done = true; }
                if (!Done) { NewEquation += Equation[Index]; Index++; continue; }
                
                const string Replacement = "((" + (LeftIsGroup ? Right : Left) + Operator + InnerLeft + ")" + InnerOperator + "(" + (LeftIsGroup ? Right : Left) + Operator + InnerRight + "))";
                const string TrialEquation = NewEquation + Replacement + Equation.substr(ClosingParenthesesIndex + 1);
                
                if (!TestLaw(TrialEquation, BooleanLaw::Distributivity)) { NewEquation += Equation[Index]; Index++; continue; }
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
            }
            
            HandleDeMorganFactor(Equation, DebugDeMorgan);
            HandleDeMorganExpand(Equation, DebugDeMorgan);
            
            if (DebugDeMorgan) std::cout << "New Equation: " << Equation << "\n\n";
        };
        static void HandleComplementation(string& Equation, bool DebugComplementation) {
            if (DebugComplementation) {
                std::cout << "\nComplementation Stats: \n";
                
                size_t Regex1MatchCount = std::distance(std::sregex_iterator(Equation.begin(), Equation.end(), ComplementationRegex1), std::sregex_iterator());
                size_t Regex2MatchCount = std::distance(std::sregex_iterator(Equation.begin(), Equation.end(), ComplementationRegex2), std::sregex_iterator());
                size_t TotalMatchCount  = Regex1MatchCount + Regex2MatchCount;
                
                std::cout << "Regex 1 Match Count: " << Regex1MatchCount << "\n";
                std::cout << "Regex 2 Match Count: " << Regex2MatchCount << "\n";
                std::cout << "Total Match Count: " << TotalMatchCount << "\n";
                std::cout << "Matched?: " << (TotalMatchCount > 0 ? "True" : "False") << "\n";
                std::cout << "Equation: " << Equation << "\n";
            }
            
            Equation = std::regex_replace(Equation, ComplementationRegex1, "1");
            Equation = std::regex_replace(Equation, ComplementationRegex2, "0");
            
            if (DebugComplementation) std::cout << "New Equation: " << Equation << "\n\n";
        }
        static void HandleAnnihilation(string& Equation, bool DebugAnnihilation) {
            string NewEquation, Left, Right, Operator;
            size_t Index = 0;
            bool FoundAnnihlation = false;
            
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
                FoundAnnihlation = true;
            }
            
            if (DebugAnnihilation) {
                std::cout << "Found Annihlation?: " <<(FoundAnnihlation ? "True" : "False") << "\n";
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
                
                NewEquation += ((Left == "1" || Left == "0") ? Right : Left);
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
                std::cout << "Match Count: " << std::distance(std::sregex_iterator(Equation.begin(), Equation.end(), IdempotenceRegex), std::sregex_iterator()) << "\n";
                std::cout << "Matched?: " << (std::regex_search(Equation, IdempotenceRegex) ? "True" : "False") << "\n";
                std::cout << "Equation: " << Equation << "\n";
            }
            
            Equation = std::regex_replace(Equation, IdempotenceRegex, "$1");
            
            if (DebugIdempotence) std::cout << "New Equation: " << Equation << "\n\n";
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
            
            HandleDistributivityFactor(Equation, DebugDistributivity);
            HandleDistributivityExpand(Equation, DebugDistributivity);
            
            if (DebugDistributivity) std::cout << "New Equation: " << Equation << "\n\n";
        };
        
        static void HandleAssociativity(string& Equation, bool DebugAssociativity) {
            
        }
        
        static void HandleCommutativity(string& Equation, bool DebugCommutativity);
        static void HandleParentheses(string& Equation) {
            Equation = std::regex_replace(Equation, ParenthesesRegex1, "$1");
        }
};