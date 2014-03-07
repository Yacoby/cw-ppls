/**
 * This version is much simpler than Part A.
 *
 * For each worker the farmer sends a range to work on. The worker
 * then sends back the total area and the number of iterations of quad per process
 *
 * Three primitives are used: MPI_Scatter, MPI_Gather and MPI_Reduce.
 *
 * The farmer array is built containing the work for each process
 *
 * MPI_Scatter is used to distribute work to all worker processes.
 * Each worker then does the work assigned using the quad function
 *
 * MPI_Reduce is then used to sum everything into a single variable using the MPI_SUM
 * operation and return the result to the farmer.
 *
 * MPI_Gather is used to gather up the number of calls to quad for each process
 * and return them to the farmer.
 *
 * MPI_Scatter and MPI_Gather were used as they fitted the model. It would have also 
 * better possible to use MPI_Send and MPI_Recv for each process but this would have
 * just been doing the same thing as Scatter Gather but (probably) worse.
 *
 * MPI_Reduce was used to gather all the areas and sum them together. This was instead of
 * sending the timing data back from the workers to the farmer using MPI_Gather. This seemed
 * more elegant and avoided data processing on the worker letting MPI do the heavy lifting.
 * The one possible downside was that it would involve more messages. Using just MPI_Gather the number
 * of calls could be sent in an length two array with the area.
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include <stdbool.h>
#include "stack.h"


#define EPSILON 1e-3
#define F(arg)  cosh(arg)*cosh(arg)*cosh(arg)*cosh(arg)
#define A 0.0
#define B 5.0

#define SLEEPTIME 1

#define FARMER_ID 0

#define TAG_WORK 0x1
#define TAG_RESULT 0x2

int* tasks_per_process;

double farmer(int);

void worker(int);

int main(int argc, char** argv ) {
    int i, myid, numprocs;
    double area;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if(numprocs < 2) {
        fprintf(stderr, "ERROR: Must have at least 2 processes to run\n");
        MPI_Finalize();
        exit(1);
    }

    if (myid == FARMER_ID) {
        tasks_per_process = (int*) calloc(numprocs, sizeof(int));
    }

    if (myid == FARMER_ID) {
        area = farmer(numprocs);
    } else {
        worker(myid);
    }

    if(myid == FARMER_ID) {
        fprintf(stdout, "Area=%lf\n", area);
        fprintf(stdout, "\nTasks Per Process\n");
        for (i=0; i<numprocs; i++) {
            fprintf(stdout, "%d\t", i);
        }
        fprintf(stdout, "\n");
        for (i=0; i<numprocs; i++) {
            fprintf(stdout, "%d\t", tasks_per_process[i]);
        }
        fprintf(stdout, "\n");
        free(tasks_per_process);
    }
    MPI_Finalize();
    return 0;
}

double farmer(int numprocs) {
    double* data = (double*)calloc(numprocs*2, sizeof(double));
    const int splits = numprocs - 1;
    for ( int i = 1; i < numprocs; ++i ){
        data[i*2] = (B-A)/splits*(i-1);
        data[i*2+1] = (B-A)/splits*i;
    }

    MPI_Scatter(data, 2, MPI_DOUBLE, data, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    free(data);

    //with these functions the root process has to send, exploits the fact that the first items of
    //data are always 0
    double totalArea = 0;
    MPI_Reduce(data, &totalArea, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    //again exploiting the fact that tasks_per_process[0] = 0
    MPI_Gather(tasks_per_process, 1, MPI_INT, tasks_per_process, 1, MPI_INT, 0, MPI_COMM_WORLD);

    return totalArea;
}

double quad(double left, double right, double fleft, double fright, double lrarea, int* timesCalled) {
    double mid, fmid, larea, rarea;

    ++*timesCalled;

    mid = (left + right) / 2;
    fmid = F(mid);
    larea = (fleft + fmid) * (mid - left) / 2;
    rarea = (fmid + fright) * (right - mid) / 2;
    if( fabs((larea + rarea) - lrarea) > EPSILON ) {
        larea = quad(left, mid, fleft, fmid, larea, timesCalled);
        rarea = quad(mid, right, fmid, fright, rarea, timesCalled);
    }
    return (larea + rarea);
}

void worker(int mypid) {
    double data[2];

    MPI_Scatter(NULL, 0, MPI_DOUBLE, data, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double a = data[0];
    double b = data[1];

    int timesCalled = 0;
    double area = quad(a, b, F(a), F(b), (F(a)+F(b)) * (b-a)/2, &timesCalled);

    MPI_Reduce(&area, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Gather(&timesCalled, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
}