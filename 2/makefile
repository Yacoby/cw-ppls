
aquadA: aquadPartA.c
	 mpicc --std=c99 -Wall -o aquadPartA aquadPartA.c stack.h stack.c -lm

aquadA_run: aquadA
	mpirun -c 5 ./aquadPartA

