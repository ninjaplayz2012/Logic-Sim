
---
# Boolean Algebra
## Logic Simplifier
### List of Boolean Algebra Laws
The following is showing each of the Boolean Algebra laws we used in the program to simplify equations along with the operation it does.  These use the 3 main operations AND, OR, and NOT to simplify, any other operation is converted to it's equivalent form using the AND, OR, and NOT operations before being converted back at the end of the program.

|         Law         | Operator(s) |                  Operation                   |
| :-----------------: | :---------: | :------------------------------------------: |
| AND Distributivity  |   AND, OR   | $$A\land(B\lor C)=(A\land B)\lor(A\land C)$$ |
|  OR Distributivity  |   AND, OR   | $$A\lor(B\land C)=(A\lor B)\land(A\lor C)$$  |
|    AND Identity     |     AND     |                $$A\land1=A$$                 |
|     OR Identity     |     OR      |                 $$A\lor0=A$$                 |
|   AND Annihilator   |     AND     |                $$A\land0=0$$                 |
|   OR Annihilator    |     OR      |                 $$A\lor0=0$$                 |
|   AND Idempotence   |     AND     |                $$A\land A=A$$                |
|   OR Idempotence    |     OR      |                $$A\lor A=A$$                 |
|   AND Absorption    |   AND, OR   |          $$A\land(A\lor B)=A$$<br>           |
|    OR Absorption    |   AND, OR   |            $$A\lor(A\land B)=A$$             |
| AND Complementation |     AND     |              $$A\land\neg A=0$$              |
| OR Complementation  |     OR      |            <br>$$A\lor\neg A=1$$             |
|   Double Negation   |     NOT     |              $$\neg(\neg A)=A$$              |
|      De Morgan      |  NOT + AND  |     $$\neg(A\land B)=\neg A\lor\neg B$$      |
|      De Morgan      |  NOT + OR   |     $$\neg(A\lor B)=\neg A\land\neg B$$      |


## Logic Evaluator
## Logic Gates
### List of Logic Gates

| Logic Gate | Operation | Input Count | Description                     |
| :--------: | :-------: | :---------: | ------------------------------- |
|    AND     |   A ∧ B   |  2 Inputs   | Checks if both inputs are true. |
|     OR     |   A ∨ B   |  2 Inputs   | Checks if any input is true.    |
|    NOT     |    ¬A     |  1 Inputs   | Inverts the input.              |
|    XOR     |   A ⊕ B   |  2 Inputs   | Checks if inputs are not equal. |
|    XNOR    |   A ⊙ B   |  2 Inputs   | Inverse of the XOR gate.        |
|    NAND    |   A ↑ B   |  2 Inputs   | Inverse of the AND gate.        |
|    NOR     |   A ↓ B   |  2 Inputs   | Inverse of the OR gate.         |
