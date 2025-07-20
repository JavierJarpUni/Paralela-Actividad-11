
# MPI Coordinated Checkpoint and Rollback Recovery

This repository contains a simple MPI C application demonstrating coordinated checkpointing and rollback recovery for fault tolerance in distributed systems.

## How to Run

Follow these steps to compile and execute the MPI application:

### 1\. Compilation

Navigate to the directory containing `mpi_checkpoint.c` in your terminal and compile the code using `mpicc`:

```bash
mpicc -o mpi_checkpoint mpi_checkpoint.c
```

This command will create an executable file named `mpi_checkpoint`.

### 2\. Execution

Run the application using `mpirun`. This program is designed to run with at least 3 MPI processes.

```bash
mpirun -np 3 ./mpi_checkpoint
```

#### Simulating a Fault

  * During the **first run**, process 1 is programmed to **simulate a fault** by exiting abruptly at a specific iteration (iteration 5 by default). This will cause the entire MPI job to terminate.

#### Recovering from a Fault

  * After a fault occurs, simply **re-run the same `mpirun` command**:
    ```bash
    mpirun -np 3 ./mpi_checkpoint
    ```
  * The processes will detect their previously saved checkpoint files (`checkpoint_rank_0.txt`, `checkpoint_rank_1.txt`, etc.), load their last consistent state, and resume the computation from that point.
