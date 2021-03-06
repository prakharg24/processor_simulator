Assumptions:
------------
1. load word and store word comes in multiples of 4
2. the byte 0th is the most significant digits
3. Exception raised when values cross 32 bits
4. all instructions in part 1 that are in part2 are valid
5. Assumed that threads are all active simulataneously and hence any non-op instruction corresponds to active thread.
6. Assume that hex follows memdump and an integer follows it.
7. Any errors in overflow of register values cause program to stop.
8. Any error in memory out of bounds are handled by exit(0). 

Run instructions:
-----------------
$ make
$ processor_simulator <input_HEX_file> <output_SVG_file> --for part1
$ processor_simulator <input_HEX_file> <output_SVG_file> <output_results_file>  --for part2

open <output_SVG_file>.html in mozilla browser for good results.
do not tamper with .css and .js files created.


Clock timing:-
--------------
1. One stall cycle for any instruction following load instruction with data dependancy. 
2. Branch instruction is detected in ALU stage and corresponding insructions before that are flushed.
3. All ohter dependacies are handled by data forwarding.
4. 3 paths are chosen for data forwarding P1 for ALU to ALU, P2 for Wb to AL, P3 for WB to Wb.


----------------------------------------------------------------------------------------------------------------------
Copyrights : Dingi & Perbro Stud Coders Co. (R)(TM)
   (Saket Dingliwal & Prakhar Ganesh)
----------------------------------------------------------------------------------------------------------------------

