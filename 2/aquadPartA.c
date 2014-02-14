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

#define TAG_HALT 0x1
#define TAG_WORK 0x2
#define TAG_PARTIAL 0x2
#define TAG_RESULT 0x3

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
    int numWorkingTasks = 0;
    bool* workingTasks = calloc(numprocs, sizeof(bool));

    stack* work_stack = new_stack();

    double initial[2] = { A, B };
    MPI_Send(initial ,2, MPI_DOUBLE, 1, TAG_WORK, MPI_COMM_WORLD);
    workingTasks[1] = true;
    ++numWorkingTasks;

    while ( true ){
        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if ( status.MPI_TAG == TAG_RESULT ){
            double partialResult;
            MPI_Recv(&partialResult, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            totalArea += partialResult;
        }else{
            double data[3];
            MPI_Recv(data, 3, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            double lhs[2] = {data[0], data[1]};
            double rhs[2] = {data[1], data[2]};

            push(lhs, work_stack);
            push(rhs, work_stack);
        }
        workingTasks[status.MPI_SOURCE] = false;
        --numWorkingTasks;

        if ( numWorkingTasks == 0 && is_empty(work_stack) ) {
            break;
        }else{
            for ( int i = 1; i < numprocs; ++i ){
                if ( !is_empty(work_stack) && !workingTasks[i] ){
                    double* work = pop(work_stack);

                    ++numWorkingTasks;
                    ++tasks_per_process[i];
                    workingTasks[status.MPI_SOURCE] = true;
                    MPI_Send(work, 2, MPI_DOUBLE, i, TAG_WORK, MPI_COMM_WORLD);
                }
            }
        }
    }

    for ( int i = 1; i < numprocs; ++i ){
        MPI_Send(NULL, 0, MPI_DOUBLE, i, TAG_HALT, MPI_COMM_WORLD);
    }

    return totalArea;
}

void worker(int mypid) {
    MPI_Status status;

    while ( true ){
        MPI_Probe(FARMER_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if ( status.MPI_TAG == TAG_HALT ){
            MPI_Recv(NULL, 0, MPI_DOUBLE, FARMER_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            break;
        }

        double data[2];
        MPI_Recv(data, //out intial address of buffer
                 2, //count
                 MPI_DOUBLE, //data type
                 FARMER_ID, //source
                 MPI_ANY_TAG, //tag
                 MPI_COMM_WORLD, //communication
                 &status); //status object

        if ( status.MPI_TAG == TAG_HALT ){
            break;
        }

        usleep(SLEEPTIME);

        double left = data[0];
        double right = data[1];
        double mid = (left + right) / 2;

        double fleft = F(left);
        double fright = F(right);
        double fmid = F(mid);

        double larea = (fleft + fmid) * (mid - left) / 2;
        double rarea = (fmid + fright) * (right - mid) / 2;
        double lrarea = (fleft + fright) * ((right - left)/2);

        if( fabs((larea + rarea) - lrarea) > EPSILON ) {
            double response[3] = {left, mid, right};
            MPI_Send(response,3,MPI_DOUBLE,FARMER_ID,TAG_PARTIAL,MPI_COMM_WORLD);
        }else{
            double total = larea + rarea;
            MPI_Send(&total,1,MPI_DOUBLE,FARMER_ID,TAG_RESULT,MPI_COMM_WORLD);
        }
    }
}
