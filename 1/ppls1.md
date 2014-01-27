
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
