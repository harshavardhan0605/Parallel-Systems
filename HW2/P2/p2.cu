/*Single Author info:
mreddy2 Muppidi Harshavardhan Reddy */

/* Program to compute Pi using Monte Carlo methods */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include<cuda_runtime.h>
#include<curand_kernel.h>
#define SEED 35791246
#define THREADS 512

__global__ void integrate(double *x_d,double *y_d, int nitter, curandState *state,double *gsum)
{
    int idx = blockDim.x * blockIdx.x + threadIdx.x;
    unsigned int i;
    curand_init(SEED, idx, 0, &state[idx]);        //Initializing for Currand Function
    __shared__ double sum[THREADS];
    if(idx<nitter)
    {
        curandState localState = state[idx];
        x_d[idx] = curand_uniform(&localState);
        y_d[idx] = curand_uniform(&localState);
        if((x_d[idx]*x_d[idx] + y_d[idx]*y_d[idx])<=1){
        sum[threadIdx.x] = 1;}
        else 
        {
            sum[threadIdx.x] =0;}
    }
    // block reduction
  __syncthreads();
  for (i = blockDim.x / 2; i > 0; i >>= 1) { /* per block */
    if (threadIdx.x < i)
      sum[threadIdx.x] += sum[threadIdx.x + i];
    __syncthreads();
  }
  if (threadIdx.x == 0){
    gsum[blockIdx.x] = sum[threadIdx.x];     // Getting Each Block Total Points 
}
}

int main(int argc, char** argv)
{
   int niter=0;                     // Total Number of Points 
   double *x_d,*y_d, *z,*result_d;  // Device Copy
   int *blocks_d; 
   double count, pi;                // Host Copies
   int i,blocks;  
   curandState *states_d;           // For Currand State Device Copy
    
   niter = atoi(argv[1]);
   blocks = (niter/THREADS) + 1;    // Caluclating Number of Blocks Based on total Points
   
   z = (double *)malloc(niter * sizeof(double));    // Allocating Memory in CPU to use for copying back from GPU

    // GPU Memory Allocation for device copies
   cudaMalloc( (void **) &blocks_d, sizeof(int) * 1 );
   cudaMalloc((void **)&states_d, niter * sizeof(curandState));
   cudaMalloc((void **)&x_d, niter * sizeof(double));
	cudaMalloc((void **)&y_d, niter * sizeof(double));
    cudaMalloc( (void **) &result_d, sizeof(double) * THREADS * blocks);

    integrate<<<blocks,THREADS>>>(x_d, y_d, niter,states_d,result_d);
   
    // copy back from GPU to CPU
    cudaMemcpy( z, result_d, blocks*sizeof(double), cudaMemcpyDeviceToHost);

    for(i=0;i<blocks;i++)       // Summing up total points at all Blocks
    {
        count+= z[i];
    }

    cudaFree(x_d);              // Deallocation of the Memory
    cudaFree(blocks_d);
    cudaFree(y_d);
    cudaFree(result_d);
    cudaFree(states_d);

    pi=(double)count/niter*4;
    printf(" # of trials= %d , estimate of pi is %.16f \n",niter,pi);
}
