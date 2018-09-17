/*
Single Author info:
mreddy2 Muppidi Harshavardhan Reddy
*/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include<stdlib.h>
#include "mpi.h"

main(int argc, char** argv)
{

    struct timeval time1,time2;        /* For Time Calculations */
    double time_used1[20];        
    double time_diff(struct timeval * prior, struct timeval * latter);  /* Function Declaration for Time Difference Calculation */
    double avg_rtt[17];           /* Average RTT for each Pair*/
    double time_used2[17][11];    /* Each Individual RTT for each Iterations*/
    double std_dev[17];           /* Standard Deviation */

    int my_rank,p, source,dest;   /* Variables for MPI Functions */
    int k,j,z,i;                  /* Loop Variables */
    int tag=50;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
for(j=0;j<17;j++)                  /* Loop for all the sizes */
{
  for(z=0;z<11;z++)                /* Loop for 10 iterations (11 since ignoring the 1st one) */
  {  
    k = pow(2,j+5);
    char *message = (char*) malloc(k * sizeof(char));     /* Allocating Sending Message Size */
    if(my_rank %2 == 0)                                   /* Even Rank Processes Sending the Message and Receiving it back */
    {
        source = dest = my_rank + 1;
        gettimeofday(&time1, NULL);
        MPI_Send(message,k * sizeof(char),MPI_CHAR, dest, tag, MPI_COMM_WORLD);
        MPI_Recv(message,k * sizeof(char),MPI_CHAR,source,tag,MPI_COMM_WORLD, &status);
        gettimeofday(&time2, NULL);

        
    if(z!=0)                                               /* Excluding the 1st Iteration */
    {
        time_used1[j] += time_diff(&time1, &time2);        /* Summation of all RTT Times */
        time_used2[j][z] = time_diff(&time1, &time2);      /* Storing Individual RTT Time for STD DEV Calculation */
    }   
    }
    
    else                                                   /* Odd Rank Processes Receiving and Sending Back */                
    {
            dest = source = my_rank -1;
            MPI_Recv(message,k * sizeof(char),MPI_CHAR,source,tag,MPI_COMM_WORLD, &status);
            MPI_Send(message,k * sizeof(char),MPI_CHAR,dest,tag,MPI_COMM_WORLD);
    }
    free(message); 
  }
   
}
    for(j=0;j<17;j++)
    {
        avg_rtt[j]= time_used1[j]/10;                                /* Avg RTT Calculation*/
        for(i=1;i<11;i++)
        {
            std_dev[j]+= pow((time_used2[j][i]- avg_rtt[j]),2);
        }
            std_dev[j] = sqrt(std_dev[j]/10);                        /* Standard Deviation Calculation */
    }
    double *final_rtt;  
    double *final_std;
    if(my_rank == 0)                                                /* Gathering all the pair RTT Values */
    {
        final_rtt = (double*) malloc(17*p*sizeof(double));          /*Allocating Memory for storing All Final RTT Values at Rank 0 */
        final_std = (double*) malloc(17*p*sizeof(double));          /*Allocating Memory for storing All Final STD Values at Rank 0 */
    }
    /*Gathering all the RTT and STD values from all the processes at Rank 0 */

    MPI_Gather(avg_rtt,17,MPI_DOUBLE,final_rtt,17,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Gather(std_dev,17,MPI_DOUBLE,final_std,17,MPI_DOUBLE,0,MPI_COMM_WORLD);

    if(my_rank==0)
    {
    for(j=0;j<17;j++)
    {
        k = pow(2,j+5);
        printf("%d   %lf %lf  %lf %lf  %lf %lf  %lf %lf\n", k* sizeof(char),final_rtt[j],final_std[j],final_rtt[j+34],final_std[j+34],final_rtt[j+68],final_std[j+68],final_rtt[j+102],final_std[j+102]);
    }
    }
    MPI_Finalize();
}

//Time difference Calculation Function
double time_diff(struct timeval * prior, struct timeval * latter)
{
  double x =
   (double)(latter->tv_usec - prior->tv_usec) / 1000.0 +
   (double)(latter->tv_sec - prior->tv_sec) * 1000.0;
  return x;
} 