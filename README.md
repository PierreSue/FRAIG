# Functionally Reduced And-Inverter Graph (FRAIG)

<img src="https://github.com/PierreSue/FRAIG/blob/master/Diagram.png">

## Objective

This is the implementation of circuit simplication simplification. By means of unused gate sweeping, trivial optimization, simplification by structural hash, and previous simulation, I try to preliminarily simplify the circuits in an efficient manner. After that, I also apply Equivalence gate merging to the circuits using Boolean Satisfiability (SAT) solver. 

## The analysis of pros and cons of each method
1. Unused Gate Sweeping

Sweeping out the gates that cannot be reached from POs. After this operation, all the gates that are originally “defined-butnot-used” will be deleted. ***Depth First Search Traversal*** is used here to check if the signal will pass to POs through these gates. This method can decrease the overall complexity and the required running time.

2. Trivial optimization

There are four cases here. 

(1) When the both fanins are the same, replace them with only one fanin. <br></br>
(2) When one of the fanins is true while the other is false, replace it with 0. <br></br>
(3) When one of the fanins is true, replace it with the other fanin. <br></br>
(4) When one of the fanins is false, replace it with 0.

The overall required running time is the same as one Depth First Search Traversal.

3. Simplification by structural hash

This section will be divided into two parts for discussion.

First of all, ***HashMap*** is used here. I store information of the both fannin and the ***HashKey*** is represented as the first element of ***HashNode***. Moreover, I overload () as the multiplication of two fanin IDs, having the overlapping possibility low.

Second, I use a large number of ***HashMap*** to search the whole circuits. The advantage here is that it is so efficient to run over the whole circuit and utilize Strash(replace the overlapping parts). It basically takes one or two second to complete.

4. Previous Simulation

Perform circuit simulation to distinguish the functionally different signals and thus collect the FEC pairs/groups. The operations are all related to ***HashMap***. The first element is ***SimValue***, responsible for recording the value after simulation. The second element is ***vector<CirGate* >***, used for storing all the gates with same SimValue. To make the whole process quicker, I use a bool number here to check if the the total number is more than 20000. If it is, I only simulate 5 times of fails because I found that the results of 5 fails and 181 fails are comparable. And if I choose to start FRAIG first, the total number of gates will decline significantly. But the disadvantage here is that when the total number is really big, there will be a large number of repetition. All in all, the pros outweighs the cons after several tests.

5. FRAIG: Equivalence gate merging

Based on the identified FEC pairs/groups, perform fraig operations on the circuit. All the ***SAT-proven equivalent gates*** should be merged together, while it is optional to re-simulate the witness patterns of the non-equivalent FEC pairs. 
