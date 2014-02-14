
aquad: aquad.c
	 mpicc --std=c99 -Wall -o aquad aquad.c stack.h stack.c -lm

aquad_run: aquad
	mpirun -c 5 ./aquad

