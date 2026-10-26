#pragma once
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <unordered_set>

using std::string, std::array, std::vector, std::unordered_set;

class LogicGate {
    public:
        const string ClassName;
        bool CurrentState = false;
        bool NextState = false;
        
        virtual void ComputeNextState() = 0;
        virtual void CommitState() { CurrentState = NextState; }
        
        // Non-recursive getter for current value
        virtual bool Evaluate() { return CurrentState; }
        
        // Cyclic-safe equation generator using a visited stack
        virtual string GetEquation(unordered_set<LogicGate*>& VisitedGates) = 0;
        string GetEquation() {
            unordered_set<LogicGate*> VisitedGates;
            return GetEquation(VisitedGates);
        }
        
        explicit LogicGate(string ClassName) : ClassName(std::move(ClassName)) {}
        virtual ~LogicGate() = default;
};

template <int InputCountTemplate>
class BaseGate : public LogicGate {
    public:
        explicit BaseGate(string ClassName, array<LogicGate*, InputCountTemplate> Inputs) : LogicGate(ClassName), Inputs(Inputs) {}
        explicit BaseGate(string ClassName) : LogicGate(ClassName) {}
        
        static constexpr int InputCount = InputCountTemplate;
        array<LogicGate*, InputCount> Inputs{};
};

class Input : public BaseGate<0> {
    public:
        Input(string Name) : BaseGate("INPUT"), Name(Name) {}
        Input() : BaseGate("INPUT") {}
        
        string Name;
        
        void ComputeNextState() override {
            NextState = CurrentState; // Input state is set externally
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            return Name.empty() ? (CurrentState ? "1" : "0") : Name;
        }
};

class Output : public BaseGate<1> {
    public:
        Output(string Name, LogicGate* InputGate) : BaseGate("OUTPUT", {InputGate}), Name(Name) {}
        Output(string Name) : BaseGate("OUTPUT"), Name(Name) {}
        Output() : BaseGate("OUTPUT") {}
        
        string Name;
        
        void ComputeNextState() override {
            NextState = Inputs[0] ? Inputs[0]->CurrentState : false;
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if (!Inputs[0]) return "0";
            return Inputs[0]->GetEquation(VisitedGates);
        }
        
        void OutputData() { std::cout << CurrentState; }
        string ToStr() { return CurrentState ? "1" : "0"; }
};

class Data : public BaseGate<0> {
    public:
        Data() : BaseGate("Data") {}
        
        void ComputeNextState() override {
            NextState = CurrentState;
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            return CurrentState ? "1" : "0";
        }
};

class ANDGate : public BaseGate<2> {
    public:
        ANDGate(array<LogicGate*, 2> Inputs) : BaseGate("AND", Inputs) {}
        ANDGate() : BaseGate("AND") {}
        
        void ComputeNextState() override {
            NextState = Inputs[0]->CurrentState && Inputs[1]->CurrentState;
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if (VisitedGates.count(this)) return ClassName + "(t-1)";
            VisitedGates.insert(this);
            string Result = "(" + Inputs[0]->GetEquation(VisitedGates) + "∧" + Inputs[1]->GetEquation(VisitedGates) + ")";
            VisitedGates.erase(this);
            return Result;
        }
};

class ORGate : public BaseGate<2> {
    public:
        ORGate(array<LogicGate*, 2> Inputs) : BaseGate("OR", Inputs) {}
        ORGate() : BaseGate("OR") {}
        
        void ComputeNextState() override {
            NextState = Inputs[0]->CurrentState || Inputs[1]->CurrentState;
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if (VisitedGates.count(this)) return ClassName + "(t-1)";
            VisitedGates.insert(this);
            string Result = "(" + Inputs[0]->GetEquation(VisitedGates) + "∨" + Inputs[1]->GetEquation(VisitedGates) + ")";
            VisitedGates.erase(this);
            return Result;
        }
};

class NOTGate : public BaseGate<1> {
    public:
        NOTGate(LogicGate* InputGate) : BaseGate("NOT", {InputGate}) {}
        NOTGate() : BaseGate("NOT") {}
        
        void ComputeNextState() override {
            NextState = !Inputs[0]->CurrentState;
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if (VisitedGates.count(this)) return ClassName + "(t-1)";
            VisitedGates.insert(this);
            string Result = "(¬" + Inputs[0]->GetEquation(VisitedGates) + ")";
            VisitedGates.erase(this);
            return Result;
        }
};

class NORGate : public BaseGate<2> {
    public:
        NORGate(array<LogicGate*, 2> Inputs) : BaseGate("NOR", Inputs) {}
        NORGate() : BaseGate("NOR") {}
        
        void ComputeNextState() override {
            NextState = !(Inputs[0]->CurrentState || Inputs[1]->CurrentState);
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if (VisitedGates.count(this)) return ClassName + "(t-1)";
            VisitedGates.insert(this);
            string Result = "(" + Inputs[0]->GetEquation(VisitedGates) + "↓" + Inputs[1]->GetEquation(VisitedGates) + ")";
            VisitedGates.erase(this);
            return Result;
        }
};

class XORGate : public BaseGate<2> {
    public:
        XORGate(array<LogicGate*, 2> Inputs) : BaseGate("XOR", Inputs) {}
        XORGate() : BaseGate("XOR") {}
        
        void ComputeNextState() override {
            NextState = Inputs[0]->CurrentState != Inputs[1]->CurrentState;
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if (VisitedGates.count(this)) return ClassName + "(t-1)";
            VisitedGates.insert(this);
            string Result = "(" + Inputs[0]->GetEquation(VisitedGates) + "⊕" + Inputs[1]->GetEquation(VisitedGates) + ")";
            VisitedGates.erase(this);
            return Result;
        }
};

class XNORGate : public BaseGate<2> {
    public:
        XNORGate(array<LogicGate*, 2> Inputs) : BaseGate("XNOR", Inputs) {}
        XNORGate() : BaseGate("XNOR") {}
        
        void ComputeNextState() override {
            NextState = Inputs[0]->CurrentState == Inputs[1]->CurrentState;
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if (VisitedGates.count(this)) return ClassName + "(t-1)";
            VisitedGates.insert(this);
            string Result = "(" + Inputs[0]->GetEquation(VisitedGates) + "⊙" + Inputs[1]->GetEquation(VisitedGates) + ")";
            VisitedGates.erase(this);
            return Result;
        }
};

class NANDGate : public BaseGate<2> {
    public:
        NANDGate(array<LogicGate*, 2> Inputs) : BaseGate("NAND", Inputs) {}
        NANDGate() : BaseGate("NAND") {}
        
        void ComputeNextState() override {
            NextState = !(Inputs[0]->CurrentState && Inputs[1]->CurrentState);
        }
        
        string GetEquation(unordered_set<LogicGate*>& VisitedGates) override {
            if (VisitedGates.count(this)) return ClassName + "(t-1)";
            VisitedGates.insert(this);
            string Result = "(" + Inputs[0]->GetEquation(VisitedGates) + "↑" + Inputs[1]->GetEquation(VisitedGates) + ")";
            VisitedGates.erase(this);
            return Result;
        }
};