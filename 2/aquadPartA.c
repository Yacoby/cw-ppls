/**
 * In this implementation I aimed for simplicity above all else. The design decisions
 * with regards to MPI reflect this aim.
 *
 * The farmer sends an initial item of work to a single worker to start processing.
 *
 * Until all workers complete it waits for the data from any worker. The call to MPI_Probe is blocking
 * however this doesn't matter as for a worker to become inactive it has to send some data to the
 * farmer so it wouldn't be possible to get into a state where the farmer is blocked and a worker
 * is inactive.
 *
 * After some data has been received the farmer then sends work to any worker that doesn't
 * currently have a task assigned. For the most part at this point there will only
 * be one task that doesn't have any work allocated (the task that has just sent some
 * data back to the farmer).
 *
 * Rather than have a worker send a message when it has no more work or storing state to attempt
 * to deduce when a worker is inactive, as soon as work is sent back from a worker it is known to
 * be inactive.
 *
 * This means that it is impossible to send back multiple messages from the worker (as doing so would
 * invalidate the assumption above). So in the case where two tasks need to be spawned rather than sending
 * two request for more work containing (left, mid) and (mid, right) a single message is sent. This message 
 * contains (left, mid, right) and is is a requirement of the farmer to split the message and add tasks.
 *
 * This has a slightly lower overhead (3 doubles need to be sent as opposed to 4) and leads to a much simpler 
 * (although slightly less generic) design.
 *
 *
 * The worker preforms the work of the algorithm, taking two input values and either
 * producing a single result to add to the total or sends a (single) message with
 * two new tasks to preform to the farmer. The farmer sends a message to the worker to halt
 * when the algorithm finishes running.
 *
 * For both the worker and farmer MPI_Probe is used before MPI_Recv. This is because
 * different sized data is sent between processes so that it is required to get the
 * type of the message before it is possible to know how much data to receive (i.e.
 * the arguments to MPI_Recv)
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

#define TAG_HALT 0x1 //Sent to workers when needed to halt
#define TAG_WORK 0x2
#define TAG_MORE 0x2 //Sent from workers to the farmer with more tasks to do
#define TAG_RESULT 0x3 //Sent from workers to farmers with a partial result

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
            MPI_Recv(&partialResult, 1, MPI_DOUBLE, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
            totalArea += partialResult;
        }else{
            //get more work from the worker, in the form (left, mid, right)
            double data[3];
            MPI_Recv(data, 3, MPI_DOUBLE, MPI_ANY_SOURCE, TAG_MORE, MPI_COMM_WORLD, &status);

            double lhs[2] = {data[0], data[1]};
            push(lhs, work_stack);

            double rhs[2] = {data[1], data[2]};
            push(rhs, work_stack);
        }
        //as a task has sent some data, it no longer has any work to do and so
        //can be flagged as not working
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

    //when the task has finished we need to send a command to the workers for
    //them to halt.
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
            MPI_Recv(NULL, 0, MPI_DOUBLE, FARMER_ID, TAG_HALT, MPI_COMM_WORLD, &status);
            break;
        }

        double data[2];
        MPI_Recv(data, 2, MPI_DOUBLE, FARMER_ID, TAG_WORK, MPI_COMM_WORLD, &status);

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
            //this is a request for two new items of work. The farmer has enough
            //domain knowledge to split up the data in to the following tasks
            //(left, mid) and (mid, right)
            double response[3] = {left, mid, right};
            MPI_Send(response, 3, MPI_DOUBLE, FARMER_ID, TAG_MORE,MPI_COMM_WORLD);
        }else{
            double total = larea + rarea;
            MPI_Send(&total, 1, MPI_DOUBLE, FARMER_ID, TAG_RESULT, MPI_COMM_WORLD);
        }
    }
}
