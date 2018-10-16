#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mpi.h"


int my_rank;            // Global Variable Declarations for rank, arrays, collecting calls and number of proccesses 
static int *calls;
static int *final;   
int p;
   


int MPI_Init(int *argc , char **argv[])             
{
    
    PMPI_Init(argc, argv);                              
    PMPI_Comm_rank(MPI_COMM_WORLD,&my_rank);          // Getting the Rank 
    PMPI_Comm_size(MPI_COMM_WORLD, &p);               // Getting the total number of processes

   calls = (int*) malloc(p*sizeof(int));              // Allocation memory for all ranks to store number of counts to every other rank
    int j;
    for(j=0;j<p;j++)                                  // Setting all values to 0 initially 
    {
       calls[j] =0;
    }
    if(my_rank==0)
    final = (int*) malloc(p * p * sizeof(int));       // Incase of rank 0 allocating memory for gathering all counts information at rank 0

    return 0;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    int result;
    calls[dest]++;                                  // Increment count w.r.t the destination rank
   result = PMPI_Send(buf,count,datatype,dest,tag,comm);

    return result;
    
}
int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm,MPI_Request *request)
{
   int result; 

    result = PMPI_Isend(buf,count,datatype,dest,tag,comm,request);

    calls[dest]++;                                 // Increment count w.r.t the destination rank
    return result;
    
}
int MPI_Finalize(void)
{
    MPI_Gather(calls,64,MPI_INT,final,64,MPI_INT,0,MPI_COMM_WORLD);    // Gathering all the Values at Rank 0 
    if(my_rank==0)                       // Writing to matrix.data file of the total counts at all ranks                               
    {
        int i,j;
        FILE *data;
        data = fopen("matrix.data","w");
        for(i=0;i<p;i++)
        {
            fprintf(data,"%d ",i);
            for(j=0;j<p;j++)
            {
                fprintf(data, "%d ",final[i*p +j]);
            }
       fprintf(data,"\n");
        }
        fclose(data);
    }

    return PMPI_Finalize();

}
