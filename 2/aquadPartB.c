/**
 * This version is much simpler than Part A.
 *
 * For each worker the farmer sends a range to work on. The worker
 * then 
 *
 * Using the fact that all tasks need to finish for the farmer to compute the
 * result there is no need to continously poll to see )
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
    double totalArea = 0;

    float splits = numprocs - 1;
    for ( int i = 1; i < numprocs; ++i ){
        double range[2] = { (B-A)/splits*(i-1), (B-A)/splits*i};
        MPI_Send(range, 2, MPI_DOUBLE, i, TAG_WORK, MPI_COMM_WORLD);
    }

    for ( int i = 1; i < numprocs; ++i ){
        double result[2];
        MPI_Status status;
        MPI_Recv(result, 2, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        totalArea += result[0];
        tasks_per_process[i] = result[1];
    }

    return totalArea;
}

double quad(double left, double right, double fleft, double fright, double lrarea, int* timesCalled) {
    double mid, fmid, larea, rarea;

    ++(*timesCalled);

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
    MPI_Status status;

    double data[2];
    MPI_Recv(data, //out intial address of buffer
             2, //count
             MPI_DOUBLE, //data type
             FARMER_ID, //source
             MPI_ANY_TAG, //tag
             MPI_COMM_WORLD, //communication
             &status); //status object

    double a = data[0];
    double b = data[1];

    int timesCalled = 0;
    double area = quad(a, b, F(a), F(b), (F(a)+F(b)) * (b-a)/2, &timesCalled);
    double result[2] = {area, timesCalled};
    MPI_Send(result, 2, MPI_DOUBLE, FARMER_ID, TAG_RESULT, MPI_COMM_WORLD);
}
