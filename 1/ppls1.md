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

Investigating the program we find that there are the following paths through the program. 

Path are represented in a simalar way to regex, in that (A,B)\* means 0 or more iterations of A followed by B
Where a line isn't an atomic operation, C represents all operations on the line but  C' is the operation, C'' the second operation etc

First considering only simple paths through the program that treat the loop (A,B) and all lines as atomic we find the following ouputs and paths:

    x = 8, y = 3: (A,B)*, D, E, F, C
    x = 8, y = 2: (A,B)*, D, E, C, F

    x = 0, y = 3: (A,B)*, D, E, (A,B)*, F, C
    x = 0, y = 2: (A,B)*, D, E, (A,B)*, C, F

    x = 0, y = 1 (and fail to terminate): (A, B)*, C, D

The result where x = 0, y = 1 fails to terminate due to the condition for D not being able to be met.

Lines C and B are clearly not atomic as they are actually both a read and a write. Representing the operation in some form of asmembly languge may give something such as follows:

    Load @x, r1
    Add r1, 1
    Store r1, @x

Clearly other commands can be interleaved between these three commands, leading to inconsistencies, so we cannot consider just a single thread.

The add is not observable to other processors, so in our considerations we can reduce the complexity of our considerations by just considering the observable events such for B and C, i.e. load and store. We labeled these as B' and B'' or C' and C'' for B and C respectively.

    Load @x, r1 ##B'
    Add r1, 1
    Store r1, @x ##B''

This means that other operations on the variable can be interleaved between the read and the write. As mentioned above the notation used is that the operation C is in fact two operations, C and C'

    x = 8, y = 1: (A, B)*, D, E, C', F, C''
    x -> -INF , y = 2: (A, B)*, D, E, (A, B)*, F, (A, B)*, C
    x = 2, y = 3: (A, B)*, D, E, (A, B)*, A, F, B, (A, B)*, C
    x = 0, y = 1: (A, B)*, B, D, E, (A,B)*, C', F, C''

The following examples are very dependant on the ordering of what 

    x = 2, y = 3: (A, B)*, D, E, (A, B)*, A, F, B, (A, B)*, C 
    x -> -INF, y = 2: (A, B)*, D, E, (A, B)*, A, F, B, (A, B)*


Of course this is not the only set of paths. The code we were considering was unoptimized and so maintined its order. Due to things such as superscaler execution and reordering, loop unrolling etc I would suspect that the actual possible outputs are far larger and also compiler and machine dependant. I also expcet that this would be close to impossible to analysise by hand


OLD:
So there are the following paths through the program:

    E, F, (A, B)*, C - (NOT POSSIBLE)
    E, (A, B)*, C, F - (NOT POSSIBLE)
    E, (A, B)*, F, C - (NOT POSSIBLE)
    (A, B)*, E, F, C - x = 8, y = 3
    (A, B)*, E, C, F - x = 8, y = 2
    (A, B)*, C, D, - x = 0, y = 1 AND HANG

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
    x = 0, y = 1 (and hang)





### Q2

Write a short report (of around a page) on such a shared variable version of the algorithm, discussing its relationship to any patterns, synchronisation requirements, and the issues which would arise if it were to be amended to allow for more nodes in the graph than processors. NB You are not being asked to write pseudo-code for the algorithm, and no credit will be given for doing so.

 * In the mp impl, messages sent are deg(v) and rndvalue(v) and the first legal colour
 * In the shared memory version this data is readable by everyone

In the message passing implementation of the algorithm each processor represents a node in the graph. Each processor holds internal state about the node $v$ such as the list of colours it cannot be. At each iteration of the algorithm if the node hasn't yet been allocated a colour the processor sends deg(v), rndvalue(v) and its choice of colour to all its neighbours. 

Unlike the message passing implementation all data is readable by all nodes which means that rather than nodes sending data to receiving nodes, the receiving nodes would just read the data themselves. This leads to the problem that it is possible for the receiving node to read data in an inconsistent state and or if the data is processed at different speeds the algorithm could move out of lockstep and so one node could be several iterations ahead of other nodes thereby breaking the algorithm.

 * This is most simalar to the Interacting Peers Pattern
    * Same Program Multiple Data
    * Each processor responsible for maintaining data about invlaid colours etc
    * Use barriers to maintain syncronisation. Simalar to Jacboi algo... mentioned in lectures 

    * Dealing with more tasks per node just means that a single processor has to preform the work of multiple nodes per round
       * Sequentially
       * Either threads to allow each task to be run in parallel
       * Equlivent as long as long as the entire thing remains in lockstep

This is most similar to the interacting peers pattern, as each processor has some other number of processors it interacts with defined as defined by the graph. As with other iteration based algorithms that use this pattern such as the Jacobi algorithm an ideal way of sycronising the processors is to use  barriers to ensure the algorithm runs in lockstep. This barrier would ensure that all processors wait until all active processors have entered the barrier, but given that it is possible for a node to be allocated a colour and so not need to preform any further processing care should be taken that only active processors need to enter the barrier.

If the algorithm was altered to allow for multiple graph nodes per processor then for each iteration of the algorithm each processor would have to preform each the tasks for some number of nodes >= 1.

 * Could be implemented using bag of tasks, but doesn't fit that well
    * Would need to use a master to schedule tasks
    * Does have the advantage of implictly supporting more than one node per processor
    * Allows different performance of nodes as faster nodes just process more tasks
    * load distribution, deals better with nodes stopping
    * node not tied to a processor

It would also be possible to implement this use the bag of tasks pattern. This isn't quite as elegant and it would be slightly shoehorned in however it does allow have several distinct advantages, most notably the ability lack of a processor being tied to a set of nodes (which is possible due to shared memory between processors) and the ability to load balance nodes between processors to minimize the waiting time if a processor is idle (due to all its nodes having been assigned a colour).

Care would have to be taken to ensure that the task queue is ordered so that the tasks are processed in the correct order. Any system implanting this  would have to be taken to ensure that the tasks don't overrun and tasks for a further iteration started before all the tasks for the current iteration have finished. This could be implemented by a bag of tasks that has some understanding of the task and so requires all processors to be waiting for a task before the next iterations tasks are made available.

Using this method does mean that having multiple nodes per processor is trivial as the pattern doesn't make any assumption of which node belongs to which processor and so to support more nodes than processors is simply a matter of creating more tasks.

 * In this case, neither the other two patterns metnioned in lectures makes any sense
    * Producer and consumer
    * Pipeline
 * Not the best of ideas, maybe use a better algo? Look into this


