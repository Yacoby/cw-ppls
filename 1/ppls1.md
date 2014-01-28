### Q2

    int x = 10, y = 0;
    co
        while (x!=y) { ##A
            x = x-1; ##B
        }
        y = y+1; ##C
        //
        <await (x==y);> ##D
        x = 8; ##E
        y = 2; ##F
    oc

So there are the following paths through the program:

    E, F, (A, B)*, C - (NOT POSSIBLE)
    E, (A, B)*, C, F - (NOT POSSIBLE)
    E, (A, B)*, F, C - (NOT POSSIBLE)
    (A, B)*, E, F, C - x = 8, y = 3
    (A, B)*, E, C, F - x = 8, y = 2
    (A, B)*, C, E, F - x = 8, y = 2

    E, (A, B)*, F, (A, B)*, C - (NOT POSSIBLE)
    (A, B)*, E, (A, B)*, F, C - x = 0, y = 3
    (A, B)*, E, (A, B)*, C, F - x = 0, y = 2
    (A, B)*, E, (A, B)*, F, (A, B)*, C - (FAIL TO TERMINATE, y = 2, x -> -INF)

    E, (A, B)* A, F, B, (A, B)*, C - (NOT POSSIBLE)
    (A, B)*, A, E, B, (A, B)*, F, C - No difference from previous
    (A, B)*, A, E, B, (A, B)*, C, F - No difference from previous

    (A, B)*, A, E, B, (A, B)*, F, (A, B)*, C
    (A, B)*, A, E, B, (A, B)*, A, F, B, (A, B)*, C
    (A, B)*, E, (A, B)*, A, F, B, (A, B)*, C - Can fail to terminate, can lead to x = 2, y = 3


NB: Think I have missed something. There should be x = 2, y = 2 (+-1). EDIT: Nope, don't think so?

Treating C as C' and C'' (as there is both a read and an assignment and the operation isn't atomic gives an additional path that results in a distinct result)

    (A, B)*, E, C', F, C'' - x = 8, y = 1
    (A, B)*, B, E, (A,B)*, C', F, C'' - x = 0, y = 1

Consider B' and B'' there should be no difference

Consider A' and A''. Don't think it has an effect but need to ensure that this is case case.



So finally (not considering A', B'') there are the following results

    x = 8, y = 3
    x = 8, y = 2
    x = 0, y = 3
    x = 0, y = 2
    x -> -INF , y = 2 (Fail to terminate)

    x = 2, y = 3
    x = 8, y = 1
    x = 0, y = 1



### Q2

Write a short report (of around a page) on such a shared variable version of the
algorithm, discussing its relationship to any patterns, synchronisation requirements,
and the issues which would arise if it were to be amended to allow for more nodes in
the graph than processors. NB You are not being asked to write pseudo-code for the
algorithm, and no credit will be given for doing so.


Notes:
 * Rounds are simalar to the Interacting Peers Pattern
    * Same Program Multiple Data
 * Could be implemented using bag of tasks, but doesn't fit that well
    * Does have the advantage of allowing higher performance processors
 * The messages sent are deg(v) and rndvalue(v) and the first legal colour
 * In a multiple machine env these cannot be read and it is clear when a round starts and ends
 * In a shared memory environment these can be read at any time
 * Syncronisation is needed to allow rounds to be formatted

 * Dealing with more tasks per node just means that a single processor has to preform the work of multiple nodes per round
    * Either threads to allow each task to be run in parallel
    * Sequentially
    * Equlivent as long as long as the entire thing remains in lockstep
 
 * NOt the best of ideas, maybe use a better algo? Look into this


