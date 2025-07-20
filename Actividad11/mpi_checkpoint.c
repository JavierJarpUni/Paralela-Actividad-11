// mpi_checkpoint.c
// Demonstrates coordinated checkpointing and rollback recovery with MPI
// Author: [Your Name]
//
// Compilation: mpicc -o mpi_checkpoint mpi_checkpoint.c
// Execution:   mpirun -np 3 ./mpi_checkpoint
//
// This program simulates a simple computation with periodic coordinated checkpoints.
// If a fault is simulated, processes can recover from the last checkpoint.

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DATA_SIZE 5                 // Size of the data vector
#define CHECKPOINT_INTERVAL 3       // How often to checkpoint (in iterations)
#define FAULT_ITERATION 5           // When to simulate a fault (on rank 1)

// Function to save checkpoint to a file
// Each process writes its iteration count and data vector to a unique file
void save_checkpoint(int rank, int iteration, int* data) {
    char filename[64];
    snprintf(filename, sizeof(filename), "checkpoint_rank_%d.txt", rank);
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Process %d: Error opening checkpoint file for writing!\n", rank);
        return;
    }
    if (fprintf(fp, "%d\n", iteration) < 0) {
        fprintf(stderr, "Process %d: Error writing iteration to checkpoint!\n", rank);
        fclose(fp);
        return;
    }
    for (int i = 0; i < DATA_SIZE; i++) {
        if (fprintf(fp, "%d ", data[i]) < 0) {
            fprintf(stderr, "Process %d: Error writing data to checkpoint!\n", rank);
            fclose(fp);
            return;
        }
    }
    fprintf(fp, "\n");
    fclose(fp);
}

// Function to load checkpoint from a file
// Returns 1 if successful, 0 if no checkpoint found
int load_checkpoint(int rank, int* iteration, int* data) {
    char filename[64];
    snprintf(filename, sizeof(filename), "checkpoint_rank_%d.txt", rank);
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        // No checkpoint file found
        return 0;
    }
    if (fscanf(fp, "%d", iteration) != 1) {
        fprintf(stderr, "Process %d: Error reading iteration from checkpoint!\n", rank);
        fclose(fp);
        return 0;
    }
    for (int i = 0; i < DATA_SIZE; i++) {
        if (fscanf(fp, "%d", &data[i]) != 1) {
            fprintf(stderr, "Process %d: Error reading data from checkpoint!\n", rank);
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

int main(int argc, char* argv[]) {
    int my_rank, num_procs;
    int iteration_count = 0;
    int data_vector[DATA_SIZE];
    int recovered = 0;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (num_procs < 3) {
        if (my_rank == 0) {
            fprintf(stderr, "This application requires at least 3 MPI processes.\n");
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    // Attempt to load checkpoint
    recovered = load_checkpoint(my_rank, &iteration_count, data_vector);
    if (recovered) {
        printf("Process %d: Recovering from checkpoint. Loaded iteration: %d, Data: [", my_rank, iteration_count);
        for (int i = 0; i < DATA_SIZE; i++) {
            printf("%d%s", data_vector[i], (i < DATA_SIZE-1) ? ", " : "]\n");
        }
    } else {
        iteration_count = 0;
        for (int i = 0; i < DATA_SIZE; i++) {
            data_vector[i] = my_rank * 10 + i; // Initial data unique per process
        }
        printf("Process %d: Starting from scratch. Initial iteration: %d, Data: [", my_rank, iteration_count);
        for (int i = 0; i < DATA_SIZE; i++) {
            printf("%d%s", data_vector[i], (i < DATA_SIZE-1) ? ", " : "]\n");
        }
    }

    // Main computation loop
    int max_iterations = 10;
    for (; iteration_count < max_iterations; iteration_count++) {
        // Simulate computation: increment each element by rank + iteration
        for (int i = 0; i < DATA_SIZE; i++) {
            data_vector[i] += (my_rank + 1);
        }
        printf("Process %d: Iteration %d, Data: [", my_rank, iteration_count+1);
        for (int i = 0; i < DATA_SIZE; i++) {
            printf("%d%s", data_vector[i], (i < DATA_SIZE-1) ? ", " : "]\n");
        }

        // Simulate a fault on process 1 at FAULT_ITERATION
        if (my_rank == 1 && iteration_count+1 == FAULT_ITERATION) {
            printf("Process %d: Simulating fault and exiting!\n", my_rank);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        // Coordinated checkpointing
        if ((iteration_count+1) % CHECKPOINT_INTERVAL == 0) {
            printf("Process %d: Reached checkpoint interval. Synchronizing...\n", my_rank);
            fflush(stdout);
            MPI_Barrier(MPI_COMM_WORLD); // All processes synchronize here
            printf("Process %d: Synchronization complete. Saving checkpoint...\n", my_rank);
            save_checkpoint(my_rank, iteration_count+1, data_vector);
            printf("Process %d: Checkpoint saved.\n", my_rank);
        }
        fflush(stdout);
        // Sleep to make output readable and faults visible
        usleep(200000); // 0.2 seconds
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}

/*
============================
Compilation Instructions:
============================
mpicc -o mpi_checkpoint mpi_checkpoint.c

============================
Execution Instructions:
============================
mpirun -np 3 ./mpi_checkpoint

- The program requires at least 3 processes.
- To simulate a fault, simply let the program run: process 1 will exit at iteration 5.
- To recover, re-run the same mpirun command. All processes will load their last checkpoint and continue.
- Checkpoint files are named checkpoint_rank_<rank>.txt and are local to each process.
*/ 