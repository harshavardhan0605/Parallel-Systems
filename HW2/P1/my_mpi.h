/*Single Author info:
mreddy2 Muppidi Harshavardhan Reddy */

#ifndef MYMPI_H_
#define MYMPI_H_

#include <stdio.h>
#include <stdlib.h>

#define MPI_Datatype int
#define MPI_CHAR 1
#define MPI_INT 4
#define MPI_DOUBLE 8

#define MAXLEN 128

typedef struct mpi_Comm{
        int my_rank;
        char **all_hosts;
        char *hostnames;
}MPI_Comm;

MPI_Comm MPI_COMM_WORLD;

#define MPI_Status int

int MPI_Init( int *argc, char **argv[] );
int MPI_Comm_size ( MPI_Comm comm, int *size );
int MPI_Comm_rank ( MPI_Comm comm, int *rank );
int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Gather(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Barrier ( MPI_Comm comm );
int MPI_Finalize( MPI_Comm comm );
#endif 