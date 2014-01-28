### Q1

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

 * In the mp impl, messages sent are deg(v) and rndvalue(v) and the first legal colour
 * In the shared memory version this data is readable by everyone

In the message passing implementation of the algorithm each node represents a node in the graph. Each node v holds internal state such as the list of colours it cannot be. At each round of the algorithm if it hasn't yet been allocated a colour it sends deg(v), rndvalue(v) and its choice of colour to all its neighbours.

Unlike the message passing implementation all data is readable by all nodes which means that rather than nodes sending data to receiving nodes, the receiving nodes would just read the data themselves. This leads to the problem that it is possible for the receiving node to read data in an inconsistent state and or if the data is processed at different speeds the algorithm could move out of lockstep and so one node could be several rounds ahead of other nodes.

 * This is most simalar to the Interacting Peers Pattern
    * Same Program Multiple Data
    * Each processor responsible for maintaining data about invlaid colours etc
    * Use barriers to maintain syncronisation. Simalar to Jacboi algo... mentioned in lectures 

    * Dealing with more tasks per node just means that a single processor has to preform the work of multiple nodes per round
       * Sequentially
       * Either threads to allow each task to be run in parallel
       * Equlivent as long as long as the entire thing remains in lockstep

This is most similar to the interacting peers pattern, as each node has some other number of nodes it interacts with. As with other algorithms that use rounds that use this pattern such as the Jacobi algorithm barriers would be used to enforce synchronisation between the rounds and ensure the algorithm runs in lockstep. This barrier would ensure that all processors wait until all active processors have entered the barrier, but given that it is possible for a node to complete (and so the algorithm on that node to terminate) care would have to be taken to ensure that the barrier only waited for active threads.

If the algorithm was altered to allow for multiple node per processor then for each stage of the algorithm each processor would have to preform the tasks for some number of nodes >= 1. This would not present an issue as long as the total number of TODO

 * Could be implemented using bag of tasks, but doesn't fit that well
    * Would need to use a master to schedule tasks
    * Does have the advantage of implictly supporting more than one node per processor
    * Allows different performance of nodes as faster nodes just process more tasks
    * load distribution, deals better with nodes stopping
    * node not tied to a processor

It would also be possible to implement this use the bag of tasks pattern. This isn't quite as elegant and it would be slightly shoehorned in however it does allow have several distinct advantages, most notably the ability lack of a processor being tied to a set of nodes (which is possible due to shared memory between processors) and the ability to load balance nodes between processors to minimize the waiting time if a processor is idle (due to all its nodes having been assigned a colour).

Care would have to be taken to ensure that the task queue is ordered so that the tasks are processed in the correct order. Care would have to be taken to ensure that the tasks don't overrun and taks for a futher iteration started before all the tasks for the current iteration have finished.

To enforce syncronisation either tasks that simply wait for all processors to enter a barrier would ensure TODO

Using this menthod does mean that having multiple nodes per processor is trivial as the pattern doesn't make any assumption of which node belongs to which processor and so to support more nodes than processors is simply a matter of creating more tasks.

 * In this case, neither the other two patterns metnioned in lectures makes any sense
 * Not the best of ideas, maybe use a better algo? Look into this
